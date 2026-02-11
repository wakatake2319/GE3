#pragma once
#include <string>
#include <DirectXTex/DirectXTex.h>
#include <wrl.h>
#include <d3d12.h>
class TextureManager {
private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator=(TextureManager&) = delete;

	struct TextureData {
		// 画像ファイルのパス
		std::string filePath;
		// 画像の幅や高さなどの情報
		DirectX::TexMetadata metadata;
		// テクスチャリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
		// SRV作成時に必要なCPUハンドル
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		// 描画コマンドに必要なGPUハンドル
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;	
	};

	// テクスチャデータ
	std::vector<TextureData> textureDatas;

public:
	// シングルトンインスタンスの取得
	static TextureManager* GetInstance();

	// 終了
	void Finalize();


};
