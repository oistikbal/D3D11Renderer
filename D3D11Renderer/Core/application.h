#pragma once

#include <Windows.h>
#include <memory>

#include "d3dclass.h"
#include "camera.h"
#include "model.h"
#include "texture_shader.h"
#include "light_shader.h"
#include "light.h"
#include "skybox.h"
#include "reinhard_shader.h"

constexpr bool FULL_SCREEN = false;
constexpr bool VSYNC_ENABLED = true;
constexpr float SCREEN_DEPTH = 1000.0f;
constexpr float SCREEN_NEAR = 0.3f;

namespace d3d11renderer 
{
	class application
	{
		enum class scene_state {
			Sponza,
			DamagedHelmet,
			ScifiHelmet
		};
	public:
		application(int, int, HWND, std::shared_ptr<d3d11renderer::input>);
		~application();

		void shutdown();
		bool frame(float deltaTime);
		void resize(int width, int height);

	private:
		bool render(float);
		void update_fps_plot(float deltaTime);
	private:
		std::shared_ptr<d3d11renderer::d3dclass> m_d3d;
		std::shared_ptr<camera> m_camera;
		std::shared_ptr<model> m_sponza;
		std::shared_ptr<model> m_damagedHelmet;
		std::shared_ptr<model> m_scifiHelmet;
		std::shared_ptr<model> m_sphere;
		std::shared_ptr<light> m_light;
		std::shared_ptr<light_shader> m_lightShader;
		std::shared_ptr<skybox> m_skybox;
		std::shared_ptr<reinhard_shader> m_reinhardShader;
		std::vector<float> m_fpsHistory;
		scene_state m_current_scene;
		bool m_scene_values[3];
	};
}