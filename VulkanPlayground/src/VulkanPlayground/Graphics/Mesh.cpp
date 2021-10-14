#include "pch.h"
#include "Mesh.h"
#include "glm/gtc/type_ptr.hpp"

namespace VKPlayground {

	Mesh::Mesh(const std::string& path)
		: m_Path(path)
	{
		Init();
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Init()
	{
		tinygltf::TinyGLTF loader;
		std::string error;
		std::string warning;

		bool result = loader.LoadASCIIFromFile(&m_Model, &error, &warning, m_Path);
		ASSERT(warning.empty(), warning);
		ASSERT(error.empty(), error);

		LoadData();
		CalculateNodeTransforms(m_Model);

		m_VertexBuffer = CreateRef<VulkanVertexBuffer>(m_Vertices.data(), sizeof(Vertex) * m_Vertices.size());
		m_IndexBuffer = CreateRef<VulkanIndexBuffer>(m_Indices.data(), sizeof(uint16_t) * m_Indices.size(), m_Indices.size());
	}

	void Mesh::LoadData()
	{
		m_SubMeshes.reserve(m_Model.meshes.size());

		int subMeshVertexOffset = 0;
		int subMeshIndexOffset = 0;
		int meshIndex = 1;
		uint32_t indexCount = 0;

		for (tinygltf::Mesh mesh : m_Model.meshes)
		{
			uint32_t vertexCount = m_Model.accessors[0].count;
			int subMeshIndexCount = 0;

			for (int i = 0; i < mesh.primitives.size(); i++)
			{
				if (m_Vertices.size() < vertexCount * meshIndex)
					m_Vertices.resize(vertexCount * meshIndex);

				// Positions
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[mesh.primitives[i].attributes["POSITION"]];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];
					const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].Position.x = positions[j * 3 + 0];
						m_Vertices[subMeshVertexOffset + j].Position.y = positions[j * 3 + 1];
						m_Vertices[subMeshVertexOffset + j].Position.z = positions[j * 3 + 2];
					}
				}

				// Normals
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[mesh.primitives[i].attributes["NORMAL"]];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];
					const float* normals = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].Normal.x = normals[j * 3 + 0];
						m_Vertices[subMeshVertexOffset + j].Normal.y = normals[j * 3 + 1];
						m_Vertices[subMeshVertexOffset + j].Normal.z = normals[j * 3 + 2];
					}
				}

				// Tangents
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[mesh.primitives[i].attributes["TANGENT"]];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];
					const float* tangents = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].Tangent.x = tangents[j * 4 + 0];
						m_Vertices[subMeshVertexOffset + j].Tangent.y = tangents[j * 4 + 1];
						m_Vertices[subMeshVertexOffset + j].Tangent.z = tangents[j * 4 + 2];
						//	m_Vertices[subMeshVertexOffset+ j].Tangent.w = tangents[j * 4 + 3];
					}
				}

				// Texture Coordinates
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[mesh.primitives[i].attributes["TEXCOORD_0"]];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];
					const float* texCoords = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

					for (int j = 0; j < accessor.count; j++)
					{
						m_Vertices[subMeshVertexOffset + j].TextureCoords.x = texCoords[j * 2 + 0];
						m_Vertices[subMeshVertexOffset + j].TextureCoords.y = texCoords[j * 2 + 1];
					}
				}

				// Indices
				{
					const tinygltf::Accessor& accessor = m_Model.accessors[mesh.primitives[i].indices];
					const tinygltf::BufferView& bufferView = m_Model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = m_Model.buffers[bufferView.buffer];
					const uint16_t* indices = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

					m_Indices.resize(accessor.count * meshIndex);

					for (int j = 0; j < accessor.count; j++)
					{
						m_Indices[indexCount + j] = indices[j];
					}

					indexCount += accessor.count;
					subMeshIndexCount = accessor.count;
				}

				meshIndex++;
			}

			SubMesh& subMesh = m_SubMeshes.emplace_back();

			subMesh.VertexOffset = subMeshVertexOffset;
			subMesh.IndexOffset = subMeshIndexOffset;
			subMesh.IndexCount = subMeshIndexCount;

			subMeshVertexOffset += vertexCount;
			subMeshIndexOffset += indexCount;
		}

	}

	void Mesh::CalculateNodeTransforms(const tinygltf::Model& model)
	{
		for (int i = 0; i < model.nodes.size(); i++)
		{
			tinygltf::Node node = model.nodes[i];

			glm::mat4 transform;

			if (node.translation.size() == 3) 
			{
				transform = glm::translate(transform, glm::vec3(glm::make_vec3(node.translation.data())));
			}
			if (node.rotation.size() == 4) 
			{
				glm::quat q = glm::make_quat(node.rotation.data());
				transform *= glm::mat4(q);
			}
			if (node.scale.size() == 3) 
			{
				transform = glm::scale(transform, glm::vec3(glm::make_vec3(node.scale.data())));
			}
			if (node.matrix.size() == 16) 
			{
				transform = glm::make_mat4x4(node.matrix.data());
			};
		}
	}

}