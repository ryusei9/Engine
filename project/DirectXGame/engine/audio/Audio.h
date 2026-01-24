#pragma once
#include <wrl.h>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <fstream>
#include <memory>
#include <cstdint>

// Audio用の定数
namespace AudioConstants {
	// チャンクIDのサイズ
	constexpr size_t kChunkIdSize = 4;
	
	// チャンクID文字列
	constexpr const char* kRiffChunkId = "RIFF";
	constexpr const char* kWaveType = "WAVE";
	constexpr const char* kFmtChunkId = "fmt ";
	constexpr const char* kDataChunkId = "data";
	constexpr const char* kJunkChunkId = "JUNK";
	
	// XAudio2のデフォルト値
	constexpr uint32_t kXAudio2Flags = 0;
	constexpr XAUDIO2_PROCESSOR kDefaultProcessor = XAUDIO2_DEFAULT_PROCESSOR;
}

// チャンクヘッダ
struct ChunkHeader {
	char id[AudioConstants::kChunkIdSize]; // チャンクのID
	int32_t size; // チャンクのサイズ
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk;
	char type[AudioConstants::kChunkIdSize];
};

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

// サウンドデータ
struct SoundData {
	WAVEFORMATEX wfex; // 波形フォーマット
	BYTE* pBuffer; // バッファの先頭アドレス
	uint32_t bufferSize; // バッファのサイズ
};

/// <summary>
/// オーディオ管理クラス
/// </summary>
class Audio
{
public:
	// コンストラクタ・デストラクタ
	Audio() = default;
	~Audio() = default;
	
	// コピー禁止
	Audio(const Audio&) = delete;
	Audio& operator=(const Audio&) = delete;

	// シングルトンインスタンスの取得
	static std::shared_ptr<Audio> GetInstance();

	// 初期化
	void Initialize();

	// サウンドデータを読み込む
	SoundData SoundLoadWave(const char* filename);

	// 音声データ解放
	void SoundUnload(SoundData* soundData);

	// 音声再生
	void SoundPlayWave(const SoundData& soundData);

	// 終了
	void Finalize();

private:
	// WAVファイル読み込みヘルパー関数
	void ReadRiffHeader(std::ifstream& file, RiffHeader& riff);
	void ReadFormatChunk(std::ifstream& file, FormatChunk& format);
	void ReadDataChunk(std::ifstream& file, ChunkHeader& data);
	char* ReadWaveData(std::ifstream& file, uint32_t size);
	
	// チャンク検証
	bool ValidateChunkId(const char* chunkId, const char* expectedId) const;
	void SkipJunkChunk(std::ifstream& file, ChunkHeader& data);
	
	// XAudio2初期化
	void InitializeXAudio2();
	void CreateMasteringVoice();
	
	// SourceVoice作成と再生
	IXAudio2SourceVoice* CreateSourceVoice(const WAVEFORMATEX& wfex);
	void PlaySourceVoice(IXAudio2SourceVoice* sourceVoice, const SoundData& soundData);

	// シングルトンインスタンス
	static std::shared_ptr<Audio> sInstance_;

	// XAudio2関連
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;

	// エラーコード
	HRESULT result_;
};

