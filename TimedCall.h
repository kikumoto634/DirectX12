#pragma once
#include <functional>

/// <summary>
/// 時間発動
/// </summary>
class TimedCall
{
public:
	//コンストラクタ
	TimedCall(std::function<void(void)> func, uint32_t time);
	//更新
	void Update();
	//完了ならtrueを返す
	bool IsFinished()	{return IsFlag;}

private:
	//コールバック
	std::function<void(void)> func;
	//残り時間
	uint32_t time = 0;
	//完了フラグ
	bool IsFlag = false;
}; 

