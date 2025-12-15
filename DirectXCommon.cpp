#include "DirectXCommon.h"
#include <cassert>
#include <filesystem>
#include "Logger.h"
#include "stringUtility.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <format>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace Microsoft::WRL;
using namespace Logger;
using namespace stringUtility;

// 初期化
void DirectXCommon::Initialize(WindowsAPI* windowsAPI) {

	// NULL検出
	assert(windowsAPI);

	// 借りてきたWindowsAPIのインスタンスを記録
	this->directXWindowsAPI_ = windowsAPI;

	// デバイスの生成
	InitializeDevice();

	// コマンド関連の初期化
	InitializeCommand();

	// スワップチェーンの生成
	InitializeSwapChain();

	// 深度バッファの生成
	InitializeDepthBuffer();

	// 各種でスクリプタヒープの生成
	InitializeDescriptorHeaps();

	// レンダーターゲットビューの生成
	InitializeRenderTargetViews();

	// 深度ステンシルビューの生成
	InitializeDepthStencilView();

	// フェンスの初期化
	InitializeFence();

	// ビューポート矩形の初期化
	InitializeViewport();

	// シザリング矩形の初期化
	InitializeScissorRect();

	// DXCコンパイラの生成
	InitializeDXCCompiler();

	// ImGuiの初期化
	InitializeImGui();

}

// デバイスの初期化
void DirectXCommon::InitializeDevice() {
	// デバックレイヤーをオンにする
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	// 関数が成功したかどうかをSUCCEEDEマクロで判断できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
		// アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
		assert(SUCCEEDED(hr));
		// ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。wstringのほうなので注意
			Log(ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		// ソフトウェアアダプタの場合は見なかったことにする
		useAdapter = nullptr;
	}
	// 適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);


	// デバイスの生成
	device_ = nullptr;



	// エラー時にブレークを発生させる設定

	
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12.0"};
	// 高い順に生成ができるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		if (SUCCEEDED(hr)) {
			Log(std::format("featureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	assert(device_ != nullptr);
	Log(ConvertString(L"Complete create D3D12Device!!!\n"));
	

//	#ifdef _DEBUG
//ComPtr<ID3D12Debug1> debugController;
//if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
//	debugController->EnableDebugLayer();
//	debugController->SetEnableGPUBasedValidation(TRUE);
//}
//#endif
//
//HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
//assert(SUCCEEDED(hr));
//
//ComPtr<IDXGIAdapter4> useAdapter;
//for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
//
//	DXGI_ADAPTER_DESC3 desc{};
//	useAdapter->GetDesc3(&desc);
//
//	if (!(desc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
//		Log(ConvertString(std::format(L"Use Adapter: {}\n", desc.Description)));
//		break;
//	}
//	useAdapter.Reset();
//}
//assert(useAdapter);
//
//D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
//
//for (auto level : levels) {
//	hr = D3D12CreateDevice(
//		useAdapter.Get(), level,
//		IID_PPV_ARGS(&device) // ★メンバに代入
//	);
//	if (SUCCEEDED(hr))
//		break;
//}
//assert(device);
}

// コマンド関連の初期化
void DirectXCommon::InitializeCommand() {

	HRESULT hr;

	// ===============================
	// 命令保存用のメモリ管理機構
	// ===============================
	// コマンドアロケータを生成する
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドリストは生成直後は記録モードなので閉じておく
	//commandList->Close();
}

// スワップチェーンの生成
void DirectXCommon::InitializeSwapChain() {
	HRESULT hr;
	// =================================
	// モニター表示
	// =================================
	// スワップチェーンを生成する
	//Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WindowsAPI::kClientWidth;
	swapChainDesc.Height = WindowsAPI::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// コマンドキュー、ウィンドウハンドル、設定して生成する
	hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), directXWindowsAPI_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));

	// ==============================================================================
	// バックバッファを取得
	for (UINT i = 0; i < 2; ++i) {
		hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		assert(SUCCEEDED(hr));
	}
	// ==============================================================================
}



