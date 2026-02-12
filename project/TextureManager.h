#pragma once
#include <string>
#include <DirectXTex/DirectXTex.h>
#include <wrl.h>
#include <d3d12.h>

// 前方クラス
class DirectXCommon;

class TextureManager {
private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	DirectXCommon* dXCommon_ = nullptr;

	struct TextureData {
		// 画像ファイルのパス
		std::string filePath;
		// 画像の幅や高さなどの情報
		DirectX::TexMetadata metadata;
		// テクスチャリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		// 中間用リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
		// SRV作成時に必要なCPUハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		// 描画コマンドに必要なGPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;	
	};

	// テクスチャデータ
	std::vector<TextureData> textureDatas;

	// SRVインデックスの開始番号
	static uint32_t kSRVIndexTop;


public:

	// 初期化
	void Initialize(DirectXCommon* dxCommon);

	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();

	// 終了
	void Finalize();

	void LoadTexture(const std::string& filePath);

	// SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	// テクスチャ番号からGPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

	// メタデータを取得
	const DirectX::TexMetadata& GetMetadata(uint32_t textureIndex);
};
