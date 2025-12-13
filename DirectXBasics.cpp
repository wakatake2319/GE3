#include "DirectXBasics.h"
#include <cassert>
#include "Logger.h"
#include "stringUtility.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <format>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;
using namespace Logger;
using namespace stringUtility;

// 初期化
void DirectXBasics::Initialize() {

}

// 初期化
void DirectXBasics::Initialize(WindowsAPI* windowsAPI) {

	// NULL検出
	assert(windowsAPI);

	// 借りてきたWindowsAPIのインスタンスを記録
	this->windowsAPI = windowsAPI;

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
void DirectXBasics::InitializeDevice() {
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
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	// 使用するアダプタ用の変数。最初にnullptrを入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	// 良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
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
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;


	// エラー時にブレークを発生させる設定

	
	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};
	const char* featureLevelStrings[] = {"12.2", "12.1", "12.0"};
	// 高い順に生成ができるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			Log(std::format("featureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	assert(device != nullptr);
	Log(ConvertString(L"Complete create D3D12Device!!!\n"));
	
}

// コマンド関連の初期化
void DirectXBasics::InitializeCommand() {

	HRESULT hr;

	// ===============================
	// 命令保存用のメモリ管理機構
	// ===============================
	// コマンドアロケータを生成する
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	// コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	// コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	// コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));
}

// スワップチェーンの生成
void DirectXBasics::InitializeSwapChain() {
	HRESULT hr;
	// =================================
	// モニター表示
	// =================================
	// スワップチェーンを生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WindowsAPI::kClientWidth;
	swapChainDesc.Height = WindowsAPI::kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	// コマンドキュー、ウィンドウハンドル、設定して生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), windowsAPI->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));
}

// 深度バッファの生成
void DirectXBasics::InitializeDepthBuffer() {
	HRESULT hr;

	// 深度バッファリソース設定
	D3D12_RESOURCE_DESC depthResourceDesc{};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = WindowsAPI::kClientWidth;
	depthResourceDesc.Height = WindowsAPI::kClientHeight;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
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
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.DepthStencil.Stencil = 0;

	// リソース生成
	hr = device->CreateCommittedResource(
	    &heapProps, D3D12_HEAP_FLAG_NONE, &depthResourceDesc,
	    D3D12_RESOURCE_STATE_DEPTH_WRITE, // 初期状態を深度書き込みにする
	    &depthClearValue, IID_PPV_ARGS(&depthBuffer));
	assert(SUCCEEDED(hr));
	if (FAILED(hr)) {
		Log(std::format("CreateCommittedResource for depthBuffer failed: 0x{:08X}\n", static_cast<unsigned>(hr)));
		return;
	}
}

// 各種でスクリプタヒープの生成
void DirectXBasics::InitializeDescriptorHeaps() {
	//HRESULT hr;

	// RTV用のDescriptorSizeを取得	
	const uint32_t descriptorSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	// SRV用のDescriptorSizeを取得
	const uint32_t descriptorRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// DSV用のDescriptorSizeを取得
	const uint32_t descriptorDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	// RTV用ディスクリプタヒープの生成
	rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	// SRV用ディスクリプタヒープの生成
	srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	// DSV用ディスクリプタヒープの生成。Shaderから触らないのでShaderVisibleはfalse
	dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}

// デスクリプタヒープ生成関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
    DirectXBasics::CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible) {
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
void DirectXBasics::InitializeRenderTargetViews() {

	//HRESULT hr;
	// SwapCainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = {nullptr};
	

	// RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// RTVハンドルの要素数を2個に変更する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	
	// 裏表の2つ分RTVを作成
	for (uint32_t i = 0; i < 2; ++i) {
		// RTVハンドルを取得
		rtvHandles[i].ptr = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + i * device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// レンダーターゲットビューの生成
		device->CreateRenderTargetView(swapChainResources[i].Get(), &rtvDesc, rtvHandles[i]);

	}

}

// CPUデスクリプタハンドル取得関数の定義
D3D12_CPU_DESCRIPTOR_HANDLE DirectXBasics::GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) 
{
   D3D12_CPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
   handle.ptr += descriptorSize * index;
   return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXBasics::GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += descriptorSize * index;
	return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXBasics::GetSRVCPUDescriptorHandle(uint32_t index) { return GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index); }

D3D12_GPU_DESCRIPTOR_HANDLE DirectXBasics::GetSRVGPUDescriptorHandle(uint32_t index) { return GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, index); }

D3D12_CPU_DESCRIPTOR_HANDLE DirectXBasics::GetRTVCPUDescriptorHandle(uint32_t index) { return GetCPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index); }

D3D12_GPU_DESCRIPTOR_HANDLE DirectXBasics::GetRTVGPUDescriptorHandle(uint32_t index) { return GetGPUDescriptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index); }

D3D12_CPU_DESCRIPTOR_HANDLE DirectXBasics::GetDSVCPUDescriptorHandle(uint32_t index) { return GetCPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index); }

D3D12_GPU_DESCRIPTOR_HANDLE DirectXBasics::GetDSVGPUDescriptorHandle(uint32_t index) { return GetGPUDescriptorHandle(dsvDescriptorHeap, descriptorSizeDSV, index); }

// 深度ステンシルビューの初期化
void DirectXBasics::InitializeDepthStencilView() {
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

	// DSVをデスクリプタヒープの先頭に作る
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(depthBuffer.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

// フェンスの生成
void DirectXBasics::InitializeFence() {
	// フェンス生成
	HRESULT hr;

	// 初期値0でFenceを作る
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	// フェンスのSignalを持つためのイベントを作成する
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
}

// ビューポート矩形の初期化
void DirectXBasics::InitializeViewport() {  
   // ビューポートの設定 
   viewport.TopLeftX = 0.0f;  
   viewport.TopLeftY = 0.0f;  
   viewport.Width = static_cast<float>(WindowsAPI::kClientWidth);  
   viewport.Height = static_cast<float>(WindowsAPI::kClientHeight);  
   viewport.MinDepth = 0.0f;  
   viewport.MaxDepth = 1.0f;  
}

// シザリング矩形の初期化
void DirectXBasics::InitializeScissorRect() {
	// シザリング矩形の設定
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = WindowsAPI::kClientWidth;
	scissorRect.bottom = WindowsAPI::kClientHeight;
}

// DXCコンパイラの生成
void DirectXBasics::InitializeDXCCompiler() {
	HRESULT hr;
	// dxCompilerを初期化
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
}

// ImGuiの初期化
void DirectXBasics::InitializeImGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(windowsAPI->GetHwnd());
	ImGui_ImplDX12_Init(
	    device.Get(), 2, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvDescriptorHeap.Get(), srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
}