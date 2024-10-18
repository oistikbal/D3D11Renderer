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

void skybox::render(ID3D11DeviceContext* context, int indexCount)
{

    context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

    context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
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
    // Compile and create shaders (you'll need to create your own shaders)
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
