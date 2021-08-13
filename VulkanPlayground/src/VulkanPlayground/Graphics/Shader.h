#pragma once
#include <vulkan/vulkan.h>

namespace VKPlayground {

	enum class ShaderStage
	{
		NONE = -1, VERTEX, FRAGMENT, COMPUTE
	};

	enum class ShaderUniformType
	{
		NONE = -1, BOOL, INT, FLOAT, FLOAT2, FLOAT3, FLOAT4, MAT4, TEXTURE_2D, TEXTURE_CUBE, IMAGE_2D, IMAGE_CUBE
	};

	struct ShaderResource
	{
		std::string Name;
		ShaderUniformType Type;
		uint32_t BindingPoint;
		uint32_t DescriptorSet;
		uint32_t Dimension;
	};

	struct ShaderUniform
	{
		std::string Name;
		ShaderUniformType Type;
		uint32_t Size;
		uint32_t Offset;
	};

	struct UniformBuffer
	{
		std::string Name;
		uint32_t Size;
		uint32_t BindingPoint;
		uint32_t DescriptorSet;
		std::vector<ShaderUniform> Uniforms;
	};

	class Shader
	{
	public:
		Shader(const std::string& path);
		~Shader();

	public:
		inline const std::vector<UniformBuffer>& GetUniformBufferDescriptions() { return m_UniformBufferDescriptions; }
		inline const std::vector<ShaderResource>& GetShaderResourceDescriptions() { return m_ShaderResourceDescriptions; }
		inline const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderCreateInfo() { return m_ShaderCreateInfo; };

	private:
		void Init();
		bool CompileShaders(const std::unordered_map<ShaderStage, std::string>& shaderSrc);
		void ReflectShader(const std::vector<uint32_t>& data, ShaderStage stage);
		std::unordered_map<ShaderStage, std::string> SplitShaders(const std::string& path);

	private:
		const std::string m_Path;
		std::unordered_map<ShaderStage, std::string> m_ShaderSrc;
		std::vector<VkPipelineShaderStageCreateInfo> m_ShaderCreateInfo;

		std::vector<UniformBuffer> m_UniformBufferDescriptions;
		std::vector<ShaderResource> m_ShaderResourceDescriptions;
	};

}