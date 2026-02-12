#include "TextureManager.h"
#include "base/DirectXCommon.h"
#include <io/Input.h>


TextureManager* TextureManager::instance = nullptr;
// ImGuiで0番を使用するため、1番から使用開始
uint32_t TextureManager::kSRVIndexTop = 1;

void TextureManager::Initialize(DirectXCommon* dxCommon) {
	dXCommon_ = dxCommon;

	// SRVの数と同数
	textureDatas.reserve(DirectXCommon::kMaxSRVCount);
}

TextureManager* TextureManager::GetInstance() {
	if (instance == nullptr) {
		instance = new TextureManager();
	}
	return instance;
}

void TextureManager::Finalize() {
		delete instance;
		instance = nullptr;
}

// テクスチャの読み込み
void TextureManager::LoadTexture(const std::string& filePath) {
	// 読み込みのテクスチャを検索
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
	// mipmapの作成
	HRESULT hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr) && "GenerateMipMaps failed");

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
	uint32_t srvIndex = static_cast<uint32_t>(textureDatas.size() - 1) + kSRVIndexTop;

	textureData.srvHandleCPU = dXCommon_->GetSRVCPUDescriptorHandle(srvIndex);
	textureData.srvHandleGPU = dXCommon_->GetSRVGPUDescriptorHandle(srvIndex);

	// SRVの生成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};

	// SRVの設定を行う
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = textureData.metadata.format; // ★これが必要
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = UINT(textureData.metadata.mipLevels);
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f; // 設定をもとにSRVの生成
	dXCommon_->GetDevice()->CreateShaderResourceView(textureData.resource.Get(), &srvDesc, textureData.srvHandleCPU);

	// 転送用に生成した中間リソースをテクスチャデータ構造体に格納
	Input::ComPtr<ID3D12Resource> intermediateResource = textureData.intermediateResource = dXCommon_->UploadTextureData(textureData.resource, mipImages);

	// テクスチャ枚数上限をチェック
	assert(textureDatas.size() + kSRVIndexTop < DirectXCommon::kMaxSRVCount);

	// commandListをcloseし、commandQueue->ExecuteCommandListsを使いキックする
	ID3D12CommandList* commandLists[] = {dXCommon_->GetCommandList()};
	dXCommon_->GetCommandList()->Close();
	dXCommon_->GetCommandQueue()->ExecuteCommandLists(1, commandLists);

	// 実行を待つ
	dXCommon_->Signal();     // フェンス値を進める
	dXCommon_->WaitForGPU(); // GPU完了待ち

	// 実行が完了したので、allocatorとcommandListをリセットして次のコマンドを積めるようにする
	dXCommon_->GetCommandAllocator()->Reset();
	dXCommon_->GetCommandList()->Reset(dXCommon_->GetCommandAllocator(), nullptr);

	// ここまで来たら転送は終わっているので、intermediateResourceを解放しても良い
	textureData.intermediateResource.Reset();
	
}

// SRVインデックスの開始番号
uint32_t TextureManager::GetTextureIndexByFilePath(const std::string& filePath) {
	// 読み込みのテクスチャを検索
	auto it = std::find_if(
		textureDatas.begin(), 
		textureDatas.end(), 
		[&](const TextureData& textureData) { return textureData.filePath == filePath; });
	if (it != textureDatas.end()) {
		// 読み込み済みなら要素番号を返す
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textureDatas.begin(), it));
		return textureIndex;
	}

	assert(0);
	return 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureIndex) {
	// 範囲外指定速度をチェック
	assert(textureIndex < textureDatas.size());

	TextureData& textureData = textureDatas[textureIndex];
	return textureData.srvHandleGPU;

}

// メタデータを取得
const DirectX::TexMetadata& TextureManager::GetMetadata(uint32_t textureIndex) {
	// 範囲外指定速度をチェック
	assert(textureIndex < textureDatas.size());
	TextureData& textureData = textureDatas[textureIndex];
	return textureData.metadata;
}