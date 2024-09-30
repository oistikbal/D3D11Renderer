#pragma once
#include <array>

namespace d3d11renderer 
{
	class input
	{
	public:
		input();
		~input();

		void key_down(unsigned int);
		void key_up(unsigned int);

		bool is_key_down(unsigned int);

	private:
		std::array<bool, 256> m_keys;
	};
}