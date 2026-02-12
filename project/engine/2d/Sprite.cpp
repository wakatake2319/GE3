#include "Sprite.h"
#include "base/SpriteCommon.h"
#include <base/Logger.h>
#include "../../TextureManager.h"
using namespace Logger;

void Sprite::Initialize(SpriteCommon* spriteCommon, std::string textureFilePath) {
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
	CreateMaterialResource();
	// マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	MapMaterialResource();

	// マテリアルデータの初期値を書き込む
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();

	// 座標変換行列リソースを作る
	CreateTransformationMatrixResource();
	// 座標変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixDataに割り当てる
	MapTransformationMatrixResource();

	// 単位行列を書き込んでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();

	textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);
	// SRV設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels);

	// Sprite専用のSRV index を1つ決める
	uint32_t textureIndex = 1;

	// CPUハンドル（index分ずらす）
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = spriteCommon_->GetDXCommon()->GetSRVCPUDescriptorHandle(textureIndex);

	// ⑤ GPUハンドルを保存（最重要）
	textureSrvHandleGPU_ = spriteCommon_->GetDXCommon()->GetSRVGPUDescriptorHandle(textureIndex);

	// デバッグ保険
	//assert(textureSrvHandleGPU_.ptr != 0);

	// 単位行列を書き込んでおく
	textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath);

	// テクスチャサイズをイメージに合わせる
	AdjustTextureSize();
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
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点4つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * kVertexCount;
	// 1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
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
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * kIndexCount;
	// インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
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

// 座標変換行列リソースを作る
void Sprite::CreateTransformationMatrixResource() { 
	transformationMatrixResource_ = spriteCommon_->GetDXCommon()->CreateBufferResource(sizeof(TransformationMatrix)); 
	assert(transformationMatrixResource_ != nullptr);
}
// 座標変換行列リソースにデータを書き込む
void Sprite::MapTransformationMatrixResource() {
	// 座標変換行列リソースにデータを書き込む
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
}

// 更新処理
void Sprite::Update() {
	// アンカーポイント-反映処理-
	float left = 0.0f - anchorPoint_.x;
	float right = 1.0f - anchorPoint_.x;
	float top = 0.0f - anchorPoint_.y;
	float bottom = 1.0f - anchorPoint_.y;

	const DirectX::TexMetadata& metadata = 
		TextureManager::GetInstance()->GetMetadata(textureIndex_);
	float tex_left = textureLeftTop_.x / metadata.width;
	float tex_right = (textureLeftTop_.x + textureSize_.x) / metadata.width;
	float tex_top = textureLeftTop_.y / metadata.height;
	float tex_bottom = (textureLeftTop_.y + textureSize_.y) / metadata.height;


	// 左右反転
	if (isFlipX_) {
		left = -left;
		right = -right;
		tex_left = -tex_left;
		tex_right = -tex_right;
	}
	// 上下反転
	if (isFlipY_) {
		top = -top;
		bottom = -bottom;
		tex_top = -tex_top;
		tex_bottom = -tex_bottom;
	}




	// 頂点リソースにデータを書き込む(4点分)
	// 拡縮-反映処理-
	// 左下
	vertexData[0].position = {left, bottom, 0.0f, 1.0f};
	vertexData[0].texcoord = {tex_left, tex_bottom};
	vertexData[0].normal = {0.0f, 0.0f, -1.0f};
	// 左上
	vertexData[1].position = {left, top, 0.0f, 1.0f};
	vertexData[1].texcoord = {tex_left, tex_top};
	vertexData[1].normal = {0.0f, 0.0f, -1.0f};

	// 右下
	vertexData[2].position = {right, bottom, 0.0f, 1.0f};
	vertexData[2].texcoord = {tex_right, tex_bottom};
	vertexData[2].normal = {0.0f, 0.0f, -1.0f};

	// 右上
	vertexData[3].position = {right, top, 0.0f, 1.0f};
	vertexData[3].texcoord = {tex_right, tex_top};
	vertexData[3].normal = {0.0f, 0.0f, -1.0f};

	// インデックスリソースにデータを書き込む(6点分)
	for (uint32_t lat = 0; lat < kSubdivision; ++lat) {
		for (uint32_t lon = 0; lon < kSubdivision; ++lon) {
			indexData[0] = 0;
			indexData[1] = 1;
			indexData[2] = 2;
			indexData[3] = 1;
			indexData[4] = 3;
			indexData[5] = 2;
		}
	}
	// Transform情報を作る
	Transform transform{
	    {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	// 座標-反映処理-
	transform.translate = {position_.x, position_.y, 0.0f};
	// 回転-反映処理-
	transform.rotate = {0.0f, 0.0f, rotation_};
	// 拡縮-反映処理-
	transform.scale = {size_.x, size_.y, 1.0f};

	// TransformからWorldMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	
	// ViewMatrixを作って単位行列を代入
	Matrix4x4 viewMatrix = MakeIdentity4x4();

	// ProjectionMatrixを作って平行投影行列を書き込む
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WindowsAPI::kClientWidth), float(WindowsAPI::kClientHeight), 0.0f, 100.0f);

	transformationMatrixData->WVP = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->World = worldMatrix;


}

// 描画処理
void Sprite::Draw() {
	// VertexBufferViewを設定
	spriteCommon_->GetDXCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	
	// IndexBufferViewを設定
	spriteCommon_->GetDXCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);

	// マテリアルCBufferの場所を設定
	spriteCommon_->GetDXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	
	// 座標変換行列CBufferの場所を設定
	spriteCommon_->GetDXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

	// SRVのDescriptorTableの先頭を設定
	spriteCommon_->GetDXCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));

	// 描画(DrawCall)
	spriteCommon_->GetDXCommon()->GetCommandList()->DrawIndexedInstanced(kIndexCount, 1, 0, 0, 0);
}

//void Sprite::cahngeTexture(std::string textureFilePath) { textureIndex_ = TextureManager::GetInstance()->GetTextureIndexByFilePath(textureFilePath); }

// ImGui表示
void Sprite::spriteImGui(int index) {
	ImGui::PushID(index);

	ImGui::Checkbox("IsFlipX", &isFlipX_);
	ImGui::Checkbox("IsFlipY", &isFlipY_);

	// テクスチャ切り取りサイズ
	ImGui::DragFloat2("TextureCutSize", &textureSize_.x, 1.0f, 0.0f, 4096.0f);


	ImGui::PopID();
}

// テクスチャサイズをイメージに合わせる
void Sprite::AdjustTextureSize() {
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance()->GetMetadata(textureIndex_);
	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);

	// 画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;

}

