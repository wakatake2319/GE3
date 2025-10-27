#pragma once
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <cassert>
#include <wrl.h>
#include <windows.h>


using namespace Microsoft::WRL;

class Input {
public:

	void Initialize(HINSTANCE hInstance, HWND hwnb);
	void Update();

private:
	// DirectInput
	//IDirectInput8* directInput;

	// キーボードデバイス
	//IDirectInputDevice8* keyboardDevice = nullptr;



	//WNDCLASS wc{};

	//HWND hwnd = nullptr;
};