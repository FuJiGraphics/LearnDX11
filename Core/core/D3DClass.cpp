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
	float fieldOfView, screenAspect;

	// Init
	displayModeList = nullptr;
	numModes = 0, i = 0, numerator = 0, denominator = 0;
	error = 0, fieldOfView = 0, screenAspect = 0;

	// Store the vsync setting.
	m_VSyncEnabled = vsync;

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

	// 어댑터 출력(모니터)에 대한 DXGI_FORMAT_R8G8B8A8_UNORM 
	// 디스플레이 형식에 맞는 모드 수를 가져옵니다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 
		DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// 이 모니터/비디오 카드 조합에 가능한 모든 디스플레이 모드를 담을 목록을 만듭니다.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (displayModeList == nullptr)
	{
		return (false);
	}

	// 이제 디스플레이 모드 목록 구조체를 채웁니다.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);

	// 이제 모든 디스플레이 모드를 살펴보고 화면 너비와 높이가 일치하는 모드를 찾습니다.
	// 일치하는 모드를 찾으면 해당 모니터의 재생률의 분자와 분모를 저장합니다.
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

	// 전용 비디오 카드 메모리를 메가바이트 단위로 저장합니다.
	m_VideoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// 비디오 카드의 이름을 문자 배열로 변환하여 저장합니다.
	error = wcstombs_s(&stringLength, m_VideoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return (false);
	}
	return (true);

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

	// 멀티 샘플링을 끕니다.
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
}

void D3DClass::Shutdown()
{
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

void D3DClass::BeginScene(float, float, float, float)
{
}

void D3DClass::EndScene()
{
}

ID3D11Device* D3DClass::GetDeivce()
{
	return nullptr;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return nullptr;
}

void D3DClass::GetProjectionMatrix(DirectX::XMMATRIX&)
{
}

void D3DClass::GetWorldMatrix(DirectX::XMMATRIX&)
{
}

void D3DClass::GetOrthoMatrix(DirectX::XMMATRIX&)
{
}

void D3DClass::GetVideoCardInfo(char*, int&)
{
}

void D3DClass::SetBackBufferRenderTarger()
{
}

void D3DClass::ResetViewport()
{
}
