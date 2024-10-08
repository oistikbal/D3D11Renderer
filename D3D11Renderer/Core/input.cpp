#include "input.h"
#include <windowsx.h>
#pragma once

d3d11renderer::input::input() : m_mouse_position{ 0, 0 }, m_mouse_wheel_delta(0)
{
    m_keys.fill(false);
    m_mouse_buttons.fill(false);
}

d3d11renderer::input::~input()
{
}

void d3d11renderer::input::update(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_KEYDOWN:
        key_down(static_cast<unsigned int>(wparam));
        break;

    case WM_KEYUP:
        key_up(static_cast<unsigned int>(wparam));
        break;

    case WM_LBUTTONDOWN:
        mouse_button_down(0); // Left mouse button
        break;

    case WM_LBUTTONUP:
        mouse_button_up(0); // Left mouse button
        break;

    case WM_RBUTTONDOWN:
        mouse_button_down(1); // Right mouse button
        break;

    case WM_RBUTTONUP:
        mouse_button_up(1); // Right mouse button
        break;

    case WM_MOUSEMOVE:
    {
        int xPos = GET_X_LPARAM(lparam);
        int yPos = GET_Y_LPARAM(lparam);
        set_mouse_position(xPos, yPos);
        break;
    }

    case WM_MOUSEWHEEL:
    {
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wparam);
        set_mouse_wheel_delta(wheelDelta);
        break;
    }

    default:
        break;
    }
}

void d3d11renderer::input::key_down(unsigned int input)
{
	m_keys[input] = true;
}

void d3d11renderer::input::key_up(unsigned int input)
{
	m_keys[input] = false;
}

bool d3d11renderer::input::is_key_down(unsigned int key) const
{
	return m_keys[key];
}

void d3d11renderer::input::mouse_button_down(unsigned int button)
{
    if (button < m_mouse_buttons.size()) {
        m_mouse_buttons[button] = true;
    }
}

void d3d11renderer::input::mouse_button_up(unsigned int button)
{
    if (button < m_mouse_buttons.size()) {
        m_mouse_buttons[button] = false;
    }
}

bool d3d11renderer::input::is_mouse_button_down(unsigned int button) const
{
    if (button < m_mouse_buttons.size()) {
        return m_mouse_buttons[button];
    }
    return false;
}

void d3d11renderer::input::set_mouse_position(int x, int y)
{
    m_mouse_position = { x, y };
}

std::pair<int, int> d3d11renderer::input::get_mouse_position() const
{
    return m_mouse_position;
}

void d3d11renderer::input::set_mouse_wheel_delta(int delta)
{
    m_mouse_wheel_delta = delta;
}

int d3d11renderer::input::get_mouse_wheel_delta() const
{
    return m_mouse_wheel_delta;
}
