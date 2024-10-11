#include "model.h"
#include <stdexcept>
#include "tiny_obj_loader.h"
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

bool model::load_texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* mtlbasepath, const std::vector<tinyobj::material_t>& materials) {
	for (const auto& material : materials) {
		// Check if the material has a valid diffuse texture name
		if (!material.diffuse_texname.empty()) {
			// Construct the full texture path
			std::string texturePath = std::string(mtlbasepath) + "/" + material.diffuse_texname;

			// Convert the texture path to a wide string for DirectX
			std::wstring wTexturePath(texturePath.begin(), texturePath.end());

			// Check if the texture file exists
			if (std::filesystem::exists(texturePath)) {
				// Initialize the texture object if the file exists
				auto textureObj = std::make_shared<texture>(device, deviceContext, wTexturePath.c_str());

				// Map the texture by the material name
				m_textures[material.name] = textureObj;
			}
		}
	}
	return true; // Return true regardless of texture loading success or failure
}

bool model::load_model(ID3D11Device* device, ID3D11DeviceContext* deviceContext,const char* modelfilename, const char* mtlPath)
{
	std::string warn, err;
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelfilename, mtlPath);

	// Check for warnings and errors
	if (!warn.empty()) {
		OutputDebugStringA(warn.c_str());
		return false;
	}
	if (!err.empty()) {
		OutputDebugStringA(warn.c_str());
		return false;
	}

	if (!load_texture(device, deviceContext, mtlPath, materials)) {
		OutputDebugStringA("Failed to load textures.");
		return false;
	}

	// Iterate through shapes to populate vertex and index data
	for (size_t s = 0; s < shapes.size(); s++) {
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Per-face loop
			for (size_t v = 0; v < fv; v++) {
				// Access vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

				// Access normal
				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

				// Access texcoords
				tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];

				// Fill the vertex array
				VertexType vertex;
				vertex.position = DirectX::XMFLOAT3(vx, vy, vz);
				vertex.texture = DirectX::XMFLOAT2(tx, ty);
				vertex.normal = DirectX::XMFLOAT3(nx, ny, nz);

				m_vertices.push_back(vertex);
				m_indices.push_back(static_cast<unsigned int>(m_indices.size())); // Auto-increment index
			}
			index_offset += fv;
		}
	}

	// Create SubMesh objects for each shape
	for (size_t s = 0; s < shapes.size(); s++) {
		SubMesh subMesh;
		subMesh.indexCount = shapes[s].mesh.indices.size();

		// Retrieve the texture for this submesh if it has one
		if (s < materials.size()) {
			const auto& material = materials[shapes[s].mesh.material_ids[0]];
			if (!material.name.empty()) {
				subMesh.texture = m_textures[material.name];
			}
		}

		m_submeshes.push_back(subMesh);
	}

	return true;
}
