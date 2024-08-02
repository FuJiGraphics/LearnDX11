#pragma once
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#include "Core.h"

// forward
class InputClass;
class ApplicationClass;

class SystemClass
{
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();

	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();


private:
	LPCWSTR m_ApplicationName;
	HINSTANCE m_hInstance;
	HWND m_Hwnd;

	fz::Shared<InputClass> m_Input;
	fz::Shared<ApplicationClass> m_Application;
};

// Function Prototypes
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Globals
static SystemClass* s_ApplicationHandle = nullptr;

#endif