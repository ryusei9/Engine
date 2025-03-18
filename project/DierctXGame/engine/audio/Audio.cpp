#include "Audio.h"
#include <cassert>


std::shared_ptr<Audio> Audio::instance = nullptr;
void Audio::Initialize()
{
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	// マスターボイスの作成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
}

SoundData Audio::SoundLoadWave(const char* filename)
{
	// ファイルオープン
	std::ifstream file;
	// ファイル入力ストリームのインスタンス
	file.open(filename, std::ios::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());

	// .wavデータ読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	// チャンクIDがRIFFであることを確認
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		// ファイルがRIFF形式でない場合はエラー
		assert(false);
	}
	// タイプがWAVEであることを確認
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		// ファイルがWAVE形式でない場合はエラー
		assert(false);
	}

	// Formatチャンクの読み込み
	FormatChunk format{};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		// ファイルがfmt形式でない場合はエラー
		assert(false);
	}
	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	//Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {

		// データチャンクをスキップ
		file.seekg(data.size, std::ios::cur);
		// 次のチャンクを読み込む
		file.read((char*)&data, sizeof(data));
	}
	// データチャンクがdataであることを確認
	if (strncmp(data.id, "data", 4) != 0) {
		// ファイルがdata形式でない場合はエラー
		assert(false);
	}
	// データチャンクのデータ部(波形データ)を読み込む
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる
	file.close();

	// 読み込んだ音声データをreturn
	// returnするための構造体を作成
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

void Audio::SoundUnload(SoundData* soundData)
{
	// バッファの解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void Audio::SoundPlayWave(const SoundData& soundData)
{
	// 波形フォーマットを元にSourceVoiceを作成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

std::shared_ptr<Audio> Audio::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_shared<Audio>();
	}
	return instance;
}

void Audio::Finalize()
{
	if (masterVoice) {
		masterVoice->DestroyVoice();
		masterVoice = nullptr;
	}
	if (xAudio2) {
		xAudio2.Reset();
	}
}
