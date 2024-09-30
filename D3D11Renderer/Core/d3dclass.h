#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

namespace d3d11renderer 
{
	class d3dclass
	{
	public:
		d3dclass(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear);
		~d3dclass();

		void begin_scene(float red, float green, float blue, float alpha);
		void end_scene();

		ID3D11Device* get_device() const;
		ID3D11DeviceContext* get_device_context() const;

		void get_projection_matrix(DirectX::XMMATRIX& projectionMatrix);
		void get_world_matrix(DirectX::XMMATRIX& worldMatrix);
		void get_ortho_matrix(DirectX::XMMATRIX& orthoMatrix);

		void get_video_card_info(char* cardName, int& memory);

		void set_back_buffer_render_target();
		void reset_viewport();
		bool is_initialized() const;

	private:
		bool m_isInitialized;
		bool m_vsync_enabled;
		int m_videoCardMemory;
		char m_videoCardDescription[128];
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;
		Microsoft::WRL::ComPtr<ID3D11Device> m_device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
		DirectX::XMMATRIX m_projectionMatrix;
		DirectX::XMMATRIX m_worldMatrix;
		DirectX::XMMATRIX m_orthoMatrix;
		D3D11_VIEWPORT m_viewport;
	};

}