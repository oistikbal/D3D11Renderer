#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <iostream>
#include "stb_image.h"


class  skybox
{
private:
	struct MatrixBufferType
	{
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

public:
	 skybox(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& hdrFileName);
	~skybox();
	void render(ID3D11DeviceContext* context, int indexCount, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);
private:
	void CreateCubemapTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& hdrFileName);
	void CreateShaders(ID3D11Device* device);
	void CreateShaderResourceView(ID3D11Device* device);
	void set_shader_parameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX viewMatrix,
		DirectX::XMMATRIX projectionMatrix);

private:
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_cubemapTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubemapSRV;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleState;

	int m_width, m_height;
};