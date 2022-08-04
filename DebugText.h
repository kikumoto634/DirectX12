#pragma once
#include <string>
#include <Windows.h>
#include "Sprite.h"

//デバック用文字列クラスの定義
class DebugText
{
public:	//静的メンバ変数

	static const int maxCharCount = 256;	//最大文字数
	static const int fontWidth = 9;			//フォント画像内1文字内の横幅
	static const int fontHeight = 18;		//フォント画像内1文字内の縦幅
	static const int fontLineCount = 14;	//フォント画像内1行分の文字数

public://静的メンバ関数

	static DebugText* GetInstance();


public://メンバ関数

	DebugText() =default;

	~DebugText();

	void Initialize();

	void Print(const std::string& text, float x, float y, float scale = 1.f);

	void DrawAll(ID3D12GraphicsCommandList* commandList);

private:
	uint32_t textureHandle = 0;
	Sprite* sprites[maxCharCount] = {};
	int spriteIndex = 0;

	float posX = 0.f;
	float posY = 0.f;
	float scale = 1.f;
};