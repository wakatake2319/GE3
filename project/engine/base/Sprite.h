#pragma once
#include "base/MathTypes.h"
#include "base/Math.h"
#include <wrl.h>  
#include <d3d12.h> 
#include "base/DirectXCommon.h"


// 前方宣言
class SpriteCommon;

class Sprite {
public:
	struct VertexData {
		Vector4 position;
		Vector2 texcoord;
		Vector3 normal;
	};
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float paddding[3];
		Matrix4x4 uvTransform;
	};
	// 座標変換行列データ
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 world;
	};

	static const uint32_t kVertexCount = 4;
	static const uint32_t kIndexCount = 6;

	// 初期化
	void Initialize(SpriteCommon* spriteCommon);

	// spriteのゲッター
	//SpriteCommon* GetSpriteCommon() const { return spriteCommon_; }


private:
	// VertexResourceを作る
	void CreateVertexResource();
	// VertexResourceにデータを書き込む
	void MapVertexResource();
	// VertexBufferViewを作る
	void CreateVertexBufferView();


	// IndexResourceを作る
	void CreateIndexResource();
	// IndexResourceにデータを書き込む
	void MapIndexResource();
	// IndexBufferViewを作る
	void CreateIndexBufferView();

	// マテリアルリソースを作る
	void CreateMaterialResource();
	// マテリアルリソースにデータを書き込む
	void MapMaterialResource();


	SpriteCommon* spriteCommon_ = nullptr;

	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	// マテリアルソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	// バッファリソース内のデータを刺すポイント
	VertexData* vertexData = nullptr;
	uint32_t* indexData = nullptr;
	Material* materialData = nullptr;
	// バッファリソースの使い道を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
};
