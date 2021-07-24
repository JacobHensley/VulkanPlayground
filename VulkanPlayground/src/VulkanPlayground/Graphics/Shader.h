#pragma once
#include <vulkan/vulkan.h>

namespace VKPlayground {

	enum class ShaderStage
	{
		NONE = -1, VERTEX, FRAGMENT, COMPUTE
	};

	class Shader
	{
	public:
		Shader(const std::string& path);

	private:
		void Init();
		bool CompileShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc);
		std::unordered_map<ShaderStage, std::string> SplitShaders(const std::string& path);

	private:
		const std::string m_Path;
		std::unordered_map<ShaderStage, std::string> m_ShaderSrc;
		std::unordered_map<ShaderStage, VkPipelineShaderStageCreateInfo> m_ShaderCreateInfo;
	};

}