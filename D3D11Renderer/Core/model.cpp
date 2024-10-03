#include "model.h"
#include <stdexcept>

model::model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* modelfilename, const wchar_t* texturefilename)
{
	auto result = load_model(modelfilename);
	if (!result)
	{
		throw std::runtime_error("Failed to initialize model");
	}


	result = initialize_buffers(device);
	if (!result) {
		throw std::runtime_error("Failed to initialize buffers");
	}

	result = load_texture(device, deviceContext, texturefilename);
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

int model::get_index_count()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* model::get_texture()
{
	return m_texture->get_texture();
}

bool model::initialize_buffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	vertices = new VertexType[m_vertexCount];

	// Create the index array.
	indices = new unsigned long[m_indexCount];

	// Create the vertex array.
	for (i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = DirectX::XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = DirectX::XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = DirectX::XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

		indices[i] = i;
	}

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

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

bool model::load_texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename)
{
	try
	{
		m_texture = std::make_shared<texture>(device, deviceContext, filename);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}

	return true;
}

bool model::load_model(const wchar_t* modelfilename)
{
	std::ifstream fin;
	char input;
	int i;

	// Open the model file.
	fin.open(modelfilename);

	if (fin.fail())
	{
		return false;
	}

	// Read up to the value of vertex count.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// Read in the vertex count.
	fin >> m_vertexCount;

	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Resize the vector to hold the model data.
	m_model.resize(m_vertexCount);

	// Read up to the beginning of the data.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// Read in the vertex data.
	for (i = 0; i < m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	fin.close();

	return true;
}
