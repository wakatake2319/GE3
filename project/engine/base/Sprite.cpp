#include "Sprite.h"
#include "SpriteCommon.h"
#include <base/Logger.h>
using namespace Logger;

void Sprite::Initialize(SpriteCommon* spriteCommon) {
	this->spriteCommon_ = spriteCommon;

	// VertexResourceを作る
	CreateVertexResource();
	// VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	MapVertexResource();
	// VertexBufferViewを作成する(値を設定するだけ)
	CreateVertexBufferView();


	// IndexResourceを作る
	CreateIndexResource();
	// IndexResourceにデータを書き込むためのアドレスを取得してindexDataに割り当てる
	MapIndexResource();
	// IndexBufferViewを作成する(値を設定する)
	CreateIndexBufferView();

	// マテリアルリソースを作る

	// マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる

	// マテリアルデータの初期値を書き込む
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();
}

// VertexResourceを作る
void Sprite::CreateVertexResource() {
	// VertexResourceを作る
	vertexResource_ = spriteCommon_->GetDXCommon()->CreateBufferResource(sizeof(VertexData) * kVertexCount);
	assert(vertexResource_ != nullptr);
}

// VertexBufferViewを作る
void Sprite::CreateVertexBufferView() {
	// 頂点バッファビューを作成する
	vertexBufferView.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * kVertexCount;
	// 1頂点当たりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);
}

// VertexResourceにデータを書き込む
void Sprite::MapVertexResource() {
	// 書き込むためのアドレス取得
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
}

// IndexResourceを作る
void Sprite::CreateIndexResource() {
	indexResource_ = spriteCommon_->GetDXCommon()->CreateBufferResource(sizeof(uint32_t) * kIndexCount);
	assert(indexResource_ != nullptr);
}

// IndexBufferViewを作る
void Sprite::CreateIndexBufferView() {
	// インデックスバッファビューを作成する
	indexBufferView.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * kIndexCount;
	// インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

// IndexResourceにデータを書き込む
void Sprite::MapIndexResource() {
	// インデックスリソースにデータを書き込む
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
}

// マテリアルリソースを作る
void Sprite::CreateMaterialResource() {
	materialResource_ = spriteCommon_->GetDXCommon()->CreateBufferResource(sizeof(Material));
	assert(materialResource_ != nullptr);
}

// マテリアルリソースにデータを書き込む
void Sprite::MapMaterialResource() {
	// マテリアルリソースにデータを書き込む
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
}