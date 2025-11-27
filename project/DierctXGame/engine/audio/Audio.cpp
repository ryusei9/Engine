#include "Audio.h"
#include <cassert>

//
// Audio: XAudio2 を使った簡易オーディオユーティリティ
// - シングルトンで管理され、アプリケーション全体で一意の XAudio2 インスタンスを保持する。
// - WAV ファイル（RIFF/WAVE）を読み込んでメモリに展開し、XAudio2 の SourceVoice で再生する。
// - 注意:
//   - SoundLoadWave は読み込んだ PCM データ用にヒープ上でバッファを確保する（new[]）。呼び出し側は
//     SoundUnload を呼んでメモリを解放する必要がある。
//   - 実装は簡易的な WAV パーサであり、すべての WAV 形式を扱えるわけではない（基本的な PCM のみ想定）。
//   - スレッドセーフではない（呼び出しはメインスレッド前提）。
//

std::shared_ptr<Audio> Audio::instance = nullptr;

void Audio::Initialize()
{
	// XAudio2 を初期化して MasteringVoice を作成する
	// 副作用: xAudio2 と masterVoice をメンバに保存する
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	// マスターボイスの作成（出力デバイスとの接続）
	result = xAudio2->CreateMasteringVoice(&masterVoice);
}

SoundData Audio::SoundLoadWave(const char* filename)
{
	// WAV ファイルを読み込み、SoundData を返す
	// - filename: 読み込む WAV ファイルのパス（C 文字列）
	// - 戻り値: 読み込まれた PCM データとフォーマットを含む SoundData（pBuffer は new[] により確保）
	// - エラー処理: ファイル形式が不正なら assert で停止する（デバッグビルド向け）
	//
	// 実装メモ:
	// - RIFF / WAVE / fmt / data チャンクのみを順次処理する
	// - "JUNK" チャンクを見つけた場合はスキップする実装を含む
	// - 読み込んだバッファは呼び出し元で SoundUnload() を呼び解放すること

	// ファイルオープン
	std::ifstream file;
	file.open(filename, std::ios::binary);
	assert(file.is_open());

	// RIFF ヘッダ読み取り
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));

	// チャンクID が "RIFF" であることを確認
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(false);
	}
	// タイプが "WAVE" であることを確認
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(false);
	}

	// fmt チャンクの読み込み
	FormatChunk format{};
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(false);
	}
	// fmt 本体を読み込む（可変長なので chunk.size を使う）
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// data チャンクの読み込み（JUNK をスキップする場合あり）
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// "JUNK" チャンクを検出したらスキップして次のチャンクを読む
	if (strncmp(data.id, "JUNK", 4) == 0) {
		file.seekg(data.size, std::ios::cur);
		file.read((char*)&data, sizeof(data));
	}
	// data チャンクであることを確認
	if (strncmp(data.id, "data", 4) != 0) {
		assert(false);
	}
	// 波形データ本体を読み込む
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// ファイルを閉じる
	file.close();

	// SoundData を構築して返す
	SoundData soundData = {};
	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

void Audio::SoundUnload(SoundData* soundData)
{
	// SoundData が保持するバッファを解放し、構造体を初期化する
	// - 呼び出し側が SoundLoadWave で確保したメモリを確実に解放すること
	delete[] soundData->pBuffer;
	soundData->pBuffer = nullptr;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

void Audio::SoundPlayWave(const SoundData& soundData)
{
	// SoundData のフォーマット情報を使って SourceVoice を作成し再生する
	// - 再生完了後のリソース破棄（SourceVoice の DestroyVoice）は現在行っていないため、
	//   長時間または多数のサウンドを再生する用途では追加の管理が必要
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// XAUDIO2_BUFFER に再生データを詰める
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 再生キューに登録して再生開始
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

std::shared_ptr<Audio> Audio::GetInstance()
{
	// シングルトン取得（遅延初期化）
	if (instance == nullptr) {
		instance = std::make_shared<Audio>();
	}
	return instance;
}

void Audio::Finalize()
{
	// 終了処理: マスターボイスの破棄と XAudio2 オブジェクトの解放
	if (masterVoice) {
		masterVoice->DestroyVoice();
		masterVoice = nullptr;
	}
	if (xAudio2) {
		xAudio2.Reset();
	}
}
