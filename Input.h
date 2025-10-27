#pragma once
#define DIRECTINPUT_VERSION 0x0800 // DirectInputのバージョン指定
#include <dinput.h>
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

	bool PushKey(BYTE keyNumber);

	bool TriggerKey(BYTE keyNumber);

private:
	// DirectInput
	ComPtr<IDirectInput8> directInput;

	// キーボードデバイス
	ComPtr<IDirectInputDevice8> keyboardDevice;

	// キーの状態
	BYTE key[256] = {};
	BYTE preKey[256] = {};
};