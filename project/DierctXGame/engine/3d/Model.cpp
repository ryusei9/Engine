#include "Model.h"
#include "MakeIdentity4x4.h"
#include "TextureManager.h"

//
// Model.cpp
// - OBJ/MTL ファイルを読み込み、頂点・法線・UV をパースして ModelData を構築する。
// - GPU 用 Upload ヒープに頂点バッファとマテリアル用定数バッファを作成・初期化する。
// - 描画時はマテリアル CBV とテクスチャ SRV をルートにセットして DrawInstanced を発行する
//
// 注意点 / 前提
// - 現在の実装は面 (f) を三角形のみサポート（ngons は非対応）
// - OBJ 内のインデックスは「位置/UV/法線」の形式であることを前提としている
// - DirectX の座標系変換（右手系→左手系）や UV 縦反転を行い、エンジン側の期待する形式に合わせている
// - 外部で DirectX, TextureManager の初期化が済んでいること
//

void Model::Initialize(ModelCommon* modelCommon, const std::string& directorypath, const std::string& filename)
{
	// ModelCommon の参照を保存（描画時に DX コマンドリスト等を取得するため）
	modelCommon_ = modelCommon;

	// .obj をパースして ModelData を構築する（頂点配列・MaterialData 等を取得）
	// -> LoadObjFile は v/vt/vn/f/mtllib を処理し、ModelData を返す
	modelData_ = LoadObjFile(directorypath, filename);

	// GPU 用バッファ類を作成して初期化
	CreateVertexData();
	CreateMaterialData();
	
	// OBJ が参照するテクスチャを TextureManager に登録しておく
	TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
	// 登録済みテクスチャのインデックスを ModelData に保存（参照情報として保持）
	modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
}

/// 描画
/// - 頂点バッファをバインドし、マテリアル CBV とテクスチャ SRV をルートに設定して DrawInstanced を発行する
/// - 呼び出し側は既に適切な PSO / RootSignature / デスクリプタヒープが設定されていること
void Model::Draw()
{
	// 頂点バッファビューを設定（頂点配列は Upload ヒープ上に保持されている）
	modelCommon_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);

	// マテリアル用定数バッファ（CBV）をルートに設定（RootParameter の b0 を想定）
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());

	// テクスチャ SRV をルートのデスクリプタテーブルに設定（テクスチャはファイルパスで管理）
	modelCommon_->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureFilePath));

	// DrawInstanced: 頂点数分を描画、インスタンス数は1
	modelCommon_->GetDxCommon()->GetCommandList()->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

/// MTL ファイルを読み込み MaterialData を構築する
/// - 対応: map_Kd を読み取りテクスチャファイルパスを materialData に格納する
/// - directoryPath/filename の組でファイルを開き、1行ずつ解析する
MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	MaterialData materialData;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// map_Kd が見つかったらテクスチャ名を読み取り、ディレクトリと連結してフルパス化する
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}
	return materialData;
}

/// OBJ ファイルをパースして ModelData を構築する
/// - サポートする識別子: v, vt, vn, f, mtllib
/// - 面 (f) の各頂点は "posIndex/uvIndex/normalIndex" の形式を想定
/// - 各要素は1オフセット（OBJ の 1 始まり）なので内部で -1 して参照する
ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	ModelData modelData;

	// 一時的に読み込む要素配列（OBJ 内の生データ）
	std::vector<Vector4> positions; // v: 位置（w を 1 に設定）
	std::vector<Vector3> normals;   // vn: 法線
	std::vector<Vector2> texcoords; // vt: テクスチャ座標

	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	// ファイルを1行ずつ処理
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// 頂点位置: "v x y z"
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			// 右手系 OBJ を左手系レンダラーに合わせるため X 軸を反転
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		// テクスチャ座標: "vt u v"
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			// OBJ の V は上方向が +、DirectX は下方向が + なので反転
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		// 法線: "vn x y z"
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			// 法線の X も位置同様反転して座標系を合わせる
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		// 面: "f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3"
		else if (identifier == "f") {
			// 本実装は三角形のみ対応
			VertexData triangle[3];
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				// "pos/tex/normal" を '/' で分割してインデックスを取得
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					// OBJ は 1 始まりなので -1 して参照する
					elementIndices[element] = std::stoi(index);
				}

				// インデックスから実データを取得（配列アクセスは安全のためファイル側の整合性に依存）
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				triangle[faceVertex] = { position, texcoord, normal };
			}
			// 頂点の登録順を逆にして回り順を調整（必要に応じてここを変更して面の向きを制御）
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		// マテリアルライブラリ参照: "mtllib filename.mtl"
		else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

/// Upload ヒープ上にバッファリソースを確保して返すユーティリティ
/// - device に対して CreateCommittedResource を呼ぶ（Upload ヒープ）
/// - 呼び出し側は必ずサイズと用途を正しく指定すること（安全性の担保は呼び出し元に依存）
Microsoft::WRL::ComPtr<ID3D12Resource> Model::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	(void)dxgiFactory; // 将来的にファクトリを使う場合に備えて取得している（現状は未使用）

	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

/// 頂点バッファを作成して CPU 側の頂点配列を GPU にコピーする
/// - modelData_.vertices に格納された頂点群を Upload ヒープ上に確保したバッファへ memcpy する
void Model::CreateVertexData()
{
	// 必要サイズ分の Upload バッファを作成
	vertexResource_ = CreateBufferResource(modelCommon_->GetDxCommon()->GetDevice(), sizeof(VertexData) * static_cast<UINT>(modelData_.vertices.size()));

	// VertexBufferView を設定（描画時に使用）
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * static_cast<UINT>(modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// バッファをマップして CPU からコピーする（Upload ヒープなので書き込みは直接可能）
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	// 注意: Upload ヒープ上に常駐させる実装。大きなモデルや効率を求める場合は Default ヒープに転送する実装へ変更を検討
}

/// マテリアル用定数バッファの作成と初期化
/// - マテリアル色のデフォルト設定や UV 変換行列の初期化を行う
void Model::CreateMaterialData()
{
	materialResource_ = CreateBufferResource(modelCommon_->GetDxCommon()->GetDevice(), sizeof(Material));

	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	// デフォルトマテリアル値
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	// ライティングはこのスプライト系モデルでは無効にしている（必要なら true に変更）
	materialData_->enableLighting = false;
	// UV トランスフォームは単位行列で初期化
	materialData_->uvTransform = MakeIdentity4x4::MakeIdentity4x4();
}
