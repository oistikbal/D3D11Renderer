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

		// Check for diffuse texture (you can extend this for other types like specular, normals, etc.)
		if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
			aiString textureFile;

			// Get the texture file name from the material
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &textureFile) == AI_SUCCESS) {
				// Construct the full path to the texture

				std::string texturePath = std::string(textureBasePath) + "/" + textureFile.C_Str();

				// Convert to a wide string for DirectX
				std::wstring wTexturePath(texturePath.begin(), texturePath.end());

				// Check if the texture file exists
				if (std::filesystem::exists(texturePath)) {
					// Initialize the texture object using your texture class
					auto textureObj = std::make_shared<texture>(device, deviceContext, wTexturePath.c_str());

					// Get the material name and map the texture to it
					aiString materialName;
					material->Get(AI_MATKEY_NAME, materialName);

					// Store the texture in the material texture map
					m_textures[materialName.C_Str()] = textureObj;
				}
			}
		}
	}

	return true;  // Return true regardless of texture loading success or failure
}

bool model::load_model(ID3D11Device* device, ID3D11DeviceContext* deviceContext,const char* modelfilename, const char* mtlPath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(modelfilename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

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

	// Process vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		VertexType vertex;
		vertex.position = DirectX::XMFLOAT3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

		if (mesh->mTextureCoords[0]) { // Check if the mesh contains texture coordinates
			vertex.texture = DirectX::XMFLOAT2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		else {
			vertex.texture = DirectX::XMFLOAT2(0.0f, 0.0f);
		}

		if (mesh->mNormals) { // Check if the mesh contains normals
			vertex.normal = DirectX::XMFLOAT3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		}

		vertices.push_back(vertex);
	}

	// Process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Now you have vertices and indices, you can use them to create buffers (DirectX) or for rendering.

	// Store in your mesh data structures
	m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
	m_indices.insert(m_indices.end(), indices.begin(), indices.end());

	// SubMesh for each material
	SubMesh subMesh;
	subMesh.indexCount = indices.size();

	// Handle materials and textures
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		aiString texturePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
			subMesh.texture = m_textures[material->GetName().C_Str()]; // Load the texture based on path
		}
	}

	m_submeshes.push_back(subMesh);

}
