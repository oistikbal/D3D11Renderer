#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include <wrl/client.h>

class light_shader
{

private:
    struct MatrixBufferType
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    struct CameraBufferType
    {
        DirectX::XMFLOAT3 cameraPosition;
        float padding;
    };

    struct LightBufferType
    {
        DirectX::XMFLOAT4 ambientColor;
        DirectX::XMFLOAT4 diffuseColor;
        DirectX::XMFLOAT3 lightDirection;
        float specularPower;
        DirectX::XMFLOAT4 specularColor;
    };

public:
    light_shader(ID3D11Device* device, HWND hwnd);
	~light_shader();
    bool render(ID3D11DeviceContext* deviceContext, int indexCount, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix,DirectX::XMMATRIX projectionMatrix, 
        ID3D11ShaderResourceView* diffuse, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* specular, ID3D11ShaderResourceView* ao, ID3D11ShaderResourceView* emissive, ID3D11ShaderResourceView* metal,
        DirectX::XMFLOAT3 lightDirection, DirectX::XMFLOAT4 diffuseColor,
        DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT3 cameraPosition, DirectX::XMFLOAT4 specularColor, float specularPower);
private:
    void output_shader_error_message(ID3D10Blob*, HWND, WCHAR*);

    bool set_shader_parameters(ID3D11DeviceContext* deviceContext, DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix,
        ID3D11ShaderResourceView* diffuse, ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* specular, ID3D11ShaderResourceView* ao, ID3D11ShaderResourceView* emissive, ID3D11ShaderResourceView* metal,
        DirectX::XMFLOAT3 lightDirection, DirectX::XMFLOAT4 diffuseColor, DirectX::XMFLOAT4 ambientColor, DirectX::XMFLOAT3 cameraPosition, DirectX::XMFLOAT4 specularColor, float specularPower);
    void render_shader(ID3D11DeviceContext* deviceContext, int indexCount);
    bool initialize_shader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename);
private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_matrixBuffer;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_sampleState;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_cameraBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_lightBuffer;
};
