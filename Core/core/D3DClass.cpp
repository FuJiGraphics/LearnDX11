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
	* Direct3D를 초기화하기 전에 비디오 카드/모니터에서 재생률을 가져와야 합니다. 
	* 컴퓨터마다 조금씩 다를 수 있으므로 해당 정보를 쿼리해야 합니다. 
	* 분자와 분모 값을 쿼리한 다음 설정 중에 분자와 분모 값을 DirectX에 
	* 전달하면 적절한 새로 고침 빈도를 계산합니다. 이 작업을 수행하지 않고 
	* 모든 컴퓨터에 존재하지 않을 수 있는 기본값으로 새로 고침 빈도를 설정하면 
	* DirectX는 버퍼 플립 대신 블릿을 수행하여 응답하므로 성능이 저하되고 
	* 디버그 출력에 성가신 오류가 발생합니다.
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

	// 스캔 라인 순서 및 배율을 지정되지 않음으로 설정합니다
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 프레젠테이션 후에는 백 버퍼 내용을 폐기합니다.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 고급 플래그 설정하지 않기
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
	// DepthWriteMask는 D3D11_DEPTH_WRITE_ZERO or ALL로 나뉩니다.
	// ZERO는 깊이 버퍼를 갱신하지 않고 대신 판정은 계속합니다.
	// ALL은 깊이버퍼와 스텐실버퍼를 모두 통과한 픽셀의 깊이가 
	// 깊이버퍼에 새로 갱신됩니다.
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	// 원본 데이터가 대상 데이터보다 작으면 비교가 통과됩니다.
	// 깊이 값이 작을수록 가까이 있다.
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 픽셀이 뒷면을 향하는 경우 스텐실 작업
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
	* 깊이 스텐실 상태가 생성되었으므로 이제 적용되도록 설정할 수 있습니다. 
	* 디바이스 컨텍스트를 사용하여 설정합니다.
	/*-------------------------------------------------------------------------------------------*/
	// Set the depth stencil state.
	m_DeviceContext->OMSetDepthStencilState(m_DepthStencilState, 1);



	/*-------------------------------------------------------------------------------------------
	* 다음으로 만들어야 할 것은 뎁스 스텐실 버퍼의 뷰에 대한 설명입니다. 
	* 이렇게 하면 Direct3D가 뎁스 버퍼를 뎁스 스텐실 텍스처로 사용한다는 것을 알 수 있습니다. 
	* 설명을 작성한 후 CreateDepthStencilView 함수를 호출하여 생성합니다.
	-------------------------------------------------------------------------------------------*/
	// 뎁스 스텐실 뷰를 초기화합니다.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// 뎁스 스텐실 뷰의 구조체를 설정합니다.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// 뎁스 스텐실 뷰를 만듭니다.
	result = m_Device->CreateDepthStencilView(m_DepthStencilBuffer, &depthStencilViewDesc, &m_DepthStencilView);
	if (FAILED(result))
	{
		return (false);
	}

	/*-------------------------------------------------------------------------------------------
	* 이제 생성된 렌더 타깃을 사용하여 OMSetRenderTargets를 호출할 수 있습니다. 
	* 그러면 렌더 대상 뷰와 뎁스 스텐실 버퍼가 출력 렌더 파이프라인에 바인딩됩니다. 
	* 이렇게 하면 파이프라인이 렌더링하는 그래픽이 이전에 생성한 백 버퍼에 그려집니다. 
	* 그래픽이 백 버퍼에 쓰여지면 이를 앞쪽으로 바꾸어 사용자 화면에 그래픽을 표시할 수 있습니다.
	-------------------------------------------------------------------------------------------*/
	// 출력 렌더 파이프라인에 뎁스 스텐실 버퍼와 렌더 타겟을 바인딩합니다.
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);
	

	/*-------------------------------------------------------------------------------------------
	* 이제 렌더 타깃이 설정되었으므로 향후 튜토리얼에서 씬을 더 잘 제어할 수 있는 
	* 몇 가지 추가 함수를 계속 살펴볼 수 있습니다. 먼저 래스터라이저 상태를 만들겠습니다. 
	* 이를 통해 다각형이 렌더링되는 방식을 제어할 수 있습니다. 씬을 와이어프레임 모드로 
	* 렌더링하거나 DirectX가 다각형의 앞면과 뒷면을 모두 그리도록 하는 등의 작업을 수행할 수 있습니다. 
	* 기본적으로 DirectX에는 래스터라이저 상태가 이미 설정되어 있고 아래와 똑같이 작동하지만 
	* 직접 설정하지 않는 한 변경은 할 수 없습니다.
	-------------------------------------------------------------------------------------------*/
	// 래스터 구조체를 설정하면 어떤 다각형을 어떻게 그릴지 결정할 수 있습니다.
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

	// 방금 작성한 구조체로 래스터라이저 상태를 만듭니다.
	result = m_Device->CreateRasterizerState(&rasterDesc, &m_RasterState);
	if (FAILED(result))
	{
		return (false);
	}

	// 이제 레스터라이저 상태를 세팅합니다.
	m_DeviceContext->RSSetState(m_RasterState);

	/*-------------------------------------------------------------------------------------------
	* 또한 Direct3D가 클립 공간 좌표를 렌더링 대상 공간에 매핑할 수 있도록 
	* 뷰포트를 설정해야 합니다. 이 값을 창의 전체 크기로 설정합니다.
	-------------------------------------------------------------------------------------------*/
	// 렌더링을 위한 뷰포트를 설정합니다.
	m_Viewport.Width = static_cast<float>(screenWidth);
	m_Viewport.Height = static_cast<float>(screenHeight);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;

	// 뷰포트를 만듭니다.
	m_DeviceContext->RSSetViewports(1, &m_Viewport);


	/*-------------------------------------------------------------------------------------------
	* 이제 투영 행렬을 생성합니다. 
	* 투영 행렬은 3D 장면을 이전에 생성한 2D 뷰포트 공간으로 변환하는 데 사용됩니다. 
	* 씬을 렌더링하는 데 사용할 셰이더에 전달할 수 있도록 이 매트릭스의 복사본을 보관해야 합니다.
	-------------------------------------------------------------------------------------------*/
	// 투영 행렬을 설정합니다.
	fieldOfView = 3.141592654f / 4.0f; // 라디안의 45도로 변환
	screenAspect = m_Viewport.Width / m_Viewport.Height;

	// 3D 렌더링을 위한 투영 행렬을 만듭니다.
	m_ProjectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);


	/*-------------------------------------------------------------------------------------------
	* 또한 월드 매트릭스라는 또 다른 행렬을 생성합니다. 
	* 이 행렬은 오브젝트의 정점을 3D 씬의 정점으로 변환하는 데 사용됩니다. 
	* 이 행렬은 3D 공간에서 오브젝트를 회전, 이동 및 크기 조정하는 데도 사용됩니다. 
	* 처음부터 매트릭스를 아이덴티티 매트릭스로 초기화하고 이 오브젝트에 복사본을 보관합니다. 
	* 이 복사본은 렌더링을 위해 셰이더에도 전달해야 합니다.
	-------------------------------------------------------------------------------------------*/
	// 단위행렬로 월드 행렬을 일시적으로 초기화합니다.
	m_WorldMatrix = XMMatrixIdentity();


	/*-------------------------------------------------------------------------------------------
	*이 부분에서는 일반적으로 뷰 행렬을 생성합니다. 
	* 뷰 행렬은 우리가 장면을 어떤 위치에서 바라보는지를 계산하는 데 사용됩니다. 
	* 이것을 카메라처럼 생각할 수 있으며, 이 카메라를 통해서만 장면을 볼 수 있습니다. 
	* 그 목적 때문에 뷰 행렬을 나중에 카메라 클래스에서 생성할 것입니다. 
	* 논리적으로 그곳에 더 잘 맞기 때문에 다음 튜토리얼에서 다룰 것이며, 
	* 지금은 그냥 넘어가겠습니다.
    * 그리고 초기화 함수에서 설정할 마지막 항목은 직교 투영 행렬입니다. 
	* 이 행렬은 사용자 인터페이스와 같은 2D 요소들을 화면에 렌더링하는 데 사용되며, 
	* 이를 통해 3D 렌더링을 건너뛸 수 있습니다. 2D 그래픽과 글꼴을 화면에 렌더링하는 방법을 
	* 다룰 다음 튜토리얼에서 이것을 사용할 것입니다.
	-------------------------------------------------------------------------------------------*/
	// 2D 사용자 인터페이스 UI를 위한 직교 행렬을 만듭니다. 
	m_OrthoMatrix = XMMatrixOrthographicLH(m_Viewport.Width, m_Viewport.Height, screenNear, screenDepth);

	return (true);
}

