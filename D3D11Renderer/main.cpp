#include <windows.h>
#include "system.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::unique_ptr<d3d11renderer::system> system = std::make_unique<d3d11renderer::system>();
	system->run();
	return 0;
}
