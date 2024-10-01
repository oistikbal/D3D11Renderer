#include "application.h"

d3d11renderer::application::application(int screenWidth, int screenHeight, HWND hwnd)
{
	try 
	{
		m_d3d = std::make_shared<d3d11renderer::d3dclass>(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		m_camera = std::make_shared<camera>();
		m_camera->set_position(0.0f, 0.0f, -5.0f);

		m_model = std::make_shared<model>(m_d3d->get_device());

		m_colorShader = std::make_shared<color_shader>(m_d3d->get_device(), hwnd);
	}
	catch (std::exception e) 
	{
		MessageBoxA(hwnd, e.what(), "Error", MB_OK | MB_ICONERROR);
		throw std::runtime_error(e.what());
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
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_d3d->begin_scene(0.3f,0.3f,0.3f,0.1f);

	// Generate the view matrix based on the camera's position.
	m_camera->render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_d3d->get_world_matrix(worldMatrix);
	m_camera->get_view_matrix(viewMatrix);
	m_d3d->get_projection_matrix(projectionMatrix);

	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.

	m_model->render(m_d3d->get_device_context());

	// Render the model using the color shader.
	result = m_colorShader->render(m_d3d->get_device_context(), m_model->get_index_count(), worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	m_d3d->end_scene();

	return true;
}
