#include "MyGame.h"
#include "SRFramework.h"
#include <memory>
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	std::unique_ptr<SRFramework> game = std::make_unique<MyGame>();

	game->Run();

	return 0;
}