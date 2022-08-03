#include "GameBase.h"
#include "WinApp.h"

/// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE,LPSTR,int)
{
	auto game = new GameBase();
	//ゲーム実行
	game->Run();
	//解放
	delete game;

	return 0;
}
