#include "application.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

d3d11renderer::application::application(int screenWidth, int screenHeight, HWND hwnd, std::shared_ptr<d3d11renderer::input> input)
{
	try 
	{
		m_d3d = std::make_shared<d3d11renderer::d3dclass>(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
		m_camera = std::make_shared<camera>(input);
		m_camera->set_position(0.0f, 0.0f, 10.0f);
		m_camera->set_rotation(0.0f, DirectX::XMConvertToRadians(90.0f), 0.0f);

		m_lightShader = std::make_shared<light_shader>(m_d3d->get_device(), hwnd);
		m_light = std::make_shared<light>();
		m_light->set_ambient_color(0.15f, 0.15f, 0.15f, 1.0f);
		m_light->set_diffuse_color(1.0f, 1.0f, 1.0f, 1.0f);
		m_light->set_direction(1.0f, 0.0f, 1.0f);
		m_light->set_specular_color(0.0f, 0.0f, 0.0f, 0.0f);
		m_light->set_specular_power(32.0f);
		m_skybox = std::make_shared<skybox>(m_d3d->get_device(), m_d3d->get_device_context(), L"Skyboxes/kloppenheim_06_puresky_4k.hdr");


		m_model = std::make_shared<model>(m_d3d->get_device(), m_d3d->get_device_context(), "Models/DamagedHelmet/DamagedHelmet.gltf", "Models/DamagedHelmet");
		m_sphere = std::make_shared<model>(m_d3d->get_device(), m_d3d->get_device_context(), "Models/sphere.gltf", "Models/");
		ImGui_ImplDX11_Init(m_d3d->get_device(), m_d3d->get_device_context());
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

bool d3d11renderer::application::frame(float deltaTime)
{
	bool result;

	// Render the graphics scene.
	result = render(deltaTime);
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

bool d3d11renderer::application::render(float deltaTime)
{
	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// Clear the buffers to begin the scene.
	m_d3d->begin_scene(0.3f,0.3f,0.3f,0.1f);

	m_camera->frame(deltaTime);
	m_camera->render();

	// Get the world, view, and projection matrices from the camera and d3d objects.
	m_d3d->get_world_matrix(worldMatrix);
	m_camera->get_view_matrix(viewMatrix);
	m_d3d->get_projection_matrix(projectionMatrix);

	m_d3d->set_culling(false);
	m_d3d->set_depth(false);

	m_sphere->render(m_d3d->get_device_context());

	for (const auto& subMesh : m_sphere->get_sub_meshes()) // Assuming get_sub_meshes() returns a collection of sub-mesh data
	{
		m_skybox->render(m_d3d->get_device_context(), subMesh.indexCount);
	}

	m_d3d->set_culling(true);
	m_d3d->set_depth(true);
	static float rotation = 0.0f;
	// Update the rotation variable each frame.
	rotation -= 0.0174532925f * deltaTime * 10.0f;
	if (rotation < 0.0f)
	{
		rotation += 360.0f;
	}


	worldMatrix = DirectX::XMMatrixRotationY(rotation);
	worldMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2), worldMatrix);


	// Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_model->render(m_d3d->get_device_context());

	for (const auto& subMesh : m_model->get_sub_meshes()) // Assuming get_sub_meshes() returns a collection of sub-mesh data
	{
		// Retrieve the texture associated with the current sub-mesh
		ID3D11ShaderResourceView* texture = subMesh.texture ? subMesh.texture->get_texture() : nullptr; // Ensure texture retrieval is safe

		// Set shader parameters, including the texture
		result = m_lightShader->render(m_d3d->get_device_context(), subMesh.indexCount, worldMatrix, viewMatrix, projectionMatrix,
			texture, m_light->get_direction(), m_light->get_diffuse_color(), m_light->get_ambient_color(),
			m_camera->get_position(), m_light->get_specular_color(), m_light->get_specular_power());

		// Optionally, check the result for errors
		if (!result) {
			// Handle any rendering errors
		}
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	{
		auto pos = m_camera->get_position(); // Assuming you have this function
		auto rot = m_camera->get_rotation(); // Assuming you have this function
		ImGui::Begin("Camera");

		ImGui::Text("Fps:");
		ImGui::SameLine();
		ImGui::Text("%.2f", 1.0f / deltaTime);
		ImGui::Text("Position:");
		ImGui::SameLine();
		ImGui::Text("X: %.2f", pos.x);
		ImGui::SameLine();
		ImGui::Text("Y: %.2f", pos.y);
		ImGui::SameLine();
		ImGui::Text("Z: %.2f", pos.z);


		// Display camera rotation horizontally
		ImGui::Text("Rotation:");
		ImGui::SameLine();
		ImGui::Text("X: %.2f", DirectX::XMConvertToDegrees(rot.x));
		ImGui::SameLine();
		ImGui::Text("Y: %.2f", DirectX::XMConvertToDegrees(rot.y));
		ImGui::SameLine();
		ImGui::Text("Z: %.2f", DirectX::XMConvertToDegrees(rot.z));

		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_d3d->end_scene();

	return true;
}
