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
		m_camera->set_rotation(0.0f, DirectX::XM_PIDIV2, 0.0f);

		m_lightShader = std::make_shared<light_shader>(m_d3d->get_device(), hwnd);
		m_light = std::make_shared<light>();
		m_light->set_ambient_color(0.15f, 0.15f, 0.15f, 1.0f);
		m_light->set_diffuse_color(1.0f, 1.0f, 1.0f, 1.0f);
		m_light->set_direction(0.0f, 0.0f, 1.0f);
		m_light->set_specular_color(1.0f, 1.0f, 1.0f, 1.0f);
		m_light->set_specular_power(256.0f);
		m_skybox = std::make_shared<skybox>(m_d3d->get_device(), m_d3d->get_device_context(), L"Skyboxes/kloppenheim_06_puresky_4k.hdr");
		m_reinhardShader = std::make_shared<reinhard_shader>(m_d3d->get_device(), hwnd);
		m_scene_values[0] = false;
		m_scene_values[1] = false;
		m_scene_values[2] = false;

		m_sponza = std::make_shared<model>(m_d3d->get_device(), m_d3d->get_device_context(), "Models/Sponza/Sponza.gltf", "Models/Sponza");
		m_damagedHelmet = std::make_shared<model>(m_d3d->get_device(), m_d3d->get_device_context(), "Models/DamagedHelmet/DamagedHelmet.gltf", "Models/DamagedHelmet");
		m_scifiHelmet = std::make_shared<model>(m_d3d->get_device(), m_d3d->get_device_context(), "Models/SciFiHelmet/SciFiHelmet.gltf", "Models/SciFiHelmet");
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
	update_fps_plot(deltaTime);
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
		m_skybox->render(m_d3d->get_device_context(), subMesh.indexCount, viewMatrix, projectionMatrix);
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


	//worldMatrix = DirectX::XMMatrixRotationY(rotation);
	//worldMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(DirectX::XM_PIDIV2), worldMatrix);
	//worldMatrix = DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.1f,0.1f,0.1f), worldMatrix);


	switch (m_current_scene) {
	case scene_state::DamagedHelmet:
		m_damagedHelmet->render(m_d3d->get_device_context());

		for (const auto& subMesh : m_damagedHelmet->get_sub_meshes()) // Assuming get_sub_meshes() returns a collection of sub-mesh data
		{
			// Retrieve the texture associated with the current sub-mesh
			ID3D11ShaderResourceView* diffuse = subMesh.diffuseTexture ? subMesh.diffuseTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* normal = subMesh.normalTexture ? subMesh.normalTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* specular = subMesh.specularTexture ? subMesh.specularTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* ao = subMesh.aoTexture ? subMesh.aoTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* emissive = subMesh.emissiveTexture ? subMesh.emissiveTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* metal = subMesh.metalRoughnessTexture ? subMesh.metalRoughnessTexture->get_texture() : nullptr;

			// Set shader parameters, including the texture
			result = m_lightShader->render(m_d3d->get_device_context(), subMesh.indexCount, subMesh.startIndex, worldMatrix, viewMatrix, projectionMatrix,
				diffuse, normal, specular, ao, emissive, metal,
				m_light->get_direction(), m_light->get_diffuse_color(), m_light->get_ambient_color(),
				m_camera->get_position(), m_light->get_specular_color(), m_light->get_specular_power());

			// Optionally, check the result for errors
			if (!result) {
				// Handle any rendering errors
			}
		}
		break;
	case scene_state::ScifiHelmet:
		m_scifiHelmet->render(m_d3d->get_device_context());

		for (const auto& subMesh : m_scifiHelmet->get_sub_meshes()) // Assuming get_sub_meshes() returns a collection of sub-mesh data
		{
			// Retrieve the texture associated with the current sub-mesh
			ID3D11ShaderResourceView* diffuse = subMesh.diffuseTexture ? subMesh.diffuseTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* normal = subMesh.normalTexture ? subMesh.normalTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* specular = subMesh.specularTexture ? subMesh.specularTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* ao = subMesh.aoTexture ? subMesh.aoTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* emissive = subMesh.emissiveTexture ? subMesh.emissiveTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* metal = subMesh.metalRoughnessTexture ? subMesh.metalRoughnessTexture->get_texture() : nullptr;

			// Set shader parameters, including the texture
			result = m_lightShader->render(m_d3d->get_device_context(), subMesh.indexCount, subMesh.startIndex, worldMatrix, viewMatrix, projectionMatrix,
				diffuse, normal, specular, ao, emissive, metal,
				m_light->get_direction(), m_light->get_diffuse_color(), m_light->get_ambient_color(),
				m_camera->get_position(), m_light->get_specular_color(), m_light->get_specular_power());

			// Optionally, check the result for errors
			if (!result) {
				// Handle any rendering errors
			}
		}
		break;
	case scene_state::Sponza:
		m_sponza->render(m_d3d->get_device_context());

		for (const auto& subMesh : m_sponza->get_sub_meshes()) // Assuming get_sub_meshes() returns a collection of sub-mesh data
		{
			// Retrieve the texture associated with the current sub-mesh
			ID3D11ShaderResourceView* diffuse = subMesh.diffuseTexture ? subMesh.diffuseTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* normal = subMesh.normalTexture ? subMesh.normalTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* specular = subMesh.specularTexture ? subMesh.specularTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* ao = subMesh.aoTexture ? subMesh.aoTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* emissive = subMesh.emissiveTexture ? subMesh.emissiveTexture->get_texture() : nullptr;
			ID3D11ShaderResourceView* metal = subMesh.metalRoughnessTexture ? subMesh.metalRoughnessTexture->get_texture() : nullptr;

			// Set shader parameters, including the texture
			result = m_lightShader->render(m_d3d->get_device_context(), subMesh.indexCount, subMesh.startIndex, worldMatrix, viewMatrix, projectionMatrix,
				diffuse, normal, specular, ao, emissive, metal,
				m_light->get_direction(), m_light->get_diffuse_color(), m_light->get_ambient_color(),
				m_camera->get_position(), m_light->get_specular_color(), m_light->get_specular_power());

			// Optionally, check the result for errors
			if (!result) {
				// Handle any rendering errors
			}
		}
		break;
	}


	m_d3d->end_scene();

	static float exposure = 0.6f;         // Initial exposure
	static float averageLuminance = 1.0f; // Average luminance
	static float maxLuminance = 1.0f;     // Maximum luminance for clamping
	static float burn = 1.0f;             // Burn threshold for tone mapping

	m_reinhardShader->render(m_d3d->get_device_context(), m_d3d->get_tonemap_srv(), exposure, averageLuminance, maxLuminance, burn);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	{
		auto pos = m_camera->get_position(); // Assuming you have this function
		auto rot = m_camera->get_rotation(); // Assuming you have this function
		ImGui::Begin("Settings");
			if (ImGui::CollapsingHeader("Stats"))
			{
				ImGui::Text("Fps:");
				ImGui::SameLine();
				ImGui::Text("%.2f", 1.0f / deltaTime);
				ImGui::PlotLines("FPS", m_fpsHistory.data(), static_cast<int>(m_fpsHistory.size()), 0, nullptr, 0.0f, 100.0f, ImVec2(0, 80));


				ImGui::Text("Video Card: %s", m_d3d->get_gpu_name().c_str());

				ImGui::Text("Video Card Memory: %d MB", m_d3d->get_gpu_memory());
			}

			if (ImGui::CollapsingHeader("Camera"))
			{
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
			}

			if (ImGui::CollapsingHeader("Tone Map"))
			{
				ImGui::SliderFloat("Exposure", &exposure, 0.1f, 10.0f, "%.2f");

				// Slider for Average Luminance
				ImGui::SliderFloat("Average Luminance", &averageLuminance, 0.0f, 2.0f, "%.2f");

				// Slider for Max Luminance
				ImGui::SliderFloat("Max Luminance", &maxLuminance, 0.1f, 10.0f, "%.2f");

				// Slider for Burn
				ImGui::SliderFloat("Burn", &burn, 0.1f, 5.0f, "%.2f");
			}

			if (ImGui::CollapsingHeader("Scene"))
			{
				for (int i = 0; i < 3; ++i) {
					if (ImGui::Checkbox(("Scene " + std::to_string(i + 1)).c_str(), &m_scene_values[i])) {
						// If this checkbox is checked, set it as the current scene
						if (m_scene_values[i]) {
							m_current_scene = static_cast<scene_state>(i);
							// Uncheck all other checkboxes
							for (int j = 0; j < 3; ++j) {
								if (j != i) {
									m_scene_values[j] = false;
								}
							}
						}
					}
					ImGui::SameLine();
				}
			}

		ImGui::End();
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_d3d->present();

	return true;
}

void d3d11renderer::application::update_fps_plot(float deltaTime)
{
	float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;

	// Store FPS in history
	m_fpsHistory.push_back(fps);
	if (m_fpsHistory.size() > 100) {
		m_fpsHistory.erase(m_fpsHistory.begin()); // Maintain the last 100 FPS values
	}
}
