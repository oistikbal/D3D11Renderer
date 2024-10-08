#include "application.h"

d3d11renderer::application::application(int screenWidth, int screenHeight, HWND hwnd, std::shared_ptr<d3d11renderer::input> input)
{
	try 
	{
		m_d3d = std::make_shared<d3d11renderer::d3dclass>(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		m_camera = std::make_shared<camera>(input);
		m_camera->set_position(0.0f, 0.0f, -10.0f);
		m_camera->set_rotation(0.0f, DirectX::XMConvertToDegrees(179.0f), 0.0f);

		m_lightShader = std::make_shared<light_shader>(m_d3d->get_device(), hwnd);
		m_light = std::make_shared<light>();
		m_light->set_ambient_color(0.15f, 0.15f, 0.15f, 1.0f);
		m_light->set_diffuse_color(1.0f, 1.0f, 1.0f, 1.0f);
		m_light->set_direction(1.0f, 0.0f, 1.0f);
		m_light->set_specular_color(0.0f, 0.0f, 0.0f, 0.0f);
		m_light->set_specular_power(32.0f);


		wchar_t modelFilename[128];
		wcscpy_s(modelFilename, L"Models\\sphere.txt");;

		m_model = std::make_shared<model>(m_d3d->get_device(), m_d3d->get_device_context(), modelFilename, L"Images\\brick.png");
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
	static float rotation = 0.0f;
	bool result;


	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * 0.1f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}

	// Render the graphics scene.
	result = render(rotation);
	if (!result)
	{
		return false;
	}

	return true;
}

void d3d11renderer::application::resize(int width, int height)
{
	if (m_d3d == nullptr)
		return;

	m_d3d->resize(width, height);
}

bool d3d11renderer::application::render(float rotation)
{
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_d3d->begin_scene(0.3f,0.3f,0.3f,0.1f);

	m_camera->frame(1.0f / 144.0f);
	m_camera->render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_d3d->get_world_matrix(worldMatrix);
	m_camera->get_view_matrix(viewMatrix);
	m_d3d->get_projection_matrix(projectionMatrix);

	worldMatrix = DirectX::XMMatrixRotationY(rotation);


	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.

	m_model->render(m_d3d->get_device_context());

	// Render the model using the color shader.
	result = m_lightShader->render(m_d3d->get_device_context(), m_model->get_index_count(), worldMatrix, viewMatrix, projectionMatrix, m_model->get_texture(), m_light->get_direction(),
		m_light->get_diffuse_color(), m_light->get_ambient_color(), m_camera->get_position(), m_light->get_specular_color(), m_light->get_specular_power());
	if (!result)
	{
		return false;
	}

	m_d3d->end_scene();

	return true;
}
