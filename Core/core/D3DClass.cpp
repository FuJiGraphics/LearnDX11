#include "D3DClass.h"

// for DirectX11 Math
using namespace DirectX;

D3DClass::D3DClass()
	: m_VSyncEnabled(false)
	, m_VideoCardMemory(0)
	, m_VideoCardDescription()
	, m_SwapChain(nullptr)
	, m_Device(nullptr)
	, m_DeviceContext(nullptr)
	, m_RenderTargetView(nullptr)
	, m_DepthStencilBuffer(nullptr)
	, m_DepthStencilState(nullptr)
	, m_DepthStencilView(nullptr)
	, m_RasterState(nullptr)
	, m_ProjectionMatrix(DirectX::XMMatrixIdentity())
	, m_WorldMatrix(DirectX::XMMatrixIdentity())
	, m_OrthoMatrix(DirectX::XMMatrixIdentity())
	, m_Viewport{ 0, 0, 0, 0, 0, 0 }
{
}

D3DClass::D3DClass(const D3DClass&)
	: m_VSyncEnabled(false)
	, m_VideoCardMemory(0)
	, m_VideoCardDescription()
	, m_SwapChain(nullptr)
	, m_Device(nullptr)
	, m_DeviceContext(nullptr)
	, m_RenderTargetView(nullptr)
	, m_DepthStencilBuffer(nullptr)
	, m_DepthStencilState(nullptr)
	, m_DepthStencilView(nullptr)
	, m_RasterState(nullptr)
	, m_ProjectionMatrix(DirectX::XMMatrixIdentity())
	, m_WorldMatrix(DirectX::XMMatrixIdentity())
	, m_OrthoMatrix(DirectX::XMMatrixIdentity())
	, m_Viewport{ 0, 0, 0, 0, 0, 0 }
{
}

D3DClass::~D3DClass()
{
}

