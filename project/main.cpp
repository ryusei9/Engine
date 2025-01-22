#include "MyGame.h"
#include "SRFramework.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	SRFramework* game = new MyGame();

	game->Run();

	delete game;

	return 0;
}