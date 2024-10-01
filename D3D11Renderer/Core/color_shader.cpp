#include "color_shader.h"

using namespace Microsoft::WRL;

color_shader::color_shader(ID3D11Device* device, HWND hwnd)
{
	bool result;
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int error;


	error = wcscpy_s(vsFilename, 128, L"Shaders/colorvs.hlsl");
	if (error != 0) {
		throw std::runtime_error("Failed to copy the vertex shader filename.");
	}

	// Set the filename of the pixel shader.
	error = wcscpy_s(psFilename, 128, L"Shaders/colorps.hlsl");
	if (error != 0) {
		throw std::runtime_error("Failed to copy the pixel shader filename.");
	}

	// Initialize the vertex and pixel shaders.
	result = initialize_shader(device, hwnd, vsFilename, psFilename);
	if (!result) {
		throw std::runtime_error("Failed to initialize the vertex and pixel shaders.");
	}
}

color_shader::~color_shader()
{
}

bool color_shader::render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix)
{
	bool result;


	// Set the shader parameters that it will use for rendering.
	result = set_shader_parameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	render_shader(deviceContext, indexCount);

	return true;
}

bool color_shader::initialize_shader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ComPtr<ID3D10Blob> errorMessage;
	ComPtr<ID3D10Blob> vertexShaderBuffer;
	ComPtr<ID3D10Blob> pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;


	result = D3DCompileFromFile(vsFilename, NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		vertexShaderBuffer.GetAddressOf(), errorMessage.GetAddressOf());
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			output_shader_error_message(errorMessage.Get(), hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	// Compile the pixel shader code.
	result = D3DCompileFromFile(psFilename, NULL, NULL, "main", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		pixelShaderBuffer.GetAddressOf(), errorMessage.GetAddressOf());
	if (FAILED(result))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMessage)
		{
			output_shader_error_message(errorMessage.Get(), hwnd, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, m_vertexShader.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, m_pixelShader.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}


	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), m_layout.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&matrixBufferDesc, NULL, m_matrixBuffer.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void color_shader::output_shader_error_message(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	std::ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool color_shader::set_shader_parameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,
	DirectX::XMMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	result = deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer.Get(), 0);

		// Set the position of the constant buffer in the vertex shader.
		bufferNumber = 0;

	// Finanly set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, m_matrixBuffer.GetAddressOf());

	return true;
}

void color_shader::render_shader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_layout.Get());

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// Render the triangle.
	deviceContext->DrawIndexed(indexCount, 0, 0);

	return;
}
