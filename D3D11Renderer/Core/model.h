#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl/client.h>
#include "texture.h"
#include <memory>

class model
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
	};
public:
	model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename);
	~model();

	void render(ID3D11DeviceContext*);
	int get_index_count();

	ID3D11ShaderResourceView* get_texture();

private:
	bool initialize_buffers(ID3D11Device*);
	void render_buffers(ID3D11DeviceContext*);

	bool load_texture(ID3D11Device*, ID3D11DeviceContext*, const wchar_t* filename);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
	std::shared_ptr<texture> m_texture;
	int m_vertexCount, m_indexCount;

};
