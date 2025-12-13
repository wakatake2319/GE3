#pragma once
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WindowsAPI.h"
#include <array>
#include <dxcapi.h>



class DirectXBasics {
public:
	// 初期化
	void Initialize();

	// デバイスの初期化
	void InitializeDevice();

	// コマンド関連の初期化
	void InitializeCommand();

	// スワップチェーンの生成
	void InitializeSwapChain();

	// 深度バッファの生成
	void InitializeDepthBuffer();

	// 各種でスクリプタヒープの生成
	void InitializeDescriptorHeaps();

	// デスクリプタヒープ生成関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors, bool shaderVisible);

	// レンダーターゲットビューの初期化
	void InitializeRenderTargetViews();

	// CPUデスクリプタハンドル取得関数
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr< ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// GPUデスクリプタハンドル取得
	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	// SRVに特化した公開用の関数
	// SRVの指定番号のCPUデスクリプタハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	// SRVの指定番号のGPUデスクリプタハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	// RTVに特化した公開用の関数
	// RTVの指定番号のCPUデスクリプタハンドルを取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	
	// RTVの指定番号のRTVデスクリプタハンドル取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);

	// DSVに特化した公開用の関数
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);

	// DSVの指定番号のDSVデスクリプタハンドル取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

	// 深度ステンシルビューの初期化
	void InitializeDepthStencilView();

	// フェンスの生成
	void InitializeFence();

	// ビューポート矩形の初期化
	void InitializeViewport();

	// シザリング矩形の初期化
	void InitializeScissorRect();

	// DXCコンパイラの生成
	void InitializeDXCCompiler();

	// ImGuiの初期化
	void InitializeImGui();

private:
	// 初期化
	void Initialize(WindowsAPI* windowsAPI);


	// DirectX12のデバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	// WindowsAPI
	WindowsAPI* windowsAPI = nullptr;

	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	// 深度バッファのリソースを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer;


	// レンダーターゲットビュー用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	// シェーダリソースビュー用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	// 深度ステンシルビュー用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	// SwapCainからResourceを引っ張ってくる
	//Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources[2] = {nullptr};
	// RTVハンドル
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	

	// デスクリプタサイズ
	uint32_t descriptorSizeSRV = 0;
	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;

	// スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	// ビューポート矩形のメンバ変数
	D3D12_VIEWPORT viewport{};

	// シザリング矩形のメンバ変数
	D3D12_RECT scissorRect{};

	// DXCコンパイラの各生成物のメンバ変数
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	
};
