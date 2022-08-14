#pragma once
#include <functional>

/// <summary>
/// ���Ԕ���
/// </summary>
class TimedCall
{
public:
	//�R���X�g���N�^
	TimedCall(std::function<void(void)> func, uint32_t time);
	//�X�V
	void Update();
	//�����Ȃ�true��Ԃ�
	bool IsFinished()	{return IsFlag;}

private:
	//�R�[���o�b�N
	std::function<void(void)> func;
	//�c�莞��
	uint32_t time = 0;
	//�����t���O
	bool IsFlag = false;
}; 

