#pragma once
#include <Windows.h>

constexpr bool FULL_SCREEN = false;
constexpr bool VSYNC_ENABLED = true;
constexpr float SCREEN_DEPTH = 1000.0f;
constexpr float SCREEN_NEAR = 0.3f;

namespace d3d11renderer 
{
	class application
	{
	public:
		application(int, int, HWND);
		~application();

		void shutdown();
		bool frame();

	private:
		bool render();
	};
}