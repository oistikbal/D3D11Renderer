#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl/client.h>
#include "texture.h"
#include <memory>
#include <fstream>
#include <vector>

class model
{
private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
public:
	model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* modelfilename, const wchar_t* texturefilename);
	~model();

	void render(ID3D11DeviceContext*);
	int get_index_count();

	ID3D11ShaderResourceView* get_texture();

private:
	bool initialize_buffers(ID3D11Device*);
	void render_buffers(ID3D11DeviceContext*);

	bool load_texture(ID3D11Device*, ID3D11DeviceContext*, const wchar_t* filename);

	bool load_model(const wchar_t* modelfilename);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
	std::shared_ptr<texture> m_texture;
	int m_vertexCount, m_indexCount;
	std::vector<ModelType> m_model;
};
