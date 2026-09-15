#pragma once
#include <cstdint>
using AudioLoopHandle = std::uint64_t;
inline constexpr AudioLoopHandle InvalidAudioLoopHandle = 0;
// PCM WAV data is retained until UnloadAudio. Invalid IDs are ignored safely.
void InitAudio();
void UninitAudio();
bool IsAudioInitialized();
int LoadAudio(const wchar_t* fileName);
void UnloadAudio(int index);
void PlayAudio(int index, bool loop = false); // Restart playback; intended for BGM.
bool PlayAudioOneShot(int index, float volume = 1.0f); // Up to 16 overlapping voices per sound.
AudioLoopHandle StartAudioLoop(int index, float volume = 1.0f);
void StopAudioLoop(AudioLoopHandle handle);
bool IsAudioLoopPlaying(AudioLoopHandle handle);
void StopAudio(int index);
void SetAudioVolume(int index, float volume);
void SetMasterAudioVolume(float volume);
bool IsAudioPlaying(int index);
