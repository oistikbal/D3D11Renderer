#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <memory>

#include "Core/input.h"
#include "Core/application.h"

namespace d3d11renderer {
	class system
	{
	public:
		system();
		~system();

		void run();

		LRESULT CALLBACK message_handler(HWND, UINT, WPARAM, LPARAM);
	private:
		bool frame();
		void initialize_windows(int&, int&);
		void shutdown_windows();

	private:
		LPCWSTR m_applicationName;
		HINSTANCE m_hinstance;
		HWND m_hwnd;

		std::shared_ptr<input> m_input;
		std::shared_ptr<application> m_application;
	};
}

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
