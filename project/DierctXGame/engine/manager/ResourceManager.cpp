#include "ResourceManager.h"
#include <cassert>
#include <Camera.h>
#include <Inverse.h>

//
// ResourceManager
// - Direct3D12 の Upload ヒープ上にサイズ指定のバッファリソースを作成して返すユーティリティ関数を提供する。
// - 主に頂点 / インデックス / 定数バッファなど、CPU から直接書き込む用途のリソースを簡易に確保するために利用する。
//
// 注意事項（設計メモ）:
// - 本関数は Upload ヒープ（D3D12_HEAP_TYPE_UPLOAD）でリソースを作成する。
//   Upload ヒープは CPU からの書き込みが容易だが、GPU 側の読み取り性能は Default ヒープに比べ劣る。
//   大きな静的リソースや頻繁に参照される大容量データは Default ヒープを作成し、Upload バッファ経由で転送する実装を推奨する。
// - sizeInBytes はバイト単位のリソースサイズ。呼び出し側で適切に計算すること（必要ならアライメント調整を行う）。
// - 生成に失敗した場合は assert で止める実装になっている（リリースでの堅牢性が必要ならエラー処理を拡張する）。
//
Microsoft::WRL::ComPtr<ID3D12Resource> ResourceManager::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes)
{
	// DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// 関数が成功したかどうか
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};

	// UploadHeapを使う
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};

	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	// リソースのサイズ
	resourceDesc.Width = sizeInBytes;

	// バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;

	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;



	// 実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

