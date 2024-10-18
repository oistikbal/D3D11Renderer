#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include "stb_image.h"
#include <iostream>

class  skybox
{
public:
	 skybox(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& hdrFileName);
	~skybox();
	void render(ID3D11DeviceContext* context, int indexCount);
private:
	void CreateCubemapTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& hdrFileName);
	void CreateShaders(ID3D11Device* device);
	void CreateShaderResourceView(ID3D11Device* device);

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_cubemapTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubemapSRV;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

	int m_width, m_height;
};