#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include <wrl/client.h>

class color_shader
{

private:
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

public:
	color_shader(ID3D11Device*, HWND);
	~color_shader();

	bool render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,
		DirectX::XMMATRIX projectionMatrix);
private:
	void output_shader_error_message(ID3D10Blob*, HWND, WCHAR*);

	bool set_shader_parameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,
		DirectX::XMMATRIX projectionMatrix);
	void render_shader(ID3D11DeviceContext* deviceContext, int indexCount);
	bool initialize_shader(ID3D11Device*, HWND, WCHAR*, WCHAR*);


private:

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
};