bool D3DClass::Initialize(
	int screenWidth, int screenHeight, 
	bool vsync, HWND hwnd, bool fullscreen, 
	float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	float fieldOfView, screenAspect;

	// Init
	displayModeList = nullptr;
	numModes = 0, i = 0, numerator = 0, denominator = 0;
	error = 0, fieldOfView = 0, screenAspect = 0;

	// Store the vsync setting.
	m_VSyncEnabled = vsync;

	/*-------------------------------------------------------------------------------------------
	* Direct3D�� �ʱ�ȭ�ϱ� ���� ���� ī��/����Ϳ��� ������� �����;� �մϴ�. 
	* ��ǻ�͸��� ���ݾ� �ٸ� �� �����Ƿ� �ش� ������ �����ؾ� �մϴ�. 
	* ���ڿ� �и� ���� ������ ���� ���� �߿� ���ڿ� �и� ���� DirectX�� 
	* �����ϸ� ������ ���� ��ħ �󵵸� ����մϴ�. �� �۾��� �������� �ʰ� 
	* ��� ��ǻ�Ϳ� �������� ���� �� �ִ� �⺻������ ���� ��ħ �󵵸� �����ϸ� 
	* DirectX�� ���� �ø� ��� ���� �����Ͽ� �����ϹǷ� ������ ���ϵǰ� 
	* ����� ��¿� ������ ������ �߻��մϴ�.
	-------------------------------------------------------------------------------------------*/
	// Create a DirectX graphics interface factory
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return (false);
	}

	// Use the factory to create an adapter 
	// for the primary graphics interface(video card)
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return (false);
	}

	// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return (false);
	}

	// ����� ���(�����)�� ���� DXGI_FORMAT_R8G8B8A8_UNORM 
	// ���÷��� ���Ŀ� �´� ��� ���� �����ɴϴ�.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 
		DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// �� �����/���� ī�� ���տ� ������ ��� ���÷��� ��带 ���� ����� ����ϴ�.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (displayModeList == nullptr)
	{
		return (false);
	}

	// ���� ���÷��� ��� ��� ����ü�� ä��ϴ�.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);

	// ���� ��� ���÷��� ��带 ���캸�� ȭ�� �ʺ�� ���̰� ��ġ�ϴ� ��带 ã���ϴ�.
	// ��ġ�ϴ� ��带 ã���� �ش� ������� ������� ���ڿ� �и� �����մϴ�.
	for (int i = 0; i < numModes; ++i)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return (false);
	}

	// ���� ���� ī�� �޸𸮸� �ް�����Ʈ ������ �����մϴ�.
	m_VideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// ���� ī���� �̸��� ���� �迭�� ��ȯ�Ͽ� �����մϴ�.
	error = wcstombs_s(&stringLength, m_VideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return (false);
	}

	// Release the display mode list.
	delete[] displayModeList;
	displayModeList = nullptr;

	// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = nullptr;

	// Release the adapter.
	adapter->Release();
	adapter = 0;

	// Release the factory.
	factory->Release();
	factory = nullptr;

	// Initialize the swap chain description
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set to a single back buffer
	swapChainDesc.BufferCount = 1;

	// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

	// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_VSyncEnabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	// Set the usage of the back buffer
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

	// ��Ƽ ���ø��� ���ϴ�.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// Set to full screen or windowed mode.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// ��ĵ ���� ���� �� ������ �������� �������� �����մϴ�
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// ���������̼� �Ŀ��� �� ���� ������ ����մϴ�.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// ��� �÷��� �������� �ʱ�
	swapChainDesc.Flags = 0;

	// Set the feature level to DirectX 11.
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		0,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&swapChainDesc,
		&m_SwapChain,
		&m_Device,
		NULL,
		&m_DeviceContext
	);
	if (FAILED(result))
	{
		return (false);
	}

	// Get the pointer to the back buffer.
	result = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return (false);
	}

	// Create the render target view with the back buffer pointer.
	result = m_Device->CreateRenderTargetView(backBufferPtr, NULL, &m_RenderTargetView);
	if (FAILED(result))
	{
		return (false);
	}

	// Release pointer to the back buffer as we no longer need it
	backBufferPtr->Release();
	backBufferPtr = nullptr;

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	result = m_Device->CreateTexture2D(&depthBufferDesc, NULL, &m_DepthStencilBuffer);
	if (FAILED(result))
	{
		return (false);
	}

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	// DepthWriteMask�� D3D11_DEPTH_WRITE_ZERO or ALL�� �����ϴ�.
	// ZERO�� ���� ���۸� �������� �ʰ� ��� ������ ����մϴ�.
	// ALL�� ���̹��ۿ� ���ٽǹ��۸� ��� ����� �ȼ��� ���̰� 
	// ���̹��ۿ� ���� ���ŵ˴ϴ�.
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	// ���� �����Ͱ� ��� �����ͺ��� ������ �񱳰� ����˴ϴ�.
	// ���� ���� �������� ������ �ִ�.
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// �ȼ��� �޸��� ���ϴ� ��� ���ٽ� �۾�
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStencilState);
	if (FAILED(result))
	{
		return (false);
	}


	/*-------------------------------------------------------------------------------------------
	* ���� ���ٽ� ���°� �����Ǿ����Ƿ� ���� ����ǵ��� ������ �� �ֽ��ϴ�. 
	* ����̽� ���ؽ�Ʈ�� ����Ͽ� �����մϴ�.
	/*-------------------------------------------------------------------------------------------*/
	// Set the depth stencil state.
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState, 1);



	/*-------------------------------------------------------------------------------------------
	* �������� ������ �� ���� ���� ���ٽ� ������ �信 ���� �����Դϴ�. 
	* �̷��� �ϸ� Direct3D�� ���� ���۸� ���� ���ٽ� �ؽ�ó�� ����Ѵٴ� ���� �� �� �ֽ��ϴ�. 
	* ������ �ۼ��� �� CreateDepthStencilView �Լ��� ȣ���Ͽ� �����մϴ�.
	-------------------------------------------------------------------------------------------*/
	// ���� ���ٽ� �並 �ʱ�ȭ�մϴ�.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// ���� ���ٽ� ���� ����ü�� �����մϴ�.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// ���� ���ٽ� �並 ����ϴ�.
	result = m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &depthStencilViewDesc, &m_DepthStencilView);
	if (FAILED(result))
	{
		return (false);
	}

	/*-------------------------------------------------------------------------------------------
	* ���� ������ ���� Ÿ���� ����Ͽ� OMSetRenderTargets�� ȣ���� �� �ֽ��ϴ�. 
	* �׷��� ���� ��� ��� ���� ���ٽ� ���۰� ��� ���� ���������ο� ���ε��˴ϴ�. 
	* �̷��� �ϸ� ������������ �������ϴ� �׷����� ������ ������ �� ���ۿ� �׷����ϴ�. 
	* �׷����� �� ���ۿ� �������� �̸� �������� �ٲپ� ����� ȭ�鿡 �׷����� ǥ���� �� �ֽ��ϴ�.
	-------------------------------------------------------------------------------------------*/
	// ��� ���� ���������ο� ���� ���ٽ� ���ۿ� ���� Ÿ���� ���ε��մϴ�.
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);
	

	/*-------------------------------------------------------------------------------------------
	* ���� ���� Ÿ���� �����Ǿ����Ƿ� ���� Ʃ�丮�󿡼� ���� �� �� ������ �� �ִ� 
	* �� ���� �߰� �Լ��� ��� ���캼 �� �ֽ��ϴ�. ���� �����Ͷ����� ���¸� ����ڽ��ϴ�. 
	* �̸� ���� �ٰ����� �������Ǵ� ����� ������ �� �ֽ��ϴ�. ���� ���̾������� ���� 
	* �������ϰų� DirectX�� �ٰ����� �ո�� �޸��� ��� �׸����� �ϴ� ���� �۾��� ������ �� �ֽ��ϴ�. 
	* �⺻������ DirectX���� �����Ͷ����� ���°� �̹� �����Ǿ� �ְ� �Ʒ��� �Ȱ��� �۵������� 
	* ���� �������� �ʴ� �� ������ �� �� �����ϴ�.
	-------------------------------------------------------------------------------------------*/
	// ������ ����ü�� �����ϸ� � �ٰ����� ��� �׸��� ������ �� �ֽ��ϴ�.
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

	// ��� �ۼ��� ����ü�� �����Ͷ����� ���¸� ����ϴ�.
	result = m_Device->CreateRasterizerState(&rasterDesc, &m_RasterState);
	if (FAILED(result))
	{
		return (false);
	}

	// ���� �����Ͷ����� ���¸� �����մϴ�.
	m_DeviceContext->RSSetState(m_RasterState);

	/*-------------------------------------------------------------------------------------------
	* ���� Direct3D�� Ŭ�� ���� ��ǥ�� ������ ��� ������ ������ �� �ֵ��� 
	* ����Ʈ�� �����ؾ� �մϴ�. �� ���� â�� ��ü ũ��� �����մϴ�.
	-------------------------------------------------------------------------------------------*/
	// �������� ���� ����Ʈ�� �����մϴ�.
	m_Viewport.Width = static_cast<float>(screenWidth);
	m_Viewport.Height = static_cast<float>(screenHeight);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;

	// ����Ʈ�� ����ϴ�.
	m_DeviceContext->RSSetViewports(1, &m_Viewport);


	/*-------------------------------------------------------------------------------------------
	* ���� ���� ����� �����մϴ�. 
	* ���� ����� 3D ����� ������ ������ 2D ����Ʈ �������� ��ȯ�ϴ� �� ���˴ϴ�. 
	* ���� �������ϴ� �� ����� ���̴��� ������ �� �ֵ��� �� ��Ʈ������ ���纻�� �����ؾ� �մϴ�.
	-------------------------------------------------------------------------------------------*/
	// ���� ����� �����մϴ�.
	fieldOfView = 3.141592654f / 4.0f; // ������ 45���� ��ȯ
	screenAspect = m_Viewport.Width / m_Viewport.Height;

	// 3D �������� ���� ���� ����� ����ϴ�.
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);


	/*-------------------------------------------------------------------------------------------
	* ���� ���� ��Ʈ������� �� �ٸ� ����� �����մϴ�. 
	* �� ����� ������Ʈ�� ������ 3D ���� �������� ��ȯ�ϴ� �� ���˴ϴ�. 
	* �� ����� 3D �������� ������Ʈ�� ȸ��, �̵� �� ũ�� �����ϴ� ���� ���˴ϴ�. 
	* ó������ ��Ʈ������ ���̵�ƼƼ ��Ʈ������ �ʱ�ȭ�ϰ� �� ������Ʈ�� ���纻�� �����մϴ�. 
	* �� ���纻�� �������� ���� ���̴����� �����ؾ� �մϴ�.
	-------------------------------------------------------------------------------------------*/
	// ������ķ� ���� ����� �Ͻ������� �ʱ�ȭ�մϴ�.
	m_WorldMatrix = XMMatrixIdentity();


	/*-------------------------------------------------------------------------------------------
	*�� �κп����� �Ϲ������� �� ����� �����մϴ�. 
	* �� ����� �츮�� ����� � ��ġ���� �ٶ󺸴����� ����ϴ� �� ���˴ϴ�. 
	* �̰��� ī�޶�ó�� ������ �� ������, �� ī�޶� ���ؼ��� ����� �� �� �ֽ��ϴ�. 
	* �� ���� ������ �� ����� ���߿� ī�޶� Ŭ�������� ������ ���Դϴ�. 
	* �������� �װ��� �� �� �±� ������ ���� Ʃ�丮�󿡼� �ٷ� ���̸�, 
	* ������ �׳� �Ѿ�ڽ��ϴ�.
    * �׸��� �ʱ�ȭ �Լ����� ������ ������ �׸��� ���� ���� ����Դϴ�. 
	* �� ����� ����� �������̽��� ���� 2D ��ҵ��� ȭ�鿡 �������ϴ� �� ���Ǹ�, 
	* �̸� ���� 3D �������� �ǳʶ� �� �ֽ��ϴ�. 2D �׷��Ȱ� �۲��� ȭ�鿡 �������ϴ� ����� 
	* �ٷ� ���� Ʃ�丮�󿡼� �̰��� ����� ���Դϴ�.
	-------------------------------------------------------------------------------------------*/
	// 2D ����� �������̽� UI�� ���� ���� ����� ����ϴ�. 
	m_OrthoMatrix = XMMatrixOrthographicLH(m_Viewport.Width, m_Viewport.Height, screenNear, screenDepth);

	return (true);
}

