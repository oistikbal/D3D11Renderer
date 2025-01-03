#include <stdexcept>
#include <string>
#include "d3dclass.h"


using namespace Microsoft::WRL;

d3d11renderer::d3dclass::d3dclass(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	ComPtr<IDXGIFactory> factory;
	ComPtr<IDXGIAdapter> adapter;
	ComPtr<IDXGIOutput> adapterOutput;
	unsigned int numModes = 0, i = 0, numerator = 0, denominator = 0;
	unsigned long long stringLength = 0;
	DXGI_MODE_DESC* displayModeList = nullptr;
	DXGI_ADAPTER_DESC adapterDesc = {};
	int error = 0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	D3D_FEATURE_LEVEL featureLevel;
	ComPtr<ID3D11Texture2D> backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc = {};
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	D3D11_RASTERIZER_DESC rasterDesc = {};
	float fieldOfView = 0.0f, screenAspect = 0.0f;


	m_vsync_enabled = vsync;

	// Create DXGI Factory
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(factory.GetAddressOf()));
	if (FAILED(result))
		throw std::runtime_error("Failed to create DXGI Factory");

	// Enumerate adapters
	result = factory->EnumAdapters(0, adapter.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to enumerate adapters");

	// Enumerate outputs
	result = adapter->EnumOutputs(0, adapterOutput.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to enumerate outputs");

	// Get display mode list
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
		throw std::runtime_error("Failed to get display mode list");

	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
		throw std::runtime_error("Failed to allocate memory for display mode list");

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
		throw std::runtime_error("Failed to get display mode list with details");



	// Find matching refresh rate and resolution
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth && displayModeList[i].Height == (unsigned int)screenHeight)
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
		}
	}

	// Get adapter description
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
		throw std::runtime_error("Failed to get adapter description");

	// Store video card memory and description
	m_videoCardMemory = static_cast<int>(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	DXGI_OUTPUT_DESC outputDesc;
	adapterOutput->GetDesc(&outputDesc);

	int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, adapterDesc.Description, -1, nullptr, 0, nullptr, nullptr);
	m_videoCardDescription.resize(sizeNeeded);
	WideCharToMultiByte(CP_UTF8, 0, adapterDesc.Description, -1, &m_videoCardDescription[0], sizeNeeded, nullptr, nullptr);


	delete[] displayModeList;
	displayModeList = nullptr;


	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	// The width and height of the renderable area
	int renderWidth = clientRect.right - clientRect.left;
	int renderHeight = clientRect.bottom - clientRect.top;

	// Initialize swap chain description
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = renderWidth;
	swapChainDesc.BufferDesc.Height = renderHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.SampleDesc.Count = 4;
	swapChainDesc.SampleDesc.Quality = 1;
	swapChainDesc.Windowed = !fullscreen;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_1;

	// Create Device and Swap Chain
	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, m_swapChain.GetAddressOf(), m_device.GetAddressOf(), nullptr, m_deviceContext.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create device and swap chain");

	// Get back buffer
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBufferPtr.GetAddressOf()));
	if (FAILED(result))
		throw std::runtime_error("Failed to get back buffer");

	// Create render target view
	result = m_device->CreateRenderTargetView(backBufferPtr.Get(), nullptr, m_renderTargetView.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create render target view");
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = renderWidth;
	depthBufferDesc.Height = renderHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 4;
	depthBufferDesc.SampleDesc.Quality = 1;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create depth stencil buffer
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, m_depthStencilBuffer.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create depth stencil buffer");

	// Initialize depth stencil state
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;


	result = m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create depth stencil state");

	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	result = m_device->CreateDepthStencilState(&depthStencilDesc, m_skyboxDepthStencilState.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create sky depth stencil state");

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	// Create depth stencil view
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create depth stencil view");

	result = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_skyboxDepthStencilView.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create sky depth stencil view");

	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());


	// In your d3dclass constructor or initialization method
	D3D11_TEXTURE2D_DESC toneMapTextureDesc = {};
	toneMapTextureDesc.Width = renderWidth;
	toneMapTextureDesc.Height = renderHeight;
	toneMapTextureDesc.MipLevels = 1;
	toneMapTextureDesc.ArraySize = 1;
	toneMapTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // High dynamic range
	toneMapTextureDesc.SampleDesc.Count = 4;
	toneMapTextureDesc.SampleDesc.Quality = 1;
	toneMapTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	toneMapTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	result = m_device->CreateTexture2D(&toneMapTextureDesc, nullptr, m_toneMapTexture.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create tone map texture");

	result = m_device->CreateRenderTargetView(m_toneMapTexture.Get(), nullptr, m_toneMapRTV.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create tone map render target view");

	m_device->CreateShaderResourceView(m_toneMapTexture.Get(), nullptr, m_toneMapSRV.GetAddressOf());

	// Initialize rasterizer state
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = m_device->CreateRasterizerState(&rasterDesc, m_rasterState.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create rasterizer state");

	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.DepthClipEnable = true;

	result = m_device->CreateRasterizerState(&rasterDesc, m_skyRasterState.GetAddressOf());
	if (FAILED(result))
		throw std::runtime_error("Failed to create sky rasterizer state");

	m_deviceContext->RSSetState(m_rasterState.Get());

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	blendDesc.AlphaToCoverageEnable = FALSE; // Do not enable alpha to coverage
	blendDesc.IndependentBlendEnable = FALSE; // Use one blend state for all render targets

	// Set the blend state for render target 0
	D3D11_RENDER_TARGET_BLEND_DESC& rtBlendDesc = blendDesc.RenderTarget[0];
	rtBlendDesc.BlendEnable = TRUE; // Enable blending
	rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA; // Source blend factor
	rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA; // Destination blend factor
	rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD; // Blend operation
	rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE; // Alpha source blend
	rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO; // Alpha destination blend
	rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD; // Alpha blend operation
	rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // Write to all color channels

	// Create the blend state
	result = m_device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
	if (FAILED(result))
	throw std::runtime_error("Failed to create blend state");

	m_viewport.Width = (float)renderWidth;
	m_viewport.Height = (float)renderHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	// Create the viewport.
	m_deviceContext->RSSetViewports(1, &m_viewport);
	fieldOfView = DirectX::XM_PIDIV4; // 45 degrees
	screenAspect = static_cast<float>(renderWidth) / static_cast<float>(renderHeight);
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	m_worldMatrix = DirectX::XMMatrixIdentity();

	m_orthoMatrix = DirectX::XMMatrixOrthographicLH((float)renderWidth, (float)renderHeight, screenNear, screenDepth);


	m_isInitialized = true;
}

d3d11renderer::d3dclass::~d3dclass()
{

}

void d3d11renderer::d3dclass::begin_scene(float red, float green, float blue, float alpha)
{
	static float color[4];


	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;


	m_deviceContext->ClearRenderTargetView(m_toneMapRTV.Get(), color);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), color);
	m_deviceContext->OMSetRenderTargets(1, m_toneMapRTV.GetAddressOf(), m_depthStencilView.Get());

	// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_deviceContext->ClearDepthStencilView(m_skyboxDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	m_deviceContext->OMSetBlendState(m_blendState.Get(), nullptr, 0xffffffff); // Set blend state with no specific blend factor


	return;
}

void d3d11renderer::d3dclass::end_scene()
{
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

void d3d11renderer::d3dclass::present()
{
	if (m_vsync_enabled)
	{
		// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
		// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}

ID3D11Device* d3d11renderer::d3dclass::get_device() const
{
	return m_device.Get();
}

ID3D11DeviceContext* d3d11renderer::d3dclass::get_device_context() const
{
	return m_deviceContext.Get();
}

void d3d11renderer::d3dclass::get_projection_matrix(DirectX::XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}

void d3d11renderer::d3dclass::get_world_matrix(DirectX::XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}

void d3d11renderer::d3dclass::get_ortho_matrix(DirectX::XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}

int d3d11renderer::d3dclass::get_gpu_memory()
{
	return m_videoCardMemory;
}

std::string d3d11renderer::d3dclass::get_gpu_name()
{
	return m_videoCardDescription;
}

void d3d11renderer::d3dclass::set_back_buffer_render_target()
{
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

void d3d11renderer::d3dclass::reset_viewport()
{
	m_deviceContext->RSSetViewports(1, &m_viewport);
	return;
}

void d3d11renderer::d3dclass::resize(int width, int height)
{
	if (width <= 0 || height <= 0) 
		return;

	if (!m_deviceContext) return;

	// Ensure any views or buffers are released before resizing
	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_depthStencilBuffer.Reset();
	m_skyboxDepthStencilView.Reset();
	m_toneMapTexture.Reset();
	m_toneMapRTV.Reset();



	// Resize the swap chain
	HRESULT hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
	if (FAILED(hr))
		throw std::runtime_error("Failed to resize swapChain");


	// In your d3dclass constructor or initialization method
	D3D11_TEXTURE2D_DESC toneMapTextureDesc = {};
	toneMapTextureDesc.Width = width;
	toneMapTextureDesc.Height = height;
	toneMapTextureDesc.MipLevels = 1;
	toneMapTextureDesc.ArraySize = 1;
	toneMapTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // High dynamic range
	toneMapTextureDesc.SampleDesc.Count = 4;
	toneMapTextureDesc.SampleDesc.Quality = 1;
	toneMapTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	toneMapTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	hr = m_device->CreateTexture2D(&toneMapTextureDesc, nullptr, m_toneMapTexture.GetAddressOf());
	if (FAILED(hr))
		throw std::runtime_error("Failed to create tone map texture");

	hr = m_device->CreateRenderTargetView(m_toneMapTexture.Get(), nullptr, m_toneMapRTV.GetAddressOf());
	if (FAILED(hr))
		throw std::runtime_error("Failed to create tone map render target view");

	m_device->CreateShaderResourceView(m_toneMapTexture.Get(), nullptr, m_toneMapSRV.GetAddressOf());

	// Recreate the render target view with the resized back buffer
	ComPtr<ID3D11Texture2D> backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr))
		throw std::runtime_error("Failed to get back buffer");

	hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_renderTargetView.GetAddressOf());
	if (FAILED(hr))
		throw std::runtime_error("Failed to create render target view");

	// Resize the depth/stencil buffer and create a new depth stencil view
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 4;
	depthDesc.SampleDesc.Quality = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = m_device->CreateTexture2D(&depthDesc, nullptr, m_depthStencilBuffer.GetAddressOf());
	if (FAILED(hr)) {
		// Handle error
		return;
	}

	hr = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, m_depthStencilView.GetAddressOf());
	if (FAILED(hr)) {
		// Handle error
		return;
	}

	hr = m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), nullptr, m_skyboxDepthStencilView.GetAddressOf());
	if (FAILED(hr)) {
		// Handle error
		return;
	}


	// Bind the render target view and depth stencil view to the output-merger stage
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	auto fieldOfView = DirectX::XM_PIDIV4; // 45 degrees
	auto screenAspect = static_cast<float>(width) / static_cast<float>(height);
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, 1, 1000);

	// Set the new viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;

	m_deviceContext->RSSetViewports(1, &viewport);
}

bool d3d11renderer::d3dclass::is_initialized() const
{
	return m_isInitialized;
}

void d3d11renderer::d3dclass::set_culling(bool isOpen)
{
	if(isOpen)
		m_deviceContext->RSSetState(m_rasterState.Get());
	else
		m_deviceContext->RSSetState(m_skyRasterState.Get());
}

void d3d11renderer::d3dclass::set_depth(bool isOpen)
{
	if(isOpen)
		m_deviceContext->OMSetRenderTargets(1, m_toneMapRTV.GetAddressOf(), m_depthStencilView.Get());
	else
		m_deviceContext->OMSetRenderTargets(1, m_toneMapRTV.GetAddressOf(), m_skyboxDepthStencilView.Get());
}

ID3D11ShaderResourceView* d3d11renderer::d3dclass::get_tonemap_srv()
{
	return m_toneMapSRV.Get();
}
