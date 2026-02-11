#include "TextureManager.h"
#include "base/DirectXCommon.h"


TextureManager* TextureManager::instance = nullptr;

void TextureManager::Initialize() {
	// SRVの数と同数
	textureDatas.resize(DirectXCommon::kMaxSRVCount);
}

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager();
	}
	return instance;
}

void TextureManager::Finalize() {
	//if (instance != nullptr) {
		delete instance;
		instance = nullptr;
	//}
}

