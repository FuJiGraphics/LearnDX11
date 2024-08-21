#include "ColorShaderClass.h"

ColorShaderClass::ColorShaderClass()
	: m_VertexShader(nullptr)
	, m_PixelShader(nullptr)
	, m_Layout(nullptr)
	, m_MatrixBuffer(nullptr)
{
}

ColorShaderClass::ColorShaderClass(const ColorShaderClass& other)
	: m_VertexShader(nullptr)
	, m_PixelShader(nullptr)
	, m_Layout(nullptr)
	, m_MatrixBuffer(nullptr)
{
}

ColorShaderClass::~ColorShaderClass()
{
}

/*-------------------------------------------------------------------------------------------
* 초기화 함수는 셰이더의 초기화 함수를 호출합니다. 
* 이 튜토리얼에서는 HLSL 셰이더 파일의 이름을 전달하는데, 
* 이 파일의 이름은 color.vs와 color.ps입니다.
-------------------------------------------------------------------------------------------*/
bool ColorShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result = false;
	
	std::wstring vsFileName;
	std::wstring psFileName;

	vsFileName = L"../shader/Color_vs.hlsl";
	psFileName = L"../shader/Color_ps.hlsl";

	result = InitializeShader(device, hwnd, vsFileName, psFileName);

	return (result);
}

void ColorShaderClass::Shutdown()
{
	// 버텍스 및 픽셀 셰이더와 관련 오브젝트를 종료합니다.
	ShutdownShader();

	return;
}


/*-------------------------------------------------------------------------------------------
* Render는 먼저 SetShaderParameters 함수를 사용하여 
* 셰이더 내부의 파라미터를 설정합니다. 
* 파라미터가 설정되면 RenderShader를 호출하여 
* HLSL 셰이더를 사용하여 녹색 삼각형을 그립니다.
-------------------------------------------------------------------------------------------*/
bool ColorShaderClass::Render(
	ID3D11DeviceContext* deviceContext, 
	int indexCount, 
	DirectX::XMMATRIX world, 
	DirectX::XMMATRIX view, 
	DirectX::XMMATRIX projection)
{
	bool result = false;

	// 렌더링에 사용할 셰이더 파라미터를 설정합니다.
	result = SetShaderParameters(deviceContext, world, view, projection);
	if (result)
	{
		RenderShader(deviceContext, indexCount);
	}

	return (result);
}


