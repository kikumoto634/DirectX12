#pragma once
#include <functional>

/// <summary>
/// 時間発動
/// </summary>
class TimedCall
{
public:
	//コンストラクタ
	TimedCall(std::function<void(void)> func1, std::function<void(void)> func2, uint32_t time);
	//更新
	void Update();
	//完了ならtrueを返す
	bool IsFinished()	{return IsFlag;}

private:
	//コールバック
	//繰り返し
	std::function<void(void)> func1;
	//イベント
	std::function<void(void)> func2;
	//残り時間
	uint32_t time = 0;
	//完了フラグ
	bool IsFlag = false;
}; 

