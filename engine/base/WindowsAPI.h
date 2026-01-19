#pragma once
#include <windows.h>
#include <cstdint>

class WindowsAPI {
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:

	// 初期化
	void Initialize();

	// 終了
	void Finalize();

	// ウィンドウハンドルを取得
	HWND GetHwnd() const { return hwnd; }

	// ウィンドウクラスを取得
	HINSTANCE GetHInstance() const { return wc.hInstance; }

	// メッセージ処理
	bool ProcessMessage();

public:

	// 領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

private:
	// ウィンドウハンドル
	HWND hwnd = nullptr;

	// ウィンドウクラスの設定
	WNDCLASS wc{};

};