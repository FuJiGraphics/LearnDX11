#include "SystemClass.h"
#include "InputClass.h"
#include "ApplicationClass.h"

SystemClass::SystemClass()
	: m_Input(nullptr)
	, m_Application(nullptr)
{
}

SystemClass::SystemClass(const SystemClass& target)
{
}

SystemClass::~SystemClass()
{
}

bool SystemClass::Initialize()
{
	int screenWidth, screenHeight;
	bool result;

	// Initialize the width and height of the screen to 
	// zero before sending the variables into the function.
	screenWidth = 0;
	screenHeight = 0;

	// Initialize the windows api
	this->InitializeWindows(screenWidth, screenHeight);

	// Create and initialize the input object. 
	// This object will be used to handle reading the keyboard from the user.
	m_Input = fz::CreateShared<InputClass>();
	m_Input->Initialize();

	// Create and initialize the application class object.
	// This object will handle rendering all the graphics
	// for this application.
	m_Application = fz::CreateShared<ApplicationClass>();

	result = m_Application->Initialize(screenWidth, screenHeight, m_Hwnd);
	if (!result)
	{
		return (false);
	}

	return (true);
}

void SystemClass::Shutdown()
{
	// Release the application class object.
	if (m_Application)
	{
		m_Application->Shutdown();
		m_Application.reset();
	}

	// Release the input class object.
	if (m_Input)
	{
		m_Input->Shutdown();
		m_Input.reset();
	}
	
	// Shutdown the window.
	this->ShutdownWindows();
}

void SystemClass::Run()
{
	MSG msg;
	bool isRun;

	// Initialize the message structure.
	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	isRun = true;
	while (isRun)
	{
		// Handle the windows messages.
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// if windows signals to end the application then exit out.
		if (msg.message == WM_QUIT)
		{
			isRun = false;
		}
		else
		{
			// Otherwise do the frame processing
			isRun = Frame();
		}
	}
}

bool SystemClass::Frame()
{
	bool result;

	// Check if the user pressed escape and wants to exit the application.
	if (m_Input->IsKeyDown(VK_ESCAPE))
	{
		return (false);
	}

	// Do the frame processing for the application class object.
	result = m_Application->Frame();
	if (!result)
	{
		return (false);
	}
	return (true);
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
		// Check if a key has been pressed on the keyboard
		case WM_KEYDOWN:
		{
			// If a key is pressed send if to the input object so if can record that state.
			m_Input->KeyDown((unsigned int)wparam);
			return (0);
		}
		// Check if a key has been released on the keyboard
		case WM_KEYUP:
		{
			// If a key is released then send it to the input object so it can unset the state for that key.
			m_Input->KeyDown((unsigned int)wparam);
			return (0);
		}

		// Any other message send to the default message handler as our application won't make use of them.
		default:
		{
			return (DefWindowProc(hwnd, umsg, wparam, lparam));
		}
	}
}

void SystemClass::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;


	// Get an external pointer to this object.	
	s_ApplicationHandle = this;

	// Get the instance of this application.
	m_hInstance = GetModuleHandle(NULL);

	// Give the application a name.
	m_ApplicationName = L"Engine";

	// Setup the windows class with default settings.
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_ApplicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		posX = posY = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screenWidth = 800;
		screenHeight = 600;

		// Place the window in the middle of the screen.
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	m_Hwnd = CreateWindowEx(WS_EX_APPWINDOW, m_ApplicationName, m_ApplicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, m_hInstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(m_Hwnd, SW_SHOW);
	SetForegroundWindow(m_Hwnd);
	SetFocus(m_Hwnd);

	// Hide the mouse cursor.
	ShowCursor(false);

	return;
}

void SystemClass::ShutdownWindows()
{
	// Show the mouse cursor.
	ShowCursor(true);

	// Fix the display settings if leaving full screen mode.
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	// Remove the window.
	DestroyWindow(m_Hwnd);
	m_Hwnd = NULL;

	// Remove the application instance.
	UnregisterClass(m_ApplicationName, m_hInstance);
	m_hInstance = NULL;

	// Release the pointer to this class.
	s_ApplicationHandle = nullptr;

	return;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
		// Check if the window is being destroyed.
		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		// Check if the window is being closed.
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		// All other messages pass to the message handler in the system class.
		default:
		{
			return s_ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
		}
	}
}
