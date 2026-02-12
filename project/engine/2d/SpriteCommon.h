#pragma once  
#include <wrl.h>  
#include <d3d12.h>  
#include "base/DirectXCommon.h"

class SpriteCommon {  
public:  
   // 初期化  
   void Initialize(DirectXCommon* dXCommon);  

   // 共通描画設定  
   void SetCommonPipelineState();  

   // DirectXCommonのゲッター  
   DirectXCommon* GetDXCommon() const { return dXCommon_; }  

private:  
   // ルートシグネイチャの作成  
   void InitializeRootSignature();  
   // グラフィックパイプラインの生成  
   void InitializeGraphicsPipeline();  

   DirectXCommon* dXCommon_;  
   Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_; 
   Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};
