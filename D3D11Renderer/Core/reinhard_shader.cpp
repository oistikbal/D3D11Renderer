#include "reinhard_shader.h"

using namespace Microsoft::WRL;

reinhard_shader::reinhard_shader(ID3D11Device* device, HWND hwnd)
{
	bool result;
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int error;


	error = wcscpy_s(vsFilename, 128, L"Shaders/quadvs.hlsl");
	if (error != 0) {
		throw std::runtime_error("Failed to copy the vertex shader filename.");
	}

	// Set the filename of the pixel shader.
	error = wcscpy_s(psFilename, 128, L"Shaders/reinhardps.hlsl");
	if (error != 0) {
		throw std::runtime_error("Failed to copy the pixel shader filename.");
	}

	// Initialize the vertex and pixel shaders.
	result = initialize_shader(device, hwnd, vsFilename, psFilename);
	if (!result) {
		throw std::runtime_error("Failed to initialize the vertex and pixel shaders.");
	}
}

reinhard_shader::~reinhard_shader()
{
}

bool reinhard_shader::render(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, float exposure, float averageLuminance, float maxLuminance, float burn)
{
    bool result;


    // Set the shader parameters that it will use for rendering.
    result = set_shader_parameters(deviceContext, texture, exposure, averageLuminance, maxLuminance, burn);
    if (!result)
    {
        return false;
    }

    // Now render the prepared buffers with the shader.
    render_shader(deviceContext);

    return true;
}

void reinhard_shader::output_shader_error_message(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

bool reinhard_shader::set_shader_parameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, float exposure, float averageLuminance, float maxLuminance, float burn)
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result;

    // Map the constant buffer to update its values
    result = deviceContext->Map(m_toneMapBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return false;
    }

    // Fill in the values for the constant buffer
    ToneMapBufferType* dataPtr = (ToneMapBufferType*)mappedResource.pData;
    dataPtr->exposure = exposure;
    dataPtr->averageLuminance = averageLuminance;
    dataPtr->maxLuminance = maxLuminance;
    dataPtr->burn = burn;

    deviceContext->Unmap(m_toneMapBuffer.Get(), 0);

    // Set the constant buffer in the pixel shader
    deviceContext->PSSetConstantBuffers(0, 1, m_toneMapBuffer.GetAddressOf());

    // Set the texture resource in the pixel shader
    deviceContext->PSSetShaderResources(0, 1, &texture);
    return true;
}

void reinhard_shader::render_shader(ID3D11DeviceContext* deviceContext)
{
    deviceContext->IASetInputLayout(nullptr);

    // Set the vertex and pixel shaders that will be used to render this triangle.
    deviceContext->VSSetShader(m_vertexShader.Get(), NULL, 0);
    deviceContext->PSSetShader(m_pixelShader.Get(), NULL, 0);

    // Set the sampler state in the pixel shader.
    deviceContext->PSSetSamplers(0, 1, m_sampleState.GetAddressOf());

    deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


    // Render the triangle.
    deviceContext->Draw(4, 0);

    ID3D11ShaderResourceView* nullSRV = nullptr;
    deviceContext->PSSetShaderResources(0, 1, &nullSRV);
    return;
}

bool reinhard_shader::initialize_shader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
    HRESULT result;
    ComPtr<ID3D10Blob> errorMessage;
    ComPtr<ID3D10Blob> vertexShaderBuffer;
    ComPtr<ID3D10Blob> pixelShaderBuffer;

    D3D11_SAMPLER_DESC samplerDesc;


    result = D3DCompileFromFile(vsFilename, NULL, NULL, "main", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
        vertexShaderBuffer.GetAddressOf(), errorMessage.GetAddressOf());
    if (FAILED(result))
    {
        // If the shader failed to compile it should have writen something to the error message.
        if (errorMessage)
        {
            output_shader_error_message(errorMessage.Get(), hwnd, vsFilename);
        }
        // If there was nothing in the error message then it simply could not find the shader file itself.
        else
        {
            MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
        }

        return false;
    }

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

    // Create the pixel shader from the buffer.
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, m_pixelShader.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }


    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
    samplerDesc.BorderColor[1] = 0;
    samplerDesc.BorderColor[2] = 0;
    samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    result = device->CreateSamplerState(&samplerDesc, m_sampleState.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(ToneMapBufferType);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&bufferDesc, nullptr, m_toneMapBuffer.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    return true;
}
