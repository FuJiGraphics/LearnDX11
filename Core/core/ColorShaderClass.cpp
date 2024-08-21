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
* �ʱ�ȭ �Լ��� ���̴��� �ʱ�ȭ �Լ��� ȣ���մϴ�. 
* �� Ʃ�丮�󿡼��� HLSL ���̴� ������ �̸��� �����ϴµ�, 
* �� ������ �̸��� color.vs�� color.ps�Դϴ�.
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
	// ���ؽ� �� �ȼ� ���̴��� ���� ������Ʈ�� �����մϴ�.
	ShutdownShader();

	return;
}


/*-------------------------------------------------------------------------------------------
* Render�� ���� SetShaderParameters �Լ��� ����Ͽ� 
* ���̴� ������ �Ķ���͸� �����մϴ�. 
* �Ķ���Ͱ� �����Ǹ� RenderShader�� ȣ���Ͽ� 
* HLSL ���̴��� ����Ͽ� ��� �ﰢ���� �׸��ϴ�.
-------------------------------------------------------------------------------------------*/
bool ColorShaderClass::Render(
	ID3D11DeviceContext* deviceContext, 
	int indexCount, 
	DirectX::XMMATRIX world, 
	DirectX::XMMATRIX view, 
	DirectX::XMMATRIX projection)
{
	bool result = false;

	// �������� ����� ���̴� �Ķ���͸� �����մϴ�.
	result = SetShaderParameters(deviceContext, world, view, projection);
	if (result)
	{
		RenderShader(deviceContext, indexCount);
	}

	return (result);
}