// 深度バッファの生成
void DirectXCommon::InitializeDepthBuffer() {
	HRESULT hr;

	// 深度バッファリソース設定
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = WindowsAPI::kClientWidth;
	depthResourceDesc.Height = WindowsAPI::kClientHeight;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	//depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.SampleDesc.Quality = 0;
	depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// 深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	// リソース生成
	hr = device_->CreateCommittedResource(
	    &heapProps, D3D12_HEAP_FLAG_NONE, &depthResourceDesc,
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 初期状態を深度書き込みにする
	    &depthClearValue, IID_PPV_ARGS(&depthBuffer_));
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		Log(std::format("CreateCommittedResource for depthBuffer failed: 0x{:08X}\n", static_cast<unsigned>(hr)));
		return;
	}
}

// 各種でスクリプタヒープの生成
void DirectXCommon::InitializeDescriptorHeaps() {
	//HRESULT hr;

	// SRV用のDescriptorSizeを取得	
	descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// RTV用のDescriptorSizeを取得
	descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// DSV用のDescriptorSizeを取得
	descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	


	// RTV用ディスクリプタヒープの生成
	rtvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	// SRV用ディスクリプタヒープの生成
	srvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	// DSV用ディスクリプタヒープの生成。Shaderから触らないのでShaderVisibleはfalse
	dsvDescriptorHeap_ = CreateDescriptorHeap(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}

// デスクリプタヒープ生成関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
    DirectXCommon::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible) {
	HRESULT hr;
	// デスクリプタヒープの設定
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = type;                                                                                         // デスクリプタヒープのタイプ
	descriptorHeapDesc.NumDescriptors = numDescriptors;                                                                     // デスクリプタ数
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE; // シェーダから見えるかどうか
	descriptorHeapDesc.NodeMask = 0;                                                                                        // マルチアダプタ用。0固定
	// デスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// デスクリプタヒープの生成に失敗したので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

// レンダーターゲットビューの初期化
void DirectXCommon::InitializeRenderTargetViews() {

	//HRESULT hr;
	// SwapCainからResourceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2];
	
	
	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	
	// RTVハンドルの要素数を2個に変更する
	
	
	// 裏表の2つ分RTVを作成
	for (uint32_t i = 0; i < 2; ++i) {
		// RTVハンドルを取得
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles = rtvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
		rtvHandles.ptr += descriptorSizeRTV_ * i;
	
		// レンダーターゲットビューの生成
		device_->CreateRenderTargetView(swapChainResources_[i].Get(), &rtvDesc, rtvHandles);
	
	}

	//// RTVの設定
	//D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//
	//// 裏表の2つ分RTVを作成
	//for (UINT i = 0; i < 2; i++) {
	//	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//	handle.ptr += descriptorSizeRTV * i;
	//
	//	device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, handle);
	//}
}

// CPUデスクリプタハンドル取得関数の定義
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) 
{
   D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
   handle.ptr += descriptorSize * index;
   return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorSize * index;
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) { return GetCPUDescriptorHandle(srvDescriptorHeap_, descriptorSizeSRV_, index); }

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) { return GetGPUDescriptorHandle(srvDescriptorHeap_, descriptorSizeSRV_, index); }


D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVCPUDescriptorHandle(uint32_t index) { return GetCPUDescriptorHandle(rtvDescriptorHeap_, descriptorSizeRTV_, index); }

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVGPUDescriptorHandle(uint32_t index) { return GetGPUDescriptorHandle(rtvDescriptorHeap_, descriptorSizeRTV_, index); }


D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVCPUDescriptorHandle(uint32_t index) { return GetCPUDescriptorHandle(dsvDescriptorHeap_, descriptorSizeDSV_, index); }

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVGPUDescriptorHandle(uint32_t index) { return GetGPUDescriptorHandle(dsvDescriptorHeap_, descriptorSizeDSV_, index); }

// 深度ステンシルビューの初期化
void DirectXCommon::InitializeDepthStencilView() {
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

	// DSVをデスクリプタヒープの先頭に作る
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device_->CreateDepthStencilView(depthBuffer_.Get(), &dsvDesc, dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart());
}

