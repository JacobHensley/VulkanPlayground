#pragma once
#include "VulkanPlayground/Graphics/VulkanBuffers.h"
#include <tinygltf/tiny_gltf.h>
#include <glm/glm.hpp>

namespace VKPlayground {

	struct SubMesh
	{
		uint32_t VertexOffset = 0;
		uint32_t IndexOffset = 0;
		uint32_t IndexCount = 0;
	};

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec3 Tangent;
		glm::vec2 TextureCoords;
	};

	class Mesh
	{
	public:
		Mesh(const std::string& path);
		~Mesh();

		inline const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }

		inline Ref<VulkanVertexBuffer> GetVertexBuffer() const { return m_VertexBuffer; }
		inline Ref<VulkanIndexBuffer> GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		void Init();
		void LoadData();
		void CalculateNodeTransforms(const tinygltf::Model& model);

	private:
		std::string m_Path;

		std::vector<SubMesh> m_SubMeshes;
		std::vector<Vertex> m_Vertices;
		std::vector<uint16_t> m_Indices;

		Ref<VulkanVertexBuffer> m_VertexBuffer;
		Ref<VulkanIndexBuffer> m_IndexBuffer;

		tinygltf::Model m_Model;
	};

}


