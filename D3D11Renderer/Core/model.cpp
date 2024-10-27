#include "model.h"

#include <stdexcept>
#include <filesystem>


model::model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelfilename, const char* mtlBasePath)
{
	auto result = load_model(device, deviceContext, modelfilename, mtlBasePath);
	if (!result)
	{
		throw std::runtime_error("Failed to initialize model");
	}

	result = initialize_buffers(device);
	if (!result) {
		throw std::runtime_error("Failed to initialize buffers");
	}


}

model::~model()
{
}

void model::render(ID3D11DeviceContext* deviceContext)
{
	render_buffers(deviceContext);
}

const std::vector<model::SubMesh>& model::get_sub_meshes() const
{
	return m_submeshes;
}

bool model::initialize_buffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// Vertex buffer description
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexType) * m_vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Index buffer description
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(unsigned int) * m_indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Setup vertex data
	vertexData.pSysMem = m_vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Setup index data
	indexData.pSysMem = m_indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create buffers
	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf());
	if (FAILED(result)) {
		return false;
	}

	result = device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf());
	if (FAILED(result)) {
		return false;
	}

	return true;
}


void model::render_buffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

bool model::load_texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const aiScene* scene, const char* textureBasePath) {
	// Iterate through all materials in the scene
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* material = scene->mMaterials[i];

		aiString textureFile;

		// Load diffuse texture
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFile) == AI_SUCCESS) {
				std::string diffusePath = std::string(textureBasePath) + "/" + textureFile.C_Str();
				std::wstring wDiffusePath(diffusePath.begin(), diffusePath.end());

				if (std::filesystem::exists(diffusePath)) {
					auto diffuseTexture = std::make_shared<texture>(device, deviceContext, wDiffusePath.c_str());
					m_textures[textureFile.C_Str()] = diffuseTexture;
				}
			}
		}

		// Load normal map
		if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
			if (material->GetTexture(aiTextureType_NORMALS, 0, &textureFile) == AI_SUCCESS) {
				std::string normalPath = std::string(textureBasePath) + "/" + textureFile.C_Str();
				std::wstring wNormalPath(normalPath.begin(), normalPath.end());

				if (std::filesystem::exists(normalPath)) {
					auto normalTexture = std::make_shared<texture>(device, deviceContext, wNormalPath.c_str());
					m_textures[textureFile.C_Str()] = normalTexture;
				}
			}
		}

		// Load specular map (if applicable)
		if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
			if (material->GetTexture(aiTextureType_SPECULAR, 0, &textureFile) == AI_SUCCESS) {
				std::string specularPath = std::string(textureBasePath) + "/" + textureFile.C_Str();
				std::wstring wSpecularPath(specularPath.begin(), specularPath.end());

				if (std::filesystem::exists(specularPath)) {
					auto specularTexture = std::make_shared<texture>(device, deviceContext, wSpecularPath.c_str());
					m_textures[textureFile.C_Str()] = specularTexture;
				}
			}
		}

		if (material->GetTextureCount(aiTextureType_LIGHTMAP) > 0) {
			if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &textureFile) == AI_SUCCESS) {
				std::string specularPath = std::string(textureBasePath) + "/" + textureFile.C_Str();
				std::wstring wSpecularPath(specularPath.begin(), specularPath.end());

				if (std::filesystem::exists(specularPath)) {
					auto specularTexture = std::make_shared<texture>(device, deviceContext, wSpecularPath.c_str());
					m_textures[textureFile.C_Str()] = specularTexture;
				}
			}
		}

		if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
			if (material->GetTexture(aiTextureType_EMISSIVE, 0, &textureFile) == AI_SUCCESS) {
				std::string specularPath = std::string(textureBasePath) + "/" + textureFile.C_Str();
				std::wstring wSpecularPath(specularPath.begin(), specularPath.end());

				if (std::filesystem::exists(specularPath)) {
					auto specularTexture = std::make_shared<texture>(device, deviceContext, wSpecularPath.c_str());
					m_textures[textureFile.C_Str()] = specularTexture;
				}
			}
		}

		if (material->GetTextureCount(aiTextureType_METALNESS) > 0) {
			if (material->GetTexture(aiTextureType_METALNESS, 0, &textureFile) == AI_SUCCESS) {
				std::string specularPath = std::string(textureBasePath) + "/" + textureFile.C_Str();
				std::wstring wSpecularPath(specularPath.begin(), specularPath.end());

				if (std::filesystem::exists(specularPath)) {
					auto specularTexture = std::make_shared<texture>(device, deviceContext, wSpecularPath.c_str());
					m_textures[textureFile.C_Str()] = specularTexture;
				}
			}
		}
	}

	return true;  // Return true regardless of texture loading success or failure
}

