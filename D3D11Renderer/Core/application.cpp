#include "application.h"

d3d11renderer::application::application(int screenWidth, int screenHeight, HWND hwnd)
{
	try 
	{
		m_d3d = std::make_shared<d3d11renderer::d3dclass>(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	}
	catch (std::exception e) 
	{
		MessageBoxA(hwnd, e.what(), "Error", MB_OK | MB_ICONERROR);
	}
}

d3d11renderer::application::~application()
{
}

void d3d11renderer::application::shutdown()
{
}

bool d3d11renderer::application::frame()
{
	bool result;


	// Render the graphics scene.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool d3d11renderer::application::render()
{
	// Clear the buffers to begin the scene.
	m_d3d->begin_scene(0.5f, 0.5f, 0.5f, 1.0f);


	// Present the rendered scene to the screen.
	m_d3d->end_scene();

	return true;
}
