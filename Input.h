#pragma once
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <cassert>
#include <windows.h>
#include <wrl.h>



class Input {
public:
	// namespace省略
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:

	void Initialize(HINSTANCE hInstance, HWND hwnb);
	void Update();

private:
	// DirectInput
	//IDirectInput8* directInput;

	// キーボードデバイス
	ComPtr<IDirectInputDevice8> keyboardDevice;


	//WNDCLASS wc{};

	//HWND hwnd = nullptr;
};