void D3DClass::Shutdown()
{
	/*-------------------------------------------------------------------------------------------
	* 셧다운 함수는 초기화 함수에 사용된 모든 포인터를 해제하고 정리하며, 이는 매우 간단합니다. 
	* 하지만 그 전에 포인터를 해제하기 전에 먼저 스왑 체인을 강제로 창 모드로 전환하도록 호출을 넣었습니다. 
	* 이 작업을 수행하지 않고 전체 화면 모드에서 스왑 체인을 해제하려고 하면 몇 가지 예외가 발생합니다. 
	* 따라서 이를 방지하기 위해 항상 Direct3D를 종료하기 전에 창 모드로 강제 전환합니다.
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
	// Clear Color를 설정합니다.
	float clearColor[4] = { r, g, b, a };

	// 백 버퍼를 클리어 합니다.
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView, clearColor);

	// 깊이 버퍼를 클리어 합니다.
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);

	return;
}

void D3DClass::EndScene()
{
	// 렌더링이 완료되었으므로 백 버퍼를 화면에 표시합니다.
	if (m_VSyncEnabled)
	{
		// 화면 새로고침 빈도로 고정합니다.
		m_SwapChain->Present(1, 0);
	}
	else
	{
		// 가능한 한 빨리 프레젠테이션합니다.
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
	// 렌더 타깃 뷰와 뎁스 스텐실 버퍼를 출력 렌더 파이프라인에 바인딩합니다.
	m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);

	return;
}

void D3DClass::ResetViewport()
{
	// 뷰포트를 설정합니다.
	m_DeviceContext->RSSetViewports(1, &m_Viewport);

	return;
}
