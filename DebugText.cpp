#include "DebugText.h"

DebugText::~DebugText()
{
	for(int i = 0; i < _countof(sprites); i++)
	{
		//スプライトを解放
		delete sprites[i];
		sprites[i] = nullptr;
	}
}

void DebugText::Initialize(UINT texnumber)
{
	for(int i = 0; i < _countof(sprites); i++)
	{
		//スプライト生成
		sprites[i] = new Sprite();
		//スプライトを初期化する
		sprites[i]->Initialize(texnumber);
	}
}

//一文字追加
void DebugText::Print(const std::string& text, float x, float y, float scale = 1.0f)
{
	//すべての文字について
	for(int i = 0; i < text.size(); i++)
	{
		//最大文字数
		if(spriteIndex >= maxCharCount)
		{
			break;
		}

		//一文字取り出す(※ASCIIコードでしか成り立たない)
		const unsigned char& character = text[i];

		int fontIndex = character - 32;
		if(character >= 0x7f)
		{
			fontIndex = 0;
		}

		int fontIndexY = fontIndex / fontLineCount;
		int fontIndexX = fontIndex % fontLineCount;

		//座標計算
		sprites[spriteIndex]->SetPosition({x + fontWidth * scale * i, y});
		sprites[spriteIndex]->SetTextureRect((float)fontIndexX * fontWidth, (float)fontIndexY * fontHeight, (float)fontWidth, (float)fontHeight);
		sprites[spriteIndex]->SetSize({fontWidth * scale, fontHeight * scale});

		//文字を一つ進める
		spriteIndex++;
	}
}

void DebugText::DrawAll(ID3D12GraphicsCommandList* commandList)
{
	//すべての文字のスプライトについて
	for(int i = 0; i < spriteIndex; i++)
	{
		//スプライト描画
		sprites[i]->Draw(commandList);
	}
	spriteIndex = 0;
}