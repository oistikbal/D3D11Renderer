#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl/client.h>

class model
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
public:
	model(ID3D11Device*);
	~model();

	void render(ID3D11DeviceContext*);
	int get_index_count();

private:
	bool initialize_buffers(ID3D11Device*);
	void render_buffers(ID3D11DeviceContext*);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
	int m_vertexCount, m_indexCount;
};
