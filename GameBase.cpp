#include "GameBase.h"

void GameBase::Run()
{
	//初期化
	Initialize();

	while (true)
	{
		//終了フラグによってループを抜ける
		if(endFlag){
			break;
		}

		//更新
		Update();

		//描画
		Draw();
	}
	//解放
	Finalize();
}
