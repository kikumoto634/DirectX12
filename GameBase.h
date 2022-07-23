#pragma once

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
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// 解放
	/// </summary>
	virtual void Finalize() = 0;

protected:
	//終了フラグ
	bool endFlag = false;
};

