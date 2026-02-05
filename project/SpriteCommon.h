#pragma once
class SpriteCommon {
public:
	// 初期化
	void Initialize(DirectXCommon* dXCommon);

	DirectXCommon* GetDXCommon() const { return dXCommon_; }

private:
	// ルートシグネイチャの作成
	void InitializeRootSignature();
	// グラフィックパイプラインの生成
	void InitializeGraphicsPipeline();

	DirectXCommon* dXCommon_;

};
