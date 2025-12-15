#pragma once
#include <windows.h>
#include <d3d12.h>
#include <d3d12shader.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WindowsAPI.h"
#include <array>
#include <dxcapi.h>
#include <string>
#include "externals/DirectXTex/Directxtex.h"
#include "externals/DirectXTex/d3dx12.h"

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

class DirectXCommon {
public:
	// 初期化
	void Initialize(WindowsAPI* windowsAPI);

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

	// 描画前処理
	void PreDraw();

	// 描画後処理
	void PostDraw();

	// getter
	ID3D12Device* GetDevice() const { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }


	// シェーダーコンパイル
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);

	// バッファリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	// テクスチャリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	// テクスチャデータの転送
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(const Microsoft::WRL::ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImages);

	// テクスチャファイルの読み込み
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);
private:

	// DirectX12のデバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	// DXGIファクトリ
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

	// WindowsAPI
	WindowsAPI* directXWindowsAPI_ = nullptr;

	// コマンドキュー
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;
	// コマンドアロケータ
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
	// コマンドリスト
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;
	// スワップチェーンのメンバ変数
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
	// 深度バッファのリソースを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_;


	// レンダーターゲットビュー用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
	// シェーダリソースビュー用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	// 深度ステンシルビュー用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;

	// SwapCainからResourceを引っ張ってくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResources_[2];
	// RTVハンドル
	//D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	

	// デスクリプタサイズ
	uint32_t descriptorSizeSRV_ = 0;
	uint32_t descriptorSizeRTV_ = 0;
	uint32_t descriptorSizeDSV_ = 0;

	// スワップチェーンリソース
	//std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	// ビューポート矩形のメンバ変数
	D3D12_VIEWPORT viewport_{};

	// シザリング矩形のメンバ変数
	D3D12_RECT scissorRect_{};

	// DXCコンパイラの各生成物のメンバ変数
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr; // DXCユーティリティ
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr; // DXCコンパイラ
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr; // DXCインクルードハンドラ

	// フェンスのメンバ変数
	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = nullptr;


};