// フェンスの生成
void DirectXCommon::InitializeFence() {
	// フェンスの初期値を0に設定
	fenceValue_ = 0;

	// フェンス生成
	HRESULT hr;

	// 初期値0でFenceを作る
	hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	// フェンスのSignalを持つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}

// ビューポート矩形の初期化
void DirectXCommon::InitializeViewport() {  
   // ビューポートの設定 
   viewport_.TopLeftX = 0.0f;  
   viewport_.TopLeftY = 0.0f;  
   viewport_.Width = static_cast<float>(WindowsAPI::kClientWidth);  
   viewport_.Height = static_cast<float>(WindowsAPI::kClientHeight);  
   viewport_.MinDepth = 0.0f;  
   viewport_.MaxDepth = 1.0f;  
}

// シザリング矩形の初期化
void DirectXCommon::InitializeScissorRect() {
	// シザリング矩形の設定
	scissorRect_.left = 0;
	scissorRect_.top = 0;
	scissorRect_.right = WindowsAPI::kClientWidth;
	scissorRect_.bottom = WindowsAPI::kClientHeight;
}

// DXCコンパイラの生成
void DirectXCommon::InitializeDXCCompiler() {
	HRESULT hr;
	// dxCompilerを初期化
	dxcUtils_ = nullptr;
	dxcCompiler_ = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));
	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	includeHandler_ = nullptr;
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

// ImGuiの初期化
void DirectXCommon::InitializeImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(directXWindowsAPI_->GetHwnd());
	ImGui_ImplDX12_Init(
	    device_.Get(), 2, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvDescriptorHeap_.Get(), srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart());
}

// 描画前処理
void DirectXCommon::PreDraw() {
	// バックバッファの番号取得
	UINT currentBackBufferIndex = swapChain_->GetCurrentBackBufferIndex();
	// リソースバリアで書き込み可能に変更
	D3D12_RESOURCE_BARRIER barrierDesc{};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = swapChainResources_[currentBackBufferIndex].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList_->ResourceBarrier(1, &barrierDesc);

	// 描画先のRTVとDSVを指定する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRTVCPUDescriptorHandle(currentBackBufferIndex);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);
	
	// 画面全体の色をクリア
	float clearColor[] = {0.1f, 0.25f, 0.5f, 1.0f};
	commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// 画面全体の深度をクリア
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// SRV用のディスクリプタヒープを指定する
	ID3D12DescriptorHeap* descriptorHeaps[] = {srvDescriptorHeap_.Get()};
	commandList_->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	// ビューポート領域の設定
	commandList_->RSSetViewports(1, &viewport_);
	
	// シザー矩形の設定
	commandList_->RSSetScissorRects(1, &scissorRect_);

}
// 描画後処理
void DirectXCommon::PostDraw() {

	// バックバッファの番号取得
	UINT currentBackBufferIndex = swapChain_->GetCurrentBackBufferIndex();
	// リソースバリアで表示状態に変更
	D3D12_RESOURCE_BARRIER barrierDesc{};
	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrierDesc.Transition.pResource = swapChainResources_[currentBackBufferIndex].Get();
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList_->ResourceBarrier(1, &barrierDesc);

	// グラフィックスコマンドのクローズ
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	// GPUコマンドの実行
	ID3D12CommandList* commandLists[] = {commandList_.Get()};
	commandQueue_->ExecuteCommandLists(_countof(commandLists), commandLists);
	
	// GPU画面の交換を通知
	hr = swapChain_->Present(1, 0);
	assert(SUCCEEDED(hr));

	// Fenceの値を更新
	fenceValue_++;

	// コマンドキューにシグナルを送る
	hr = commandQueue_->Signal(fence_.Get(), fenceValue_);
	assert(SUCCEEDED(hr));

	// コマンド完了待ち
	if (fence_->GetCompletedValue() < fenceValue_) {
		hr = fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
	// コマンドアロケータのリセット
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));

	// コマンドリストのリセット
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));

}