/*-------------------------------------------------------------------------------------------
* ���� �� Ʃ�丮�󿡼� �� �߿��� �Լ� �� �ϳ��� 
* InitializeShader��� �Լ����� �����ϰڽ��ϴ�. 
* �� �Լ��� ������ ���̴� ������ �ε��Ͽ� 
* DirectX�� GPU���� ����� �� �ֵ��� �մϴ�. 
* ���� ���̾ƿ��� ������ ���ؽ� ���� �����Ͱ� 
* GPU�� �׷��� ���������ο��� ��� ǥ�õǴ����� Ȯ���� �� �ֽ��ϴ�. 
* ���̾ƿ��� modelclass.h ������ VertexType�� color.vs ���Ͽ� 
* ���ǵ� �Ͱ� ��ġ�ؾ� �մϴ�.
-------------------------------------------------------------------------------------------*/
bool ColorShaderClass::InitializeShader(
	ID3D11Device* device, 
	HWND hwnd, 
	const std::wstring& vsFilename, 
	const std::wstring& psFilename)
{
	HRESULT result;

	// ���� �޽����� �޾ƿ��� ���� �޸�
	ID3D10Blob* pErrorMessage = nullptr;

	/*-------------------------------------------------------------------------------------------
	* ���⿡�� ���̴� ���α׷��� ���ۿ� �������մϴ�. 
	* ���̴� ���� �̸�, ���̴� �̸�, ���̴� ����(DirectX 11�� ��� 5.0), 
	* ���̴��� �������� ���۸� �����մϴ�. 
	* ���̴� �����Ͽ� �����ϸ� errorMessage ���ڿ� �ȿ� ���� �޽����� �־� 
	* �ٸ� �Լ��� ���� ������ ����մϴ�. �׷��� �������� �����ϰ� 
	* errorMessage ���ڿ��� ������ ���̴� ������ ã�� �� ���ٴ� ���̸�, 
	* �� ��� ��ȭ ���ڰ� ��Ÿ���ϴ�.
	* https://learn.microsoft.com/ko-kr/windows/win32/api/d3dcompiler/nf-d3dcompiler-d3dcompilefromfile
	-------------------------------------------------------------------------------------------*/
	// ���ؽ� ���̴� �ڵ� ������
	ID3D10Blob* pVertexShaderBuffer = nullptr;
	result = D3DCompileFromFile(
		vsFilename.c_str(),				// ���� �̸�
		NULL,							// ���̴� ��ũ��
		NULL,							// ���̴��� #include �����ϴ� ��� 
		"ColorVertexShader",			// ���̴� ��Ʈ�� ����Ʈ
		"vs_5_0",						// ���̴� ����
		D3D10_SHADER_ENABLE_STRICTNESS,	// ���̴� �÷���1
		0,								// ���̴� �÷���2
		&pVertexShaderBuffer,			// �����ϵ� �ڵ忡 �׼��� ������ ID3DBlob �������̽�
		&pErrorMessage					// ���� �޼���
	);
	if (FAILED(result))
	{
		if (pErrorMessage != nullptr)
		{ // ���� �����Ͽ� �����ߴٸ� ���� �޽����� �����ΰ� ��ϵǾ� �ֽ��ϴ�.
			OutputShaderErrorMessage(*pErrorMessage, hwnd, vsFilename);
		}
		else
		{ // ���� ���� �޽����� nullptr�� ��� ������ ã�� �� ���� �����Դϴ�.
			MessageBox(hwnd, vsFilename.c_str(), L"vs ���̴� ������ ã�� �� �����ϴ�.", MB_OK);
		}
		return (false);
	}
	
	// �ȼ� ���̴� �ڵ� ������
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
		{ // ���� �����Ͽ� �����ߴٸ� ���� �޽����� �����ΰ� ��ϵǾ� �ֽ��ϴ�.
			OutputShaderErrorMessage(*pErrorMessage, hwnd, psFilename);
		}
		else
		{ // ���� ���� �޽����� nullptr�� ��� ������ ã�� �� ���� �����Դϴ�.
			MessageBox(hwnd, psFilename.c_str(), L"ps ���̴� ������ ã�� �� �����ϴ�.", MB_OK);
		}
		return (false);
	}
	

	/*-------------------------------------------------------------------------------------------
	* ���ؽ� ���̴��� �ȼ� ���̴� �ڵ尡 ���۷� ���������� �����ϵǸ� 
	* �ش� ���۸� ����Ͽ� ���̴� ������Ʈ ��ü�� �����մϴ�. 
	* �� �������� �� �����͸� ����Ͽ� ���ؽ� �� �ȼ� ���̴��� �������̽��� ���Դϴ�.
	-------------------------------------------------------------------------------------------*/
	// ���۷κ��� ���ؽ� ���̴��� ����ϴ�.
	result = device->CreateVertexShader( 
		pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(),
		nullptr, 
		&m_VertexShader
	);
	if (FAILED(result))
	{
		MessageBox(hwnd, L"CreateVertexShader", L"vs ���̴� ������ �����Ͽ����ϴ�.", MB_OK);
		return (false);
	}

	// ���۷κ��� �ȼ� ���̴��� ����ϴ�.
	result = device->CreatePixelShader(
		pPixelShaderBuffer->GetBufferPointer(),
		pPixelShaderBuffer->GetBufferSize(),
		nullptr,
		&m_PixelShader
	);
	if (FAILED(result))
	{
		MessageBox(hwnd, L"CreatePixelShader", L"ps ���̴� ������ �����Ͽ����ϴ�.", MB_OK);
		return (false);
	}


	/*-------------------------------------------------------------------------------------------
	* ���� �ܰ�� ���̴��� ó���� ���ؽ� �������� ���̾ƿ��� ����� ���Դϴ�. 
	* �� ���̴��� ��ġ�� ���� ���͸� ����ϹǷ� ���̾ƿ��� �� ���� ũ�⸦ �����Ͽ� 
	* �� �� ������ �մϴ�. �ø�ƽ �̸��� ���̾ƿ����� ���� ���� �ۼ��ؾ� �ϴ� ������, 
	* ���̴��� ���̾ƿ��� �� ����� �뵵�� ������ �� �ְ� ���ݴϴ�. �� ���� �ٸ� ��Ұ� 
	* �����Ƿ� ù ��° ��ҿ��� POSITION�� ����ϰ� �� ��° ��ҿ��� COLOR�� ����մϴ�. 
	* ���̾ƿ����� �������� �߿��� �κ��� �����Դϴ�. ��ġ ���Ϳ��� 
	* DXGI_FORMAT_R32G32B32_FLOAT�� ����ϰ� ���󿡴� DXGI_FORMAT_R32G32B32A32_FLOAT�� 
	* ����մϴ�. ���������� �����ؾ� �� ���� ���ۿ��� �������� ������ ��� ��ġ�Ǵ����� 
	* ��Ÿ���� AlignedByteOffset�Դϴ�. �� ���̾ƿ��� ��� ó�� 12����Ʈ�� ��ġ, 
	* ���� 16����Ʈ�� �����̸�, AlignedByteOffset�� �� ��Ұ� ���۵Ǵ� ��ġ�� ǥ���մϴ�. 
	* AlignedByteOffset�� ���� ���� �Է��ϴ� ��� D3D11_APPEND_ALIGNED_ELEMENT�� 
	* ����ϸ� ������ �˾Ƽ� ������ �ݴϴ�. �ٸ� ������ �� Ʃ�丮�󿡼��� 
	* �ʿ����� �����Ƿ� ������ �⺻������ �����߽��ϴ�.
	-------------------------------------------------------------------------------------------*/
	// ���ؽ� ��ǲ ���̾ƿ� ����ü�� ����ϴ�.
	// �� ������ ModelClass�� ���̴��� VertexType ������ ��ġ�ؾ� �մϴ�.
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
	* ���̾ƿ� ������ �����Ǹ� ���̾ƿ��� ũ�⸦ ���� ���� 
	* D3D ��ġ�� ����Ͽ� �Է� ���̾ƿ��� ���� �� �ֽ��ϴ�.
	* ���� ���̾ƿ��� �����Ǹ� �� �̻� �ʿ����� �����Ƿ� 
	* ���ؽ� �� �ȼ� ���̴� ���۸� �����մϴ�.
	-------------------------------------------------------------------------------------------*/
	// ���̾ƿ��� ������ ���մϴ�.
	unsigned int numElements = sizeof(layout_desc) / sizeof(layout_desc[0]);
	// ���̾ƿ��� �����մϴ�.
	result = device->CreateInputLayout(
		layout_desc, numElements,
		pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(),
		&m_Layout
	);
	if (FAILED(result))
	{
		MessageBox(hwnd, L"CreateInputLayout", L"���̾ƿ� ������ �����Ͽ����ϴ�.", MB_OK);
		return (false);
	}

	// ���ؽ� ���̴� ���ۿ� �ȼ� ���̴� ���۴� �� �̻� �ʿ����� �����Ƿ� �����մϴ�.
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
