#include "input.h"
#pragma once

d3d11renderer::input::input()
{
	for (bool& key : m_keys)
	{
		key = false;
	}
}

d3d11renderer::input::~input()
{
}

void d3d11renderer::input::key_down(unsigned int input)
{
	m_keys[input] = true;
}

void d3d11renderer::input::key_up(unsigned int input)
{
	m_keys[input] = false;
}

bool d3d11renderer::input::is_key_down(unsigned int key)
{
	return m_keys[key];
}
