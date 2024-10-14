#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <wrl/client.h>
#include <memory>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "texture.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


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
	struct SubMesh
	{
		int startIndex;
		int indexCount;
		std::shared_ptr<texture> texture;
	};


	model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelfilename, const char* mtlbasepath);
	~model();

	void render(ID3D11DeviceContext*);
	const std::vector<SubMesh>& get_sub_meshes() const;

private:
	bool initialize_buffers(ID3D11Device*);
	void render_buffers(ID3D11DeviceContext*);

	bool load_texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const aiScene* scene, const char* textureBasePath);
	bool load_model(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelfilename, const char* mtlPath);
	void process_node(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiNode* node, const aiScene* scene);
	void process_mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, aiMesh* mesh, const aiScene* scene);

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
	std::vector<VertexType> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<SubMesh> m_submeshes;

	// A map from material name to texture resource
	std::unordered_map<std::string, std::shared_ptr<texture>> m_textures;
};
