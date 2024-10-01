#pragma once

#include <directxmath.h>

class camera
{
public:
	camera();
	~camera();


	void set_position(float, float, float);
	void set_rotation(float, float, float);

	DirectX::XMFLOAT3 get_position();
	DirectX::XMFLOAT3 get_rotation();

	void render();
	void get_view_matrix(DirectX::XMMATRIX&);

private:
	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;
	DirectX::XMMATRIX m_viewMatrix;
};
