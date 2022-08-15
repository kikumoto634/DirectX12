#pragma once
#include <functional>

/// <summary>
/// ���Ԕ���
/// </summary>
class TimedCall
{
public:
	//�R���X�g���N�^(�^�C�}�[���Z�b�g�A�C�x���g)
	TimedCall(std::function<void(void)> func1, std::function<void(void)> func2, uint32_t time);
	//�X�V
	void Update();
	//�����Ȃ�true��Ԃ�
	bool IsFinished()	{return IsFlag;}

private:
	//�R�[���o�b�N
	//�J��Ԃ�
	std::function<void(void)> func1;
	//�C�x���g
	std::function<void(void)> func2;
	//�c�莞��
	uint32_t time = 0;
	//�����t���O
	bool IsFlag = false;
}; 

