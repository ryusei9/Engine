#include "MyGame.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	MyGame game;

	// ゲームの初期化
	game.Initialize();

	while (true) {
		// メインループ
		game.Update();
		// ゲームループを抜ける
		if (game.IsEndRequest()) {
			break;
		}
		// 描画
		game.Draw();
	}
	// ゲームの終了
	game.Finelize();

	return 0;
}