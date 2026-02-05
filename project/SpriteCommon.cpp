#include "SpriteCommon.h"  
#include <wrl.h>  
#include <d3d12.h>  
#include "engine/base/DirectXCommon.h"  

// 初期化  
void SpriteCommon::Initialize(DirectXCommon* dXCommon) {  
	// 引数で受け取ってメンバ変数に記録する
	dXCommon_ = dXCommon;
	// グラフィックスパイプラインの生成
	InitializeGraphicsPipeline();
 
}
