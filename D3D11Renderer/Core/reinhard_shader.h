#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include <wrl/client.h>

class reinhard_shader
{
private:
	struct ToneMapBufferType
	{
		float exposure;
		float averageLuminance;
		float maxLuminance;
		float burn;
		float padding[4];
	};
public:
	reinhard_shader(ID3D11Device* device, HWND hwnd);
	~reinhard_shader();
	bool render(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, float exposure, float averageLuminance, float maxLuminance, float burn);

private:
	void output_shader_error_message(ID3D10Blob*, HWND, WCHAR*);

	bool set_shader_parameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* texture, float exposure, float averageLuminance, float maxLuminance, float burn);
	void render_shader(ID3D11DeviceContext* deviceContext);
	bool initialize_shader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_toneMapBuffer;
};