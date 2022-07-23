#pragma once
#include "GameBase.h"

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


#include <memory>

class DirectXGame : public GameBase
{
public:
	static const int TextureNum = 2;
	static const int ObjectNum = 5;

public:
	void Initialize() override;

	void Update() override;

	void Draw() override;

	void Finalize() override;

private:
	//WindowsAPIオブジェクト
	std::unique_ptr<WinApp> winApp;

	//DirectXオブジェクト
	std::unique_ptr<DirectXCommon> dxCommon = nullptr;

	std::unique_ptr<Input> input = nullptr;

	//テクスチャマネージャー
	std::unique_ptr<TextureManager> textureManager = nullptr;
	//スプライト
	std::unique_ptr<Sprite> sprite[TextureNum]{};

	//モデル
	std::unique_ptr<GeometryModel> geometryModel = nullptr;
	//オブジェクト
	std::unique_ptr<GeometryObject3D> object[ObjectNum]{};

	//デバック
	std::unique_ptr<DebugText> debugText = nullptr;

	//サウンドマネージャー
	std::unique_ptr<SoundManager> soundManager = nullptr;
};

