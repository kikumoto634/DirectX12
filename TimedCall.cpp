#include "TimedCall.h"

TimedCall::TimedCall(std::function<void(void)> func, uint32_t time):
	func(func),
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
		//�R�[���o�b�N�֐��Ăяo��
		func();
	}
}
