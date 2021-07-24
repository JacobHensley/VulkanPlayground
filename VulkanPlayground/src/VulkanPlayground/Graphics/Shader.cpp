#include "pch.h"
#include "Shader.h"
#include "VulkanPlayground/Core/Application.h"
#include <shaderc/shaderc.hpp>

namespace VKPlayground {

	namespace Utils {

		static shaderc_shader_kind ShaderStageToShaderc(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::VERTEX:	return shaderc_vertex_shader;
				case ShaderStage::FRAGMENT: return shaderc_fragment_shader;
				case ShaderStage::COMPUTE:  return shaderc_compute_shader;
			}

			ASSERT(false, "Unknown Type");
			return (shaderc_shader_kind)0;
		}

		static VkShaderStageFlagBits ShaderStageToVulkan(ShaderStage stage)
		{
			switch (stage)
			{
				case ShaderStage::VERTEX:	return VK_SHADER_STAGE_VERTEX_BIT;
				case ShaderStage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
				case ShaderStage::COMPUTE:  return VK_SHADER_STAGE_COMPUTE_BIT;
			}

			ASSERT(false, "Unknown Type");
			return (VkShaderStageFlagBits)0;
		}

	}

	Shader::Shader(const std::string& path)
		: m_Path(path)
	{
		Init();
		LOG_INFO("Initialized shader: {0}", m_Path);
	}

	void Shader::Init()
	{
		m_ShaderSrc = SplitShaders(m_Path);
		ASSERT(m_ShaderSrc.size() >= 1, "Shader is empty or path is invalid");

		bool result = CompileShaders(m_ShaderSrc);
		ASSERT(result, "Failed to initialize shader: {0}", m_Path)
	}

	bool Shader::CompileShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc)
	{
		// Setup compiler
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);

		VkDevice logicalDevice = Application::GetApp().GetVulkanDevice()->GetLogicalDevice();

		for (auto&& [stage, src] : m_ShaderSrc)
		{
			// Compile shader source and check for errors
			auto compilationResult = compiler.CompileGlslToSpv(src, Utils::ShaderStageToShaderc(stage), m_Path.c_str(), options);
			if (compilationResult.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				LOG_ERROR("Warnings ({0}), Errors ({1}) \n{2}", compilationResult.GetNumWarnings(), compilationResult.GetNumErrors(), compilationResult.GetErrorMessage());
				return false;
			}

			const uint8_t* data = reinterpret_cast<const uint8_t*>(compilationResult.cbegin());
			const uint8_t* dataEnd = reinterpret_cast<const uint8_t*>(compilationResult.cend());
			uint32_t size = dataEnd - data;

			// Create shader module
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = size;
			createInfo.pCode = reinterpret_cast<const uint32_t*>(data);

			VkShaderModule shaderModule;
			VkResult result = vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule);
			ASSERT(result == VK_SUCCESS, "Failed to initialize shader module");

			// Create shader stage
			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageInfo.stage = Utils::ShaderStageToVulkan(stage);;
			vertShaderStageInfo.module = shaderModule;
			vertShaderStageInfo.pName = "main";

			// Save shader stage info
			m_ShaderCreateInfo[stage] = vertShaderStageInfo;

			// destroy shader module
			vkDestroyShaderModule(logicalDevice, shaderModule, nullptr);
		}
	}

	std::unordered_map<ShaderStage, std::string> Shader::SplitShaders(const std::string& path)
	{
		std::unordered_map<ShaderStage, std::string> result;
		ShaderStage stage = ShaderStage::NONE;

		std::ifstream stream(path);
		
		std::stringstream ss[2];
		std::string line;

		while (getline(stream, line))
		{
			if (line.find("#Shader") != std::string::npos)
			{
				if (line.find("Vertex") != std::string::npos)
				{
					stage = ShaderStage::VERTEX;
				}
				else if (line.find("Fragment") != std::string::npos)
				{
					stage = ShaderStage::FRAGMENT;
				}
				else if (line.find("Compute") != std::string::npos)
				{
					stage = ShaderStage::COMPUTE;
				}
			}
			else
			{
				result[stage] += line + '\n';
			}
		}

		return result;
	}

}