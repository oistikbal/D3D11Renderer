#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <wrl/client.h>

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

class texture
{
public:
	texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename);
	~texture();


	ID3D11ShaderResourceView* get_texture();
	bool initialize(ID3D11Device*, ID3D11DeviceContext*, const wchar_t* filename);
private:
	Microsoft::WRL::ComPtr<ID3D11Resource> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureView;
};