bool model::load_model(ID3D11Device* device, ID3D11DeviceContext* deviceContext,const char* modelfilename, const char* mtlPath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelfilename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_PreTransformVertices );

	// Check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		OutputDebugStringA(importer.GetErrorString());
		return false;
	}

	// Load textures
	if (!load_texture(device, deviceContext, scene, mtlPath)) {
		OutputDebugStringA("Failed to load textures.");
		return false;
	}

	// Process meshes
	process_node(device, deviceContext, scene->mRootNode, scene);

	return true;
} 

void model::process_node(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiNode* node, const aiScene* scene)
{
	// Process each mesh in this node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		process_mesh(device, deviceContext, mesh, scene);
	}

	// Process child nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		process_node(device, deviceContext, node->mChildren[i], scene);
	}
}

void model::process_mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMesh* mesh, const aiScene* scene)
{
	std::vector<VertexType> vertices;
	std::vector<unsigned int> indices;

	// Keep track of the starting index in the global vertex buffer
	unsigned int vertexStartIndex = m_vertices.size(); // Starting index for this submesh

	// Process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		VertexType vertex;
		vertex.position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		if (mesh->mTextureCoords[0]) {
			vertex.texture = DirectX::XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else {
			vertex.texture = DirectX::XMFLOAT2(0.0f, 0.0f);
		}

		if (mesh->mNormals) {
			vertex.normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		}

		if (mesh->mTangents && mesh->mBitangents) {
			vertex.tangent = DirectX::XMFLOAT3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			vertex.bitangent = DirectX::XMFLOAT3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		}

		vertices.push_back(vertex); // Add to local vertex list
	}

	// Process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			// Adjust index to point to the correct vertex in the global vertex buffer
			indices.push_back(face.mIndices[j] + vertexStartIndex);
		}
	}

	// Create the SubMesh for this mesh
	SubMesh subMesh;
	subMesh.startIndex = m_indices.size(); // Set the start index for this submesh
	subMesh.indexCount = indices.size();

	// Handle materials and assign textures
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString texturePath;

		// Assign diffuse texture
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
			subMesh.diffuseTexture = m_textures[texturePath.C_Str()];
		}

		// Assign normal map texture
		if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
			subMesh.normalTexture = m_textures[texturePath.C_Str()];
		}

		// Assign specular map texture
		if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS) {
			subMesh.specularTexture = m_textures[texturePath.C_Str()];
		}

		// Assign Ambient Occlusion (AO) map texture
		if (material->GetTexture(aiTextureType_LIGHTMAP, 0, &texturePath) == AI_SUCCESS) {
			subMesh.aoTexture = m_textures[texturePath.C_Str()];
		}

		// Assign Emissive texture
		if (material->GetTexture(aiTextureType_EMISSIVE, 0, &texturePath) == AI_SUCCESS) {
			subMesh.emissiveTexture = m_textures[texturePath.C_Str()];
		}

		// Assign Metal-Roughness texture
		if (material->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS) {
			subMesh.metalRoughnessTexture = m_textures[texturePath.C_Str()];
		}
	}

	// Store the vertices and indices
	m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
	m_indices.insert(m_indices.end(), indices.begin(), indices.end());
	m_submeshes.push_back(subMesh);
}

