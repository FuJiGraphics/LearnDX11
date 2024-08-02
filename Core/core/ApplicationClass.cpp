#include "applicationclass.h"
#include "D3DClass.h"


ApplicationClass::ApplicationClass()
	: m_Direct3D(nullptr)
{
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	
	// Create and initialize the Direct3D object.
	m_Direct3D = fz::CreateShared<D3DClass>();

	result = m_Direct3D->Initialize(
		screenWidth, screenHeight, 
		VSYNC_ENABLED, 
		hwnd,
		FULL_SCREEN,
		SCREEN_DEPTH,
		SCREEN_NEAR
	);

	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return (false);
	}
	
	return (true);
}


void ApplicationClass::Shutdown()
{
	// Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D.reset();
		m_Direct3D = nullptr;
	}

	return;
}


bool ApplicationClass::Frame()
{
	bool result;

	// Render the graphics scene.
	result = this->Render();
	if (!result)
	{
		return (false);
	}

	return (true);
}


bool ApplicationClass::Render()
{
	// Clear the buffers to begin the scene
	m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();
	return (true);
}
