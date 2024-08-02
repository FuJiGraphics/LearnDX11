#pragma once
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

// 그래픽을 그리기 위한 모든 API
#pragma comment(lib, "d3d11.lib")
// 모니터의 재생률, 사용중인 비디오 카드에 대한 정보
#pragma comment(lib, "dxgi.lib")
// 셰이더 컴파일 기능
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <DirectXMath.h>

class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDeivce();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(DirectX::XMMATRIX&);
	void GetWorldMatrix(DirectX::XMMATRIX&);
	void GetOrthoMatrix(DirectX::XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	void SetBackBufferRenderTarger();
	void ResetViewport();

private:
	bool						m_VSyncEnabled;
	int							m_VideoCardMemory;
	char						m_VideoCardDescription[128];
	IDXGISwapChain*				m_SwapChain;
	ID3D11Device*				m_Device;
	ID3D11DeviceContext*		m_DeviceContext;
	ID3D11RenderTargetView*		m_RenderTargetView;
	ID3D11Texture2D*			m_DepthStencilBuffer;
	ID3D11DepthStencilState*	m_DepthStencilState;
	ID3D11DepthStencilView*		m_DepthStencilView;
	ID3D11RasterizerState*		m_RasterState;
	DirectX::XMMATRIX			m_ProjectionMatrix;
	DirectX::XMMATRIX			m_WorldMatrix;
	DirectX::XMMATRIX			m_OrthoMatrix;
	D3D11_VIEWPORT				m_Viewport;
};

#endif