void D3DClass::Shutdown()
{
	/*-------------------------------------------------------------------------------------------
	* �˴ٿ� �Լ��� �ʱ�ȭ �Լ��� ���� ��� �����͸� �����ϰ� �����ϸ�, �̴� �ſ� �����մϴ�. 
	* ������ �� ���� �����͸� �����ϱ� ���� ���� ���� ü���� ������ â ���� ��ȯ�ϵ��� ȣ���� �־����ϴ�. 
	* �� �۾��� �������� �ʰ� ��ü ȭ�� ��忡�� ���� ü���� �����Ϸ��� �ϸ� �� ���� ���ܰ� �߻��մϴ�. 
	* ���� �̸� �����ϱ� ���� �׻� Direct3D�� �����ϱ� ���� â ���� ���� ��ȯ�մϴ�.
	-------------------------------------------------------------------------------------------*/
	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, NULL);
	}
	if (m_RasterState)
	{
		m_RasterState->Release();
		m_RasterState = nullptr;
	}
	if (m_DepthStencilView)
	{
		m_DepthStencilView->Release();
		m_DepthStencilView = nullptr;
	}
	if (m_DepthStencilState)
	{
		m_DepthStencilState->Release();
		m_DepthStencilState = nullptr;
	}
	if (m_DepthStencilBuffer)
	{
		m_DepthStencilBuffer->Release();
		m_DepthStencilBuffer = nullptr;
	}
	if (m_RenderTargetView)
	{
		m_RenderTargetView->Release();
		m_RenderTargetView = nullptr;
	}
	if (m_DeviceContext)
	{
		m_DeviceContext->Release();
		m_DeviceContext = nullptr;
	}
	if (m_Device)
	{
		m_Device->Release();
		m_Device = nullptr;
	}
	if (m_SwapChain)
	{
		m_SwapChain->Release();
		m_SwapChain = nullptr;
	}
}

