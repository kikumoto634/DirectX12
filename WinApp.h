#pragma once

#include <Windows.h>

/// <summary>
/// WindowsAPI
/// </summary>
class WinApp
{
public://staticメンバ変数
	//ウィンドウサイズ
	/* staticはcpp側に置かないといけないが、static const intはこちらでよい*/
	static const int window_width = 1280;	//横幅
	static const int window_height = 720;	//縦幅

public://staticメンバ関数
	/// ウィンドウプロシージャ	(グローバル関数を要求されたらstaticメンバ関数を渡せる)
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public://メンバ関数
	
	///<summary>
	/// デストラクタ
	///</summary>
	~WinApp();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	bool Update();


	/// <summary>
	/// アクセッサ
	/// </summary>
	const HWND& GetHwnd() {return hwnd;}
	const WNDCLASSEX& GetWndClass() {return w;}
	/*WNDCLASSEXはでかい構造体のため、const 参照で取得*/

private://メンバ変数

	HWND hwnd = nullptr;	//ウィンドウハンドル
	WNDCLASSEX w{};			//ウィンドウクラス

	MSG msg{};	//メッセージ
};

