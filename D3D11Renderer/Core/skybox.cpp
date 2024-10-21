#include "skybox.h"
#include <format>
#include <DirectXTex.h>


using namespace Microsoft::WRL;

skybox::skybox(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& hdrFileName)
{
    CreateCubemapTexture(device, deviceContext, hdrFileName);
    CreateShaders(device);
    CreateShaderResourceView(device);
}

skybox::~skybox()
{

}

void skybox::render(ID3D11DeviceContext* context, int indexCount, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix)
{
    set_shader_parameters(context, viewMatrix, projectionMatrix);
    context->IASetInputLayout(m_layout.Get());

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    context->PSSetSamplers(0, 1, m_sampleState.GetAddressOf());
    context->PSSetShaderResources(0, 1, m_cubemapSRV.GetAddressOf());

    // Set primitive topology to triangle list
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(indexCount, 0, 0); // Draw the sphere using index buffer
}

void skybox::CreateCubemapTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& hdrFileName)
{
    DirectX::ScratchImage scratchImage;

    // Load the panoramic HDR image
    HRESULT hr = DirectX::LoadFromHDRFile(hdrFileName.c_str(), nullptr, scratchImage);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to load HDR file.");
    }

    const DirectX::Image* image = scratchImage.GetImage(0, 0, 0);
    if (!image) {
        throw std::runtime_error("Failed to retrieve image data.");
    }

    // Create a 2D texture for the panorama (instead of a cubemap)
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = image->width;   // Full panorama width
    textureDesc.Height = image->height; // Full panorama height
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;          // Single 2D texture
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    // Create the 2D texture for the panorama
    HRESULT createTextureResult = device->CreateTexture2D(&textureDesc, nullptr, m_cubemapTexture.GetAddressOf());
    if (FAILED(createTextureResult)) {
        throw std::runtime_error("Failed to create panorama texture.");
    }

    // Copy the data from the loaded HDR image into the texture
    deviceContext->UpdateSubresource(
        m_cubemapTexture.Get(),
        0, nullptr, image->pixels, image->rowPitch, 0
    );
}

void skybox::CreateShaders(ID3D11Device* device)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
    unsigned int numElements;
    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;

    HRESULT hr = D3DCompileFromFile(L"Shaders/skyboxvs.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, vsBlob.GetAddressOf(), nullptr);
    if (SUCCEEDED(hr)) {
        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf());
    }
    else {
        throw std::runtime_error("Failed to compile vertex shader.");
    }

    hr = D3DCompileFromFile(L"Shaders/skyboxps.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, psBlob.GetAddressOf(), nullptr);
    if (SUCCEEDED(hr)) {
        hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf());
    }
    else {
        throw std::runtime_error("Failed to compile pixel shader.");
    }

    polygonLayout[0].SemanticName = "POSITION";
    polygonLayout[0].SemanticIndex = 0;
    polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    polygonLayout[0].InputSlot = 0;
    polygonLayout[0].AlignedByteOffset = 0;
    polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[0].InstanceDataStepRate = 0;

    polygonLayout[1].SemanticName = "TEXCOORD";
    polygonLayout[1].SemanticIndex = 0;
    polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    polygonLayout[1].InputSlot = 0;
    polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    polygonLayout[1].InstanceDataStepRate = 0;

    // Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

    // Create the vertex input layout.
    device->CreateInputLayout(polygonLayout, numElements, vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(), m_layout.GetAddressOf());


    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;

    // Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
    device->CreateBuffer(&matrixBufferDesc, NULL, m_matrixBuffer.GetAddressOf());

    // Create a texture sampler state description.
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
    device->CreateSamplerState(&samplerDesc, m_sampleState.GetAddressOf());
}

void skybox::CreateShaderResourceView(ID3D11Device* device)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D; // Use 2D texture instead of cubemap
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    HRESULT hr = device->CreateShaderResourceView(m_cubemapTexture.Get(), &srvDesc, m_cubemapSRV.GetAddressOf());
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create shader resource view.");
    }
}

void skybox::set_shader_parameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    unsigned int bufferNumber;


    viewMatrix = DirectX::XMMatrixTranspose(viewMatrix);
    projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);

    // Lock the constant buffer so it can be written to.
    deviceContext->Map(m_matrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    // Get a pointer to the data in the constant buffer.
    dataPtr = (MatrixBufferType*)mappedResource.pData;

    dataPtr->view = viewMatrix;
    dataPtr->projection = projectionMatrix;

    // Unlock the constant buffer.
    deviceContext->Unmap(m_matrixBuffer.Get(), 0);

    // Set the position of the constant buffer in the vertex shader.
    bufferNumber = 0;

    // Finanly set the constant buffer in the vertex shader with the updated values.
    deviceContext->VSSetConstantBuffers(bufferNumber, 1, m_matrixBuffer.GetAddressOf());

    // Set shader texture resource in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, m_cubemapSRV.GetAddressOf());
}
