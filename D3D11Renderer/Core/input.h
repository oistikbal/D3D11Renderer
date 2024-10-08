#pragma once
#include <array>
#include <utility> // for std::pair
#include <wtypes.h>

namespace d3d11renderer
{
    class input
    {
    public:
        input();
        ~input();

        void update(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        bool is_key_down(unsigned int key) const;
        bool is_mouse_button_down(unsigned int button) const;
        std::pair<int, int> get_mouse_position() const;
        int get_mouse_wheel_delta() const;


    private:
        // Helper methods for internal handling
        void key_down(unsigned int key);
        void key_up(unsigned int key);
        void mouse_button_down(unsigned int button);
        void mouse_button_up(unsigned int button);
        void set_mouse_position(int x, int y);
        void set_mouse_wheel_delta(int delta);

    private:
        std::array<bool, 256> m_keys;                // Tracks keyboard keys
        std::array<bool, 3> m_mouse_buttons;         // Tracks mouse buttons (left, right, middle)
        std::pair<int, int> m_mouse_position;        // Mouse X, Y coordinates
        int m_mouse_wheel_delta;                     // Tracks mouse wheel scroll
    };
}
