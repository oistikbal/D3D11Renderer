#include "texture.h"
#include <d3d11.h>
#include <stdexcept>

texture::texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename)
{
    auto result = initialize(device, deviceContext, filename);
    if (!result) {
        throw std::runtime_error("Failed to initialize texture");
    }
}

texture::~texture()
{
}

ID3D11ShaderResourceView* texture::get_texture()
{
    return m_textureView.Get();
}

bool texture::initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename)
{
    HRESULT result;

    result = DirectX::CreateWICTextureFromFile(device, deviceContext, filename, m_texture.GetAddressOf(), m_textureView.GetAddressOf());
    if (FAILED(result))
    {
        return false;
    }

    deviceContext->GenerateMips(m_textureView.Get());

    return true;
}
