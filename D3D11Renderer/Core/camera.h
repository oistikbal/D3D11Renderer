#pragma once

#include <directxmath.h>
#include <memory>
#include "input.h"

namespace d3d11renderer
{
	class camera
	{
	public:
		camera(std::shared_ptr<d3d11renderer::input> input);
		~camera();

		void frame(float);
		void set_position(float, float, float);
		void set_rotation(float, float, float);

		DirectX::XMFLOAT3 get_position();
		DirectX::XMFLOAT3 get_rotation();
		DirectX::XMFLOAT3 get_forward();
		DirectX::XMFLOAT3 get_right();
		DirectX::XMFLOAT3 get_up();

		void render();
		void get_view_matrix(DirectX::XMMATRIX&);

	private:
		void strafe_right(float);
		void strafe_left(float);
		void move_forward(float);
		void move_backward(float);
		void smooth_rotate(float deltaX, float deltaY, float smoothFactor);

	private:
		DirectX::XMFLOAT3 m_position;
		DirectX::XMFLOAT3 m_rotation;
		DirectX::XMMATRIX m_viewMatrix;
		std::shared_ptr<input> m_input;
		float m_moveSpeed = 4.0f;
		float m_rotationSpeed = 1.0f;
		float m_lastMouseX;
		float m_lastMouseY;
		float m_mouseSensitivity;
		const float maxPitch = DirectX::XMConvertToRadians(89.0f);
		const float minPitch = DirectX::XMConvertToRadians(-89.0f);
	};
}
