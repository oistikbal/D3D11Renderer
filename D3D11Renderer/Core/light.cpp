#include "light.h"

light::light()
{
}

light::~light()
{
}

void light::set_diffuse_color(float red, float green, float blue, float alpha)
{
	m_diffuseColor = DirectX::XMFLOAT4(red, green, blue, alpha);
	return;
}

void light::set_ambient_color(float red, float green, float blue, float alpha)
{
	m_ambientColor = DirectX::XMFLOAT4(red, green, blue, alpha);
}

void light::set_direction(float x, float y, float z)
{
	m_direction = DirectX::XMFLOAT3(x, y, z);
	return;
}

DirectX::XMFLOAT4 light::get_diffuse_color()
{
	return m_diffuseColor;
}

DirectX::XMFLOAT4 light::get_ambient_color()
{
	return m_ambientColor;
}

DirectX::XMFLOAT3 light::get_direction()
{
	return m_direction;
}
