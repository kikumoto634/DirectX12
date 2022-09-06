#pragma once

//#include <Windows.h>

//キーボード入力
#define DIRECTINPUT_VERSION 0x0800	//DirectInputのバージョン指定
#include <dinput.h>

const int KeyNum = 256;

class Input
{
public:
	Input();
	~Input() = default;

	//初期化
	void Initialize(HWND hwnd);

	//毎フレーム
	void Update();

	//キー入力check
	bool Push(int keyNumber);
	
	bool Trigger(int keyNumber);

private:
	//キーボードデバイス
	IDirectInputDevice8* keyboard = nullptr;

	//キー情報
	BYTE key[KeyNum] = {};
	//前回のキー入力
	BYTE prekey[KeyNum] = {};
};

