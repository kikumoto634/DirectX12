#pragma once
#include "WinApp.h"
#include "DirectXCommon.h"

#include "DirectXGame.h"

/// <summary>
/// ゲーム共通基盤　(フレームワーク)
/// </summary>
class GameBase
{
/// <summary>
/// メンバ関数
/// </summary>
public:

	/// <summary>
	/// 実行
	/// </summary>
	void Run();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 解放
	/// </summary>
	void Finalize();

protected:
	WinApp* winApp = nullptr;
	DirectXCommon* dxCommon = nullptr;

	//シーン
	DirectXGame* gameScene = nullptr;
};

