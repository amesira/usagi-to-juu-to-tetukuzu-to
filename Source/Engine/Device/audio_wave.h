#pragma once
#include <Windows.h>
#include <mmreg.h>
#include <cstdint>
#include <cstring>
#include <vector>
namespace AudioWave {
struct Data { WAVEFORMATEX format{}; std::vector<BYTE> samples; };
inline bool Parse(const std::vector<BYTE>& bytes, Data& destination) {
    auto u16 = [&bytes](size_t p) { return uint16_t(bytes[p]) | uint16_t(bytes[p + 1]) << 8; };
    auto u32 = [&bytes](size_t p) { return uint32_t(bytes[p]) | uint32_t(bytes[p + 1]) << 8 | uint32_t(bytes[p + 2]) << 16 | uint32_t(bytes[p + 3]) << 24; };
    if (bytes.size() < 12 || std::memcmp(bytes.data(), "RIFF", 4) || std::memcmp(bytes.data() + 8, "WAVE", 4)) return false;
    const uint64_t end = uint64_t(u32(4)) + 8;
    if (end < 12 || end > bytes.size()) return false;
    Data loaded;
    bool hasFormat = false, hasSamples = false;
    for (size_t p = 12; p + 8 <= end;) {
        const size_t length = u32(p + 4), begin = p + 8;
        if (length > end - begin) return false;
        if (!std::memcmp(bytes.data() + p, "fmt ", 4)) {
            if (length < 16 || hasFormat) return false;
            auto& f = loaded.format;
            f.wFormatTag = u16(begin); f.nChannels = u16(begin + 2);
            f.nSamplesPerSec = u32(begin + 4); f.nAvgBytesPerSec = u32(begin + 8);
            f.nBlockAlign = u16(begin + 12); f.wBitsPerSample = u16(begin + 14);
            if (f.wFormatTag != WAVE_FORMAT_PCM || f.nChannels < 1 || f.nChannels > 8
                || f.nSamplesPerSec < 1000 || f.nSamplesPerSec > 200000
                || (f.wBitsPerSample != 8 && f.wBitsPerSample != 16 && f.wBitsPerSample != 24 && f.wBitsPerSample != 32)
                || f.nBlockAlign != f.nChannels * (f.wBitsPerSample / 8)
                || f.nAvgBytesPerSec != f.nSamplesPerSec * f.nBlockAlign) return false;
            hasFormat = true;
        }
        else if (!std::memcmp(bytes.data() + p, "data", 4)) {
            if (!length || hasSamples) return false;
            loaded.samples.assign(bytes.begin() + begin, bytes.begin() + begin + length);
            hasSamples = true;
        }
        p = begin + length + (length & 1);
        if (p > end) return false;
    }
    if (!hasFormat || !hasSamples || loaded.samples.size() % loaded.format.nBlockAlign) return false;
    destination = std::move(loaded); return true;
}
}
