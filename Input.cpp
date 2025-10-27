#include "Input.h"




// 初期化処理
void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {

	// 関数が成功したかどうかをSUCCEEDEマクロで判断できる
	HRESULT hr;

	// DirectInputの初期化
	ComPtr<IDirectInput8> directInput = nullptr;
	hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードデバイスの生成
	ComPtr<IDirectInputDevice8> keyboardDevice;
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboardDevice, NULL);
	assert(SUCCEEDED(hr));

	// キーボードデバイスの協調レベルの設定
	hr = keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	// 排他制御レベルのセット
	hr = keyboardDevice->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

}

// 更新処理
void Input::Update() {
}