void D3DClass::BeginScene(float r, float g, float b, float a)
{
	// Clear Color�� �����մϴ�.
	float clearColor[4] = { r, g, b, a };

	// �� ���۸� Ŭ���� �մϴ�.
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, clearColor);

	// ���� ���۸� Ŭ���� �մϴ�.
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);

	return;
}

void D3DClass::EndScene()
{
	// �������� �Ϸ�Ǿ����Ƿ� �� ���۸� ȭ�鿡 ǥ���մϴ�.
	if (m_VSyncEnabled)
	{
		// ȭ�� ���ΰ�ħ �󵵷� �����մϴ�.
		m_SwapChain->Present(1, 0);
	}
	else
	{
		// ������ �� ���� ���������̼��մϴ�.
		m_SwapChain->Present(0, 0);
	}

}

ID3D11Device* D3DClass::GetDeivce()
{
	return (m_Device);
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return (m_DeviceContext);
}

void D3DClass::GetProjectionMatrix(DirectX::XMMATRIX& ref_out)
{
	ref_out = m_ProjectionMatrix;
}

void D3DClass::GetWorldMatrix(DirectX::XMMATRIX& ref_out)
{
	ref_out = m_WorldMatrix;
}

void D3DClass::GetOrthoMatrix(DirectX::XMMATRIX& ref_out)
{
	ref_out = m_OrthoMatrix;
}

void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_VideoCardDescription);
	memory = m_VideoCardMemory;
	return;
}

void D3DClass::SetBackBufferRenderTarger()
{
	// ���� Ÿ�� ��� ���� ���ٽ� ���۸� ��� ���� ���������ο� ���ε��մϴ�.
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	return;
}

void D3DClass::ResetViewport()
{
	// ����Ʈ�� �����մϴ�.
	m_DeviceContext->RSSetViewports(1, &m_Viewport);

	return;
}
