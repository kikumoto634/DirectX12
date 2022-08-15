#include "TimedCall.h"

TimedCall::TimedCall(std::function<void(void)> func1, std::function<void(void)> func2, uint32_t time):
	func1(func1),
	func2(func2),
	time(time)
{
}

void TimedCall::Update()
{
	if(IsFlag)
	{
		return ;
	}

	time--;
	if(time <= 0)
	{
		IsFlag = true;
		//コールバック関数呼び出し
		func1();
		func2();
	}
}
