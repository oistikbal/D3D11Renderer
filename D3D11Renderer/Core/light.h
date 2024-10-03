#pragma once

#include <directxmath.h>

class light
{
public:
	light();
	~light();

	void set_diffuse_color(float, float, float, float);
	void set_direction(float, float, float);

	DirectX::XMFLOAT4 get_diffuse_color();
	DirectX::XMFLOAT3 get_direction();

private:
	DirectX::XMFLOAT4 m_diffuseColor;
	DirectX::XMFLOAT3 m_direction;
};