// シェーダーコンパイル
Microsoft::WRL::ComPtr<IDxcBlob> DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile) {
	// これからシェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin compileShader, path:{}, profile:{}\n", filePath, profile)));
	// ======================
	// hlslファイルを読み込む
	// ======================
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	// UTF8の文字コードであることを通知
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// ======================
	// Compileする
	// ======================
	LPCWSTR arguments[] = {
	    // コンパイル対象のhlslファイル名
	    filePath.c_str(),
	    // エントリーポイントの指定。基本的にmain以外にはしない
	    L"-E",
	    L"main",
	    // shaderProfileの設定
	    L"-T",
	    profile,
	    // デバッグ用の情報を埋め込む
	    L"-Zi",
	    L"-Qembed_debug",
	    // 最適化を外しておく
	    L"-Od",
	    // メモリレイアウトは行優先
	    L"-Zpr",
	};
	// 実際にShaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
	    // 読み込んだファイル
	    &shaderSourceBuffer,
	    // コンパイルオプション
	    arguments,
	    // コンパイルオプションの数
	    _countof(arguments),
	    // includeが含まれた諸々
	    includeHandler_.Get(),
	    // コンパイル結果
	    IID_PPV_ARGS(&shaderResult));
	// コンパイルエラーではなくdxcが起動できないほど致命的な状況
	assert(SUCCEEDED(hr));

	// ==================================
	// 警告・エラーが出てないか確認する
	// ==================================
	// 警告・エラーが出ていたらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告・エラーだめ絶対
		assert(false);
	}

	ComPtr<IDxcBlob> shaderBlob;
	shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	return shaderBlob;
}

// バッファリソースの生成
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes) {

	// デバイスが初期化されていることを保証
	assert(device_);

	// ==========================
	// UploadHeap の設定
	// ==========================
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	// ==========================
	// バッファリソースの設定
	// ==========================
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// ==========================
	// リソース生成
	// ==========================
	Microsoft::WRL::ComPtr<ID3D12Resource> bufferResource;

	HRESULT hr = device_->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&bufferResource));

	assert(SUCCEEDED(hr));

	return bufferResource;
}

// テクスチャリソースの生成

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata) {

	// デバイスが初期化されていることを保証
	assert(device_);

	// ==========================
	// Resource の設定
	// ==========================
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resourceDesc.Width = static_cast<UINT64>(metadata.width);
	resourceDesc.Height = static_cast<UINT>(metadata.height);
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	resourceDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// ==========================
	// Heap の設定
	// ==========================
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	// ==========================
	// Resource 作成
	// ==========================
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;

	HRESULT hr = device_->CreateCommittedResource(
	    &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
	    D3D12_RESOURCE_STATE_COPY_DEST, // Upload → Copy 前提
	    nullptr, IID_PPV_ARGS(&textureResource));

	assert(SUCCEEDED(hr));

	return textureResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages) {

	// 事前条件チェック
	assert(device_);
	assert(commandList_);
	assert(texture);

	// ==========================
	// Subresource 情報を作成
	// ==========================
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device_.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);

	// ==========================
	// Upload 用中間バッファ作成
	// ==========================
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(intermediateSize);

	// ==========================
	// Texture へデータ転送
	// ==========================
	UpdateSubresources(commandList_.Get(), texture.Get(), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// ==========================
	// ResourceBarrier（COPY_DEST → GENERIC_READ）
	// ==========================
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;

	commandList_->ResourceBarrier(1, &barrier);

	// Upload 用リソースは描画完了まで保持する必要があるため返す
	return intermediateResource;
}

// テクスチャ読み込み
DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath) {

	// ファイル存在チェック（デバッグしやすくする）
	assert(std::filesystem::exists(filePath) && "Texture file not found");

	// ------------------------------
	// string → wstring
	// ------------------------------
	std::wstring filePathW = ConvertString(filePath);

	// ------------------------------
	// 画像読み込み
	// ------------------------------
	DirectX::ScratchImage baseImage{};
	HRESULT hr = DirectX::LoadFromWICFile(
	    filePathW.c_str(),
	    DirectX::WIC_FLAGS_FORCE_SRGB, // sRGBとして扱う
	    nullptr, baseImage);
	assert(SUCCEEDED(hr) && "LoadFromWICFile failed");

	// ------------------------------
	// mipmap 生成
	// ------------------------------
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(baseImage.GetImages(), baseImage.GetImageCount(), baseImage.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr) && "GenerateMipMaps failed");

	return mipImages;
}