/*-------------------------------------------------------------------------------------------
* 이제 이 튜토리얼에서 더 중요한 함수 중 하나인 
* InitializeShader라는 함수부터 시작하겠습니다. 
* 이 함수는 실제로 셰이더 파일을 로드하여 
* DirectX와 GPU에서 사용할 수 있도록 합니다. 
* 또한 레이아웃의 설정과 버텍스 버퍼 데이터가 
* GPU의 그래픽 파이프라인에서 어떻게 표시되는지도 확인할 수 있습니다. 
* 레이아웃은 modelclass.h 파일의 VertexType과 color.vs 파일에 
* 정의된 것과 일치해야 합니다.
-------------------------------------------------------------------------------------------*/
bool ColorShaderClass::InitializeShader(
	ID3D11Device* device, 
	HWND hwnd, 
	const std::wstring& vsFilename, 
	const std::wstring& psFilename)
{
	HRESULT result;

	// 에러 메시지를 받아오기 위한 메모리
	ID3D10Blob* pErrorMessage = nullptr;

	/*-------------------------------------------------------------------------------------------
	* 여기에서 셰이더 프로그램을 버퍼에 컴파일합니다. 
	* 셰이더 파일 이름, 셰이더 이름, 셰이더 버전(DirectX 11의 경우 5.0), 
	* 셰이더를 컴파일할 버퍼를 지정합니다. 
	* 셰이더 컴파일에 실패하면 errorMessage 문자열 안에 오류 메시지를 넣어 
	* 다른 함수에 보내 오류를 기록합니다. 그래도 컴파일이 실패하고 
	* errorMessage 문자열이 없으면 셰이더 파일을 찾을 수 없다는 뜻이며, 
	* 이 경우 대화 상자가 나타납니다.
	* https://learn.microsoft.com/ko-kr/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompilefromfile
	-------------------------------------------------------------------------------------------*/
	// 버텍스 셰이더 코드 컴파일
	ID3D10Blob* pVertexShaderBuffer = nullptr;
	result = D3DCompileFromFile(
		vsFilename.c_str(),				// 파일 이름
		NULL,							// 셰이더 매크로
		NULL,							// 셰이더에 #include 포함하는 경우 
		"ColorVertexShader",			// 셰이더 엔트리 포인트
		"vs_5_0",						// 셰이더 버전
		D3D10_SHADER_ENABLE_STRICTNESS,	// 셰이더 플래그1
		0,								// 셰이더 플래그2
		&pVertexShaderBuffer,			// 컴파일된 코드에 액세스 가능한 ID3DBlob 인터페이스
		&pErrorMessage					// 에러 메세지
	);
	if (FAILED(result))
	{
		if (pErrorMessage != nullptr)
		{ // 만약 컴파일에 실패했다면 에러 메시지에 무엇인가 기록되어 있습니다.
			OutputShaderErrorMessage(*pErrorMessage, hwnd, vsFilename);
		}
		else
		{ // 만약 에러 메시지가 nullptr일 경우 파일을 찾을 수 없는 문제입니다.
			MessageBox(hwnd, vsFilename.c_str(), L"vs 셰이더 파일을 찾을 수 없습니다.", MB_OK);
		}
		return (false);
	}
	
	// 픽셀 셰이더 코드 컴파일
	ID3D10Blob* pPixelShaderBuffer = nullptr;
	result = D3DCompileFromFile(
		psFilename.c_str(),
		NULL,
		NULL,
		"ColorPixelShader",
		"ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS,
		0,
		&pPixelShaderBuffer,
		&pErrorMessage
	);
	if (FAILED(result))
	{
		if (pErrorMessage != nullptr)
		{ // 만약 컴파일에 실패했다면 에러 메시지에 무엇인가 기록되어 있습니다.
			OutputShaderErrorMessage(*pErrorMessage, hwnd, psFilename);
		}
		else
		{ // 만약 에러 메시지가 nullptr일 경우 파일을 찾을 수 없는 문제입니다.
			MessageBox(hwnd, psFilename.c_str(), L"ps 셰이더 파일을 찾을 수 없습니다.", MB_OK);
		}
		return (false);
	}
	

	/*-------------------------------------------------------------------------------------------
	* 버텍스 셰이더와 픽셀 셰이더 코드가 버퍼로 성공적으로 컴파일되면 
	* 해당 버퍼를 사용하여 셰이더 오브젝트 자체를 생성합니다. 
	* 이 시점부터 이 포인터를 사용하여 버텍스 및 픽셀 셰이더와 인터페이스할 것입니다.
	-------------------------------------------------------------------------------------------*/
	// 버퍼로부터 버텍스 셰이더를 만듭니다.
	result = device->CreateVertexShader( 
		pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(),
		nullptr, 
		&m_VertexShader
	);
	if (FAILED(result))
	{
		MessageBox(hwnd, L"CreateVertexShader", L"vs 셰이더 생성에 실패하였습니다.", MB_OK);
		return (false);
	}

	// 버퍼로부터 픽셀 셰이더를 만듭니다.
	result = device->CreatePixelShader(
		pPixelShaderBuffer->GetBufferPointer(),
		pPixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_PixelShader
	);
	if (FAILED(result))
	{
		MessageBox(hwnd, L"CreatePixelShader", L"ps 셰이더 생성에 실패하였습니다.", MB_OK);
		return (false);
	}


	/*-------------------------------------------------------------------------------------------
	* 다음 단계는 셰이더가 처리할 버텍스 데이터의 레이아웃을 만드는 것입니다. 
	* 이 셰이더는 위치와 색상 벡터를 사용하므로 레이아웃에 두 가지 크기를 지정하여 
	* 둘 다 만들어야 합니다. 시맨틱 이름은 레이아웃에서 가장 먼저 작성해야 하는 것으로, 
	* 셰이더가 레이아웃의 이 요소의 용도를 결정할 수 있게 해줍니다. 두 개의 다른 요소가 
	* 있으므로 첫 번째 요소에는 POSITION을 사용하고 두 번째 요소에는 COLOR를 사용합니다. 
	* 레이아웃에서 다음으로 중요한 부분은 포맷입니다. 위치 벡터에는 
	* DXGI_FORMAT_R32G32B32_FLOAT를 사용하고 색상에는 DXGI_FORMAT_R32G32B32A32_FLOAT를 
	* 사용합니다. 마지막으로 주의해야 할 것은 버퍼에서 데이터의 간격이 어떻게 배치되는지를 
	* 나타내는 AlignedByteOffset입니다. 이 레이아웃의 경우 처음 12바이트는 위치, 
	* 다음 16바이트는 색상이며, AlignedByteOffset은 각 요소가 시작되는 위치를 표시합니다. 
	* AlignedByteOffset에 직접 값을 입력하는 대신 D3D11_APPEND_ALIGNED_ELEMENT를 
	* 사용하면 간격을 알아서 지정해 줍니다. 다른 설정은 이 튜토리얼에서는 
	* 필요하지 않으므로 지금은 기본값으로 설정했습니다.
	-------------------------------------------------------------------------------------------*/
	// 버텍스 인풋 레이아웃 구조체를 만듭니다.
	// 이 설정은 ModelClass와 셰이더의 VertexType 구조와 일치해야 합니다.
	D3D11_INPUT_ELEMENT_DESC layout_desc[2];
	layout_desc[0].SemanticName = "POSITION";
	layout_desc[0].SemanticIndex = 0;
	layout_desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	layout_desc[0].InputSlot = 0;
	layout_desc[0].AlignedByteOffset = 0;
	layout_desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout_desc[0].InstanceDataStepRate = 0;

	layout_desc[1].SemanticName = "COLOR";
	layout_desc[1].SemanticIndex = 0;
	layout_desc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	layout_desc[1].InputSlot = 0;
	layout_desc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	layout_desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	layout_desc[1].InstanceDataStepRate = 0;


	/*-------------------------------------------------------------------------------------------
	* 레이아웃 설명이 설정되면 레이아웃의 크기를 구한 다음 
	* D3D 장치를 사용하여 입력 레이아웃을 만들 수 있습니다.
	* 또한 레이아웃이 생성되면 더 이상 필요하지 않으므로 
	* 버텍스 및 픽셀 셰이더 버퍼를 해제합니다.
	-------------------------------------------------------------------------------------------*/
	// 레이아웃의 개수를 구합니다.
	unsigned int numElements = sizeof(layout_desc) / sizeof(layout_desc[0]);
	// 레이아웃을 생성합니다.
	result = device->CreateInputLayout(
		layout_desc, numElements,
		pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(),
		&m_Layout
	);
	if (FAILED(result))
	{
		MessageBox(hwnd, L"CreateInputLayout", L"레이아웃 생성에 실패하였습니다.", MB_OK);
		return (false);
	}

	// 버텍스 셰이더 버퍼와 픽셀 셰이더 버퍼는 더 이상 필요하지 않으므로 해제합니다.
	pVertexShaderBuffer->Release();
	pVertexShaderBuffer = nullptr;

	pPixelShaderBuffer->Release();
	pPixelShaderBuffer = nullptr;



	return (true);
}

void ColorShaderClass::ShutdownShader()
{
}

void ColorShaderClass::OutputShaderErrorMessage(const ID3D10Blob& message, HWND hwnd, const std::wstring& filename)
{
}

bool ColorShaderClass::SetShaderParameters(ID3D11DeviceContext*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX)
{
	return false;
}

void ColorShaderClass::RenderShader(ID3D11DeviceContext*, int)
{
}
