#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <map>

/// <summary>
/// サウンド再生管理
/// </summary>
class SoundManager
{
public://1エイリアス
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
	template<class T1, class T2> using map = std::map<T1,T2>;

public://インナークラス
	//チャンクヘッダー
	struct ChunkHeader
	{
		char id[4];//チャンク毎のID
		int size;	//チャンクサイズ
	};
	//RIFFヘッダチェック
	struct RiffHeader
	{
		ChunkHeader chunk;	//"RIFF"
		char type[4];	//"WAVE"
	};
	//FMTチェック
	struct FormatChunk
	{
		ChunkHeader chunk;	//"fmt"
		WAVEFORMATEX fmt;	//波形フォーマット
	};

	//音声データ
	struct SoundData
	{
		//波形フォーマット
		WAVEFORMATEX wfex{};
		//波形データ
		char* pBuffer;
		//波形データのサイズ
		unsigned int dataSize;
	};

public://メンバ関数

	/// <summary>
	///デストラクタ
	/// </summary>
	~SoundManager();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// サウンドファイルの読み込み
	/// </summary>
	/// <param name="number">サウンド番号</param>
	/// <param name="filename">wavファイル名</param>
	void LoadWave(int number, const char* filename);

	//サウンド再生
	void PlayWave(int number);

private://メンバ変数
	//XAudio2のインスタンス
	ComPtr<IXAudio2> xAudio2;
	//マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	//波形データの連想配列
	map<int, SoundData> soundDatas;
};

