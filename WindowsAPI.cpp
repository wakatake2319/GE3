#include "WindowsAPI.h"
#include "externals/imgui/imgui.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


// ウィンドウプロシージャ
LRESULT CALLBACK WindowsAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	// メッセージに応じた処理
	switch (msg) {
	case WM_DESTROY:        // ウィンドウが破棄された
		PostQuitMessage(0); // OSに対してアプリの終了を伝える
		return 0;
	}
	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

// 初期化
void WindowsAPI::Initialize() {
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);



	wc.lpfnWndProc = WindowProc;

	wc.lpszClassName = L"CG2WnidowClas";

	wc.hInstance = GetModuleHandle(nullptr);

	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&wc);



	// ウィンドウサイズを表す構造体にクラインと領域を入れる
	RECT wrc = {0, 0, kClientWidth, kClientHeight};

	// クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	hwnd = CreateWindow(
	    wc.lpszClassName,     // 利用するクラス名
	    L"CG2",               // タイトルバーの文字
	    WS_OVERLAPPEDWINDOW,  // よく見るウィンドウスタイル
	    CW_USEDEFAULT,        // 表示X座標
	    CW_USEDEFAULT,        // 表示Y座標
	    wrc.right - wrc.left, // ウィンドウ横幅
	    wrc.bottom - wrc.top, // ウィンドウ縦幅
	    nullptr,              // 親ウィンドウハンドル
	    nullptr,              // メニューハンドル
	    wc.hInstance,         // インスタンスハンドル
	    nullptr               // オプション
	);

		// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
}

// 終了
void WindowsAPI::Finalize() {
	CloseWindow(hwnd);
	CoUninitialize();
}

// メッセージ処理
bool WindowsAPI::ProcessMessage() {
	MSG msg{};
	// ウィンドウの×ボタンが押されるまでループ
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT) {
		return true;
	}

	return false;
}