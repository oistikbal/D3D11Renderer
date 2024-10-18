#include "camera.h"

d3d11renderer::camera::camera(std::shared_ptr<d3d11renderer::input> input)
    : m_input(input), m_position(0.0f, 0.0f, 0.0f), m_rotation(0.0f, 0.0f, 0.0f)
{
    m_mouseSensitivity = 0.01f;
    m_lastMouseX = 0;
    m_lastMouseY = 0;
}

d3d11renderer::camera::~camera()
{
}

void d3d11renderer::camera::frame(float deltaTime)
{
    if (m_input->is_key_down('W')) { move_forward(deltaTime); }
    if (m_input->is_key_down('S')) { move_backward(deltaTime); }
    if (m_input->is_key_down('A')) { strafe_left(deltaTime); }
    if (m_input->is_key_down('D')) { strafe_right(deltaTime); }

    auto mousePos = m_input->get_mouse_position();
    float deltaX = (float)(mousePos.first - m_lastMouseX) * m_mouseSensitivity;
    float deltaY = (float)(mousePos.second - m_lastMouseY) * m_mouseSensitivity;

    if (m_input->is_mouse_button_down(1))
    {
        smooth_rotate(deltaX, deltaY, 0.9f);
    }

    // Update last mouse positions
    m_lastMouseX = mousePos.first;
    m_lastMouseY = mousePos.second;

    if (m_rotation.x > maxPitch) { m_rotation.x = maxPitch; }
    if (m_rotation.x < minPitch) { m_rotation.x = minPitch; }
}

void d3d11renderer::camera::set_position(float x, float y, float z)
{
    m_position = DirectX::XMFLOAT3(x, y, z);
}

void d3d11renderer::camera::set_rotation(float x, float y, float z)
{
    m_rotation = DirectX::XMFLOAT3(x, y, z);
}

DirectX::XMFLOAT3 d3d11renderer::camera::get_position()
{
    return m_position;
}

DirectX::XMFLOAT3 d3d11renderer::camera::get_rotation()
{
    return m_rotation;
}

DirectX::XMFLOAT3 d3d11renderer::camera::get_forward()
{
    DirectX::XMFLOAT3 forward;
    forward.x = cos(m_rotation.y) * cos(m_rotation.x);
    forward.y = sin(m_rotation.x);
    forward.z = sin(m_rotation.y) * cos(m_rotation.x);

    // Normalize the forward vector
    float length = sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (length > 0.0f) {
        forward.x /= length;
        forward.y /= length;
        forward.z /= length;
    }
    return forward;
}

DirectX::XMFLOAT3 d3d11renderer::camera::get_right()
{
    DirectX::XMFLOAT3 forward = get_forward();
    DirectX::XMFLOAT3 up = get_up();
    DirectX::XMFLOAT3 right;

    right.x = forward.y * up.z - forward.z * up.y;
    right.y = forward.z * up.x - forward.x * up.z;
    right.z = forward.x * up.y - forward.y * up.x;

    // Normalize the right vector
    float length = sqrt(right.x * right.x + right.y * right.y + right.z * right.z);
    if (length > 0.0f) {
        right.x /= length;
        right.y /= length;
        right.z /= length;
    }

    return right;
}

DirectX::XMFLOAT3 d3d11renderer::camera::get_up()
{
    return DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
}

void d3d11renderer::camera::render()
{
    DirectX::XMFLOAT3 up = get_up();
    DirectX::XMFLOAT3 position = get_position();
    DirectX::XMFLOAT3 lookAt;

    // Calculate the lookAt vector based on yaw (m_rotation.y) and pitch (m_rotation.x)
    lookAt.x = position.x + get_forward().x;
    lookAt.y = position.y + get_forward().y;
    lookAt.z = position.z + get_forward().z;

    m_viewMatrix = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&position),
        DirectX::XMLoadFloat3(&lookAt),
        DirectX::XMLoadFloat3(&up));
}

void d3d11renderer::camera::get_view_matrix(DirectX::XMMATRIX& viewMatrix)
{
    viewMatrix = m_viewMatrix;
}

void d3d11renderer::camera::move_forward(float deltaTime)
{
    DirectX::XMFLOAT3 forward = get_forward();
    m_position.x += forward.x * m_moveSpeed * deltaTime;
    m_position.y += forward.y * m_moveSpeed * deltaTime;
    m_position.z += forward.z * m_moveSpeed * deltaTime;
}

void d3d11renderer::camera::move_backward(float deltaTime)
{
    DirectX::XMFLOAT3 forward = get_forward();
    m_position.x -= forward.x * m_moveSpeed * deltaTime;
    m_position.y -= forward.y * m_moveSpeed * deltaTime;
    m_position.z -= forward.z * m_moveSpeed * deltaTime;
}

void d3d11renderer::camera::strafe_left(float deltaTime)
{
    DirectX::XMFLOAT3 right = get_right();

    m_position.x += right.x * m_moveSpeed * deltaTime;
    m_position.y += right.y * m_moveSpeed * deltaTime;
    m_position.z += right.z * m_moveSpeed * deltaTime;
}

void d3d11renderer::camera::strafe_right(float deltaTime)
{
    DirectX::XMFLOAT3 right = get_right();
    m_position.x -= right.x * m_moveSpeed * deltaTime;
    m_position.y -= right.y * m_moveSpeed * deltaTime;
    m_position.z -= right.z * m_moveSpeed * deltaTime;
}

void d3d11renderer::camera::smooth_rotate(float deltaX, float deltaY, float smoothFactor)
{
    m_rotation.y = (1.0f - smoothFactor) * m_rotation.y + smoothFactor * (m_rotation.y + deltaX);
    m_rotation.x = (1.0f - smoothFactor) * m_rotation.x + smoothFactor * (m_rotation.x + deltaY);

    // Clamp pitch to avoid camera flipping
    if (m_rotation.x > DirectX::XM_PI / 2.0f) { m_rotation.x = DirectX::XM_PI / 2.0f; }
    if (m_rotation.x < -DirectX::XM_PI / 2.0f) { m_rotation.x = -DirectX::XM_PI / 2.0f; }
}
