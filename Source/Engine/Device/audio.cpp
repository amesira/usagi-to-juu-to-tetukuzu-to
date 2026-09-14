#include "audio.h"
#include "audio_wave.h"
#include <xaudio2.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "ole32.lib")
namespace {
constexpr size_t MaxSounds = 100, MaxVoices = 16;
struct Sound {
    AudioWave::Data data;
    std::vector<IXAudio2SourceVoice*> voices;
    size_t nextVoice = 0;
    bool loaded = false;
};
std::array<Sound, MaxSounds> sounds;
IXAudio2* engine = nullptr;
IXAudio2MasteringVoice* output = nullptr;
bool ownsCom = false;
float Volume(float value) { return std::isfinite(value) ? std::clamp(value, 0.0f, 1.0f) : 0.0f; }
Sound* Get(int index) { return index >= 0 && index < int(MaxSounds) && sounds[index].loaded ? &sounds[index] : nullptr; }
IXAudio2SourceVoice* CreateVoice(Sound& sound) {
    IXAudio2SourceVoice* voice = nullptr;
    if (!engine || FAILED(engine->CreateSourceVoice(&voice, &sound.data.format))) return nullptr;
    sound.voices.push_back(voice); return voice;
}
bool Playing(IXAudio2SourceVoice* voice) {
    XAUDIO2_VOICE_STATE state{}; voice->GetState(&state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
    return state.BuffersQueued != 0;
}
bool Submit(Sound& sound, IXAudio2SourceVoice* voice, bool loop, float volume) {
    if (!voice) return false;
    voice->Stop(); voice->FlushSourceBuffers();
    voice->SetVolume(Volume(volume));
    XAUDIO2_BUFFER buffer{};
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = static_cast<UINT32>(sound.data.samples.size());
    buffer.pAudioData = sound.data.samples.data();
    if (loop) buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
    if (FAILED(voice->SubmitSourceBuffer(&buffer))) return false;
    if (FAILED(voice->Start())) { voice->FlushSourceBuffers(); return false; }
    return true;
}
}
void InitAudio() {
    if (engine) return;
    const HRESULT com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    ownsCom = SUCCEEDED(com);
    if (FAILED(com) && com != RPC_E_CHANGED_MODE) return;
    if (FAILED(XAudio2Create(&engine)) || FAILED(engine->CreateMasteringVoice(&output))) {
        UninitAudio(); OutputDebugStringW(L"Audio initialization failed.\n");
    }
}
void UninitAudio() {
    for (int i = 0; i < int(MaxSounds); ++i) UnloadAudio(i);
    if (output) { output->DestroyVoice(); output = nullptr; }
    if (engine) { engine->Release(); engine = nullptr; }
    if (ownsCom) { CoUninitialize(); ownsCom = false; }
}
bool IsAudioInitialized() { return engine && output; }
int LoadAudio(const wchar_t* path) {
    if (!IsAudioInitialized() || !path || !*path) return -1;
    auto it = std::find_if(sounds.begin(), sounds.end(), [](const Sound& s) { return !s.loaded; });
    if (it == sounds.end()) return -1;
    std::ifstream file(std::filesystem::path(path), std::ios::binary | std::ios::ate);
    if (!file) return -1;
    const auto size = file.tellg();
    if (size < 12 || size > 256 * 1024 * 1024) return -1;
    std::vector<BYTE> bytes(static_cast<size_t>(size));
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()))) return -1;
    AudioWave::Data data;
    if (!AudioWave::Parse(bytes, data)) return -1;
    it->data = std::move(data);
    if (!CreateVoice(*it)) { *it = Sound{}; return -1; }
    it->loaded = true;
    return static_cast<int>(it - sounds.begin());
}
void UnloadAudio(int index) {
    auto* sound = Get(index); if (!sound) return;
    for (auto* voice : sound->voices) voice->DestroyVoice();
    *sound = Sound{};
}
void StopAudio(int index) {
    auto* sound = Get(index); if (!sound) return;
    for (auto* voice : sound->voices) { voice->Stop(); voice->FlushSourceBuffers(); }
}
void PlayAudio(int index, bool loop) {
    auto* sound = Get(index); if (!sound) return;
    StopAudio(index);
    // Preserve the caller's BGM volume when restarting.
    float volume = 1; sound->voices.front()->GetVolume(&volume);
    Submit(*sound, sound->voices.front(), loop, volume);
}
bool PlayAudioOneShot(int index, float volume) {
    auto* sound = Get(index); if (!sound) return false;
    auto idle = std::find_if(sound->voices.begin(), sound->voices.end(), [](auto* v) { return !Playing(v); });
    IXAudio2SourceVoice* voice = idle != sound->voices.end() ? *idle : nullptr;
    if (!voice && sound->voices.size() < MaxVoices) voice = CreateVoice(*sound);
    // At the cap, recycle voices in order rather than allocating without a limit.
    if (!voice && !sound->voices.empty()) {
        voice = sound->voices[sound->nextVoice++ % sound->voices.size()];
    }
    return Submit(*sound, voice, false, volume);
}
void SetAudioVolume(int index, float volume) {
    if (auto* sound = Get(index)) for (auto* voice : sound->voices) voice->SetVolume(Volume(volume));
}
void SetMasterAudioVolume(float volume) { if (output) output->SetVolume(Volume(volume)); }
bool IsAudioPlaying(int index) {
    if (auto* sound = Get(index)) return std::any_of(sound->voices.begin(), sound->voices.end(), Playing);
    return false;
}
