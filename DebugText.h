#pragma once
#include <string>
#include <Windows.h>
#include <d3d12.h>

#include "Sprite.h"

//デバック用文字列クラスの定義
class DebugText
{
public:

	static const int maxCharCount = 256;	//最大文字数
	static const int fontWidth = 9;			//フォント画像内1文字内の横幅
	static const int fontHeight = 18;		//フォント画像内1文字内の縦幅
	static const int fontLineCount = 14;	//フォント画像内1行分の文字数

	~DebugText();

	void Initialize(UINT texnumber);

	void Print(const std::string& text, float x, float y, float scale);

	void DrawAll(ID3D12GraphicsCommandList* commandList);

private:
	Sprite* sprites[maxCharCount];
	int spriteIndex = 0;
};