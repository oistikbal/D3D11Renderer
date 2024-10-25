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
	public:
		application(int, int, HWND, std::shared_ptr<d3d11renderer::input>);
		~application();

		void shutdown();
		bool frame(float deltaTime);
		void resize(int width, int height);

	private:
		bool render(float);
	private:
		std::shared_ptr<d3d11renderer::d3dclass> m_d3d;
		std::shared_ptr<camera> m_camera;
		std::shared_ptr<model> m_model;
		std::shared_ptr<model> m_sphere;
		std::shared_ptr<light> m_light;
		std::shared_ptr<light_shader> m_lightShader;
		std::shared_ptr<skybox> m_skybox;
		std::shared_ptr<reinhard_shader> m_reinhardShader;
	};
}