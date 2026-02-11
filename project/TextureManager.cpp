#include "TextureManager.h"
#include "base/DirectXCommon.h"


TextureManager* TextureManager::instance = nullptr;

void TextureManager::Initialize(DirectXCommon* dxCommon) {
	dXCommon_ = dxCommon;

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

// テクスチャの読み込み
void TextureManager::LoadTexture(const std::string& filePath) {
	// Todo①: 読み込みのテクスチャを検索
	auto it = std::find_if(
		textureDatas.begin(),
		textureDatas.end(), 
		[&](const TextureData& textureData) { return textureData.filePath == filePath; }
	);
	if (it != textureDatas.end()) {
		// 読み込み済みなら早期return
		return;
	}

	DirectX::ScratchImage image{};
	// テクスチャファイルを読んでプログラムで扱えるようにする
	image = DirectXCommon::LoadTexture(filePath);

	DirectX::ScratchImage mipImages{};
	// mipmapを作成
	HRESULT hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);

	// テクスチャデータを追加
	textureDatas.resize(textureDatas.size() + 1);
	// 追加したテクスチャデータの参照を取得する
	TextureData& textureData = textureDatas.back();

	// テクスチャデータ書き込み
	// ファイルパス
	textureData.filePath = filePath;
	// テクスチャメタデータを取得
	textureData.metadata = mipImages.GetMetadata();
	// テクスチャリソースの生成
	textureData.resource = dXCommon_->CreateTextureResource(textureData.metadata);



	// テクスチャデータの要素番号をSRVのインデックスとする
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1);

	textureData.srvHandleCPU = dXCommon_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dXCommon_->GetSRVGPUDescriptorHandle(srvIndex);

	// SRVの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	// SRVの設定を行う
	srvDesc.Format = textureData.metadata.format;
	// 設定をもとにSRVの生成
	dXCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// テクスチャリソースにデータを転送
	dXCommon_->UploadTextureData(textureData.resource, mipImages);
}