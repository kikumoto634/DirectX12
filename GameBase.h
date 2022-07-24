#pragma once

//Windows
#include "WinApp.h"
//基盤
#include "DirectXCommon.h"
//キーボード入力
#include "Input.h"
//テクスチャマネージャー
#include "TextureManager.h"
//スプライト
#include "Sprite.h"
//幾何学モデル
#include "GeometryModel.h"
//幾何学オブジェクト
#include "GeometryObject3D.h"
//デバックテキスト
#include "DebugText.h"
//サウンド
#include "SoundManager.h"


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
	virtual void Run();

	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update();

	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// 解放
	/// </summary>
	virtual void Finalize();

protected:
	//終了フラグ
	bool endFlag = false;

	//WindowsAPIオブジェクト
	std::unique_ptr<WinApp> winApp;

	//DirectXオブジェクト
	std::unique_ptr<DirectXCommon> dxCommon = nullptr;

	//入力
	std::unique_ptr<Input> input = nullptr;

	//テクスチャマネージャー
	std::unique_ptr<TextureManager> textureManager = nullptr;

	//モデル
	std::unique_ptr<GeometryModel> geometryModel = nullptr;

	//デバック
	std::unique_ptr<DebugText> debugText = nullptr;

	//サウンドマネージャー
	std::unique_ptr<SoundManager> soundManager = nullptr;
};

