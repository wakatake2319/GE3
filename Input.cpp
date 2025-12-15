#include "Input.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


// 初期化処理
void Input::Initialize(WindowsAPI* windowsAPI) {

	// 借りてきたWindowsAPIのインスタンスを記録
	this->inputWindowsAPI_ = windowsAPI;

	// 関数が成功したかどうかをSUCCEEDEマクロで判断できる
	HRESULT hr;

	// DirectInputの初期化
	hr = DirectInput8Create(windowsAPI->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));

	// キーボードデバイスの生成
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboardDevice, NULL);
	assert(SUCCEEDED(hr));

	// キーボードデバイスの協調レベルの設定
	hr = keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	// 排他制御レベルのセット
	hr = keyboardDevice->SetCooperativeLevel(windowsAPI->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

}

// 更新処理
void Input::Update() {

	// 前回のキーの状態を保存
	memcpy(preKey, key, sizeof(key));

	// キーボード情報の取得開始
	keyboardDevice->Acquire();

	keyboardDevice->GetDeviceState(sizeof(key), key);

	// ゲーム処理
	if (TriggerKey(DIK_SPACE)) {
		OutputDebugStringA("Press Space\n");
	}

}

// キーが押されたかどうかを調べる関数
bool Input::PushKey(BYTE keyNumber) {
	// 指定キーが押せていればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	// 押されていなければfalseを返す
	return false;
}

// キーがトリガーされたかどうかを調べる関数
bool Input::TriggerKey(BYTE keyNumber) {
	//  指定キーが押せていればtrueを返す
	if (key[keyNumber] && !preKey[keyNumber]) {
		return true;
	}
	// 押されていなければfalseを返す
	return false;
}