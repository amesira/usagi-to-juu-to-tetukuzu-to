#include "Engine/Device/audio.h"
#include "Engine/Device/audio_wave.h"
#include "Game/ControllerBehavior/Audio/audio_fade.h"
#include "Game/ControllerBehavior/Audio/game_audio_settings_asset.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
namespace {
void U16(std::vector<BYTE>& v, uint16_t n) { v.push_back(BYTE(n)); v.push_back(BYTE(n >> 8)); }
void U32(std::vector<BYTE>& v, uint32_t n) { for (int i = 0; i < 4; ++i) v.push_back(BYTE(n >> (i * 8))); }
void Tag(std::vector<BYTE>& v, const char* text) { v.insert(v.end(), text, text + 4); }
void Size(std::vector<BYTE>& v) { const auto n = uint32_t(v.size() - 8); for (int i = 0; i < 4; ++i) v[4 + i] = BYTE(n >> (8 * i)); }
std::vector<BYTE> Wav() {
    std::vector<BYTE> v; Tag(v, "RIFF"); U32(v, 0); Tag(v, "WAVE");
    Tag(v, "JUNK"); U32(v, 1); v.push_back(7); v.push_back(0); // Unknown odd-sized chunk and padding.
    Tag(v, "fmt "); U32(v, 16); U16(v, 1); U16(v, 1); U32(v, 44100); U32(v, 88200); U16(v, 2); U16(v, 16);
    Tag(v, "data"); U32(v, 4); U16(v, 0); U16(v, 123); Size(v); return v;
}
}
int main() {
    const auto wav = Wav(); AudioWave::Data loaded;
    assert(AudioWave::Parse(wav, loaded));
    assert(loaded.format.nSamplesPerSec == 44100 && loaded.format.nBlockAlign == 2 && loaded.samples.size() == 4);
    const auto savedSamples = loaded.samples;
    for (size_t size = 0; size < wav.size(); ++size) {
        const std::vector<BYTE> truncated(wav.begin(), wav.begin() + size);
        assert(!AudioWave::Parse(truncated, loaded));
        assert(loaded.samples == savedSamples);
    }
    auto bad = wav; bad[30] = 3; assert(!AudioWave::Parse(bad, loaded)); // Unsupported encoding.
    bad = wav; bad[42] = 0; assert(!AudioWave::Parse(bad, loaded)); // Invalid block alignment.
    bad = wav; bad[46] = 'x'; assert(!AudioWave::Parse(bad, loaded)); // Missing data chunk.
    bad = wav; bad[50] = 255; assert(!AudioWave::Parse(bad, loaded)); // Oversized chunk.
    AudioFade fade; fade.Begin(1, 1); fade.Update(.25f); assert(fade.value == .25f);
    fade.Begin(0, .5f); fade.Update(.25f); assert(fade.value == .125f);
    fade.Update(1); assert(fade.value == 0 && fade.Finished());
    fade.Begin(1, 0); assert(fade.value == 1 && fade.Finished());
    fade.Update(std::numeric_limits<float>::quiet_NaN()); assert(fade.value == 1);
    GameAudioSettings::Data settings;
    std::ifstream file("asset/Data/game_audio_settings.data.json"); assert(file.good());
    auto json = nlohmann::json::parse(file);
    assert(json["type"] == "GameAudioSettingsAsset");
    assert(FieldSerialization::DeserializeFields(json["data"], settings, GameAudioSettings::GetSchema()));
    settings.masterVolume = 10; settings.seVolume = -1; settings.fadeDuration = std::numeric_limits<float>::quiet_NaN();
    GameAudioSettings::Sanitize(settings);
    assert(settings.masterVolume == 1 && settings.seVolume == 0 && settings.fadeDuration == .5f);
    // Calls before initialization and after shutdown must be harmless.
    UninitAudio(); assert(!IsAudioInitialized()); assert(LoadAudio(L"missing.wav") == -1);
    for (int id : {-1, 0, 99, 100}) {
        PlayAudio(id, true); StopAudio(id); SetAudioVolume(id, 1); UnloadAudio(id);
        assert(!PlayAudioOneShot(id) && !IsAudioPlaying(id));
    }
    assert(StartAudioLoop(-1) == InvalidAudioLoopHandle);
    assert(StartAudioLoop(0) == InvalidAudioLoopHandle);
    StopAudioLoop(InvalidAudioLoopHandle);
    StopAudioLoop(12345);
    StopAudioLoop(12345);
    assert(!IsAudioLoopPlaying(12345));
    UninitAudio(); SetMasterAudioVolume(1);
    std::cout << "audio_tests passed\n";
}
