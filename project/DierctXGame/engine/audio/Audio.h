#pragma once
#include <wrl.h>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")
#include <fstream>
#include <memory>

// チャンクヘッダ
struct ChunkHeader {
	// チャンクのID
	char id[4];
	// チャンクのサイズ
	int32_t size;
};

// RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk;
	char type[4];
};

// FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};
class Audio
{
public:
	Audio() = default;
	~Audio() = default;
	// コピーコンストラクタ
	Audio(const Audio&) = delete;
	// コピー代入演算子
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

	static std::shared_ptr<Audio> instance;

	

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;

	IXAudio2MasteringVoice* masterVoice = nullptr;

	HRESULT result;
};

