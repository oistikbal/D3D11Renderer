#pragma once

#include <directxmath.h>

class light
{
public:
	light();
	~light();


	void set_diffuse_color(float, float, float, float);
	void set_ambient_color(float, float, float, float);
	void set_direction(float, float, float);
	void set_specular_color(float, float, float, float);
	void set_specular_power(float);

	DirectX::XMFLOAT4 get_diffuse_color();
	DirectX::XMFLOAT4 get_ambient_color();
	DirectX::XMFLOAT3 get_direction();
	DirectX::XMFLOAT4 get_specular_color();
	float get_specular_power();

private:
	DirectX::XMFLOAT4 m_diffuseColor;
	DirectX::XMFLOAT4 m_ambientColor;
	DirectX::XMFLOAT3 m_direction;
	DirectX::XMFLOAT4 m_specularColor;
	float m_specularPower;
};