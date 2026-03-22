/*
 * StreamingResampler.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2026 brummer <brummer@web.de>
 */

#pragma once

#include <cstdint>
#include <vector>
#include <cmath>
#include <cstring>


/****************************************************************
 StreamingResampler - cubic hermite resampler with nyquist lowpass in down sampler
*****************************************************************/

class StreamingResampler {
public:
    StreamingResampler() = default;

    void setup(uint32_t channels, uint32_t maxBlockSize, uint32_t fs_in, uint32_t fs_out) {
        chan = channels;

        downSample = fs_in > fs_out;
        if (downSample) {
            lpf.resize(chan);
            float cutoff = 0.45f * fs_out;
            for (uint32_t ch = 0; ch < chan; ++ch) {
                lpf[ch].setup((float)fs_in, cutoff);
            }
        }

        ratio = double(fs_in) / double(fs_out);
        bufferSize = maxBlockSize + 8;
        buffer.resize(bufferSize * chan, 0.0f);
        reset();
    }

    void reset() {
        readPos = 0.0;
        buffered = 0;
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        for (auto& f : lpf) f.reset();
    }

    void setSampleRates(uint32_t fs_in, uint32_t fs_out) {
        ratio = double(fs_in) / double(fs_out);
        bool useLPF = (fs_in > fs_out);
        if (useLPF != downSample) setup(chan, bufferSize / chan, fs_in, fs_out);
    }

    uint32_t getOutSize(uint32_t inFrames) const {
        return (uint32_t)std::ceil(inFrames / ratio);
    }

    uint32_t resample(const float* input, float* output, uint32_t inFrames) {
        append(input, inFrames);
        uint32_t outFrames = 0;
        while (true) {
            int ip = (int)readPos;
            if (ip + 2 >= (int)buffered) break;
            float t = float(readPos - ip);

            for (uint32_t ch = 0; ch < chan; ++ch) {
                float x0 = sample(ip - 1, ch);
                float x1 = sample(ip,     ch);
                float x2 = sample(ip + 1, ch);
                float x3 = sample(ip + 2, ch);
                output[outFrames * chan + ch] = hermite(x0, x1, x2, x3, t);
            }
            readPos += ratio;
            ++outFrames;
        }

        int consumed = (int)readPos;

        if (consumed > 0) {
            shiftBuffer(consumed);
            readPos -= consumed;
        }

        return outFrames;
    }

private:
    struct LPF {
        float a0, a1, a2, b1, b2;
        float z1 = 0.0f, z2 = 0.0f;

        void setup(float sampleRate, float cutoff) {
            float K = std::tan(M_PI * cutoff / sampleRate);
            float norm = 1.0f / (1.0f + std::sqrt(2.0f)*K + K*K);
            a0 = K*K * norm;
            a1 = 2.0f * a0;
            a2 = a0;
            b1 = 2.0f * (K*K - 1.0f) * norm;
            b2 = (1.0f - std::sqrt(2.0f)*K + K*K) * norm;
        }

        inline float process(float x) {
            float y = a0*x + z1;
            z1 = a1*x - b1*y + z2;
            z2 = a2*x - b2*y;
            return y;
        }

        void reset() { z1 = z2 = 0.0f; }
    };

    std::vector<LPF> lpf;
    bool downSample = false;
    uint32_t chan = 0;
    double ratio = 1.0;
    std::vector<float> buffer;
    uint32_t bufferSize = 0;
    uint32_t buffered = 0;
    double readPos = 0.0;

    void append(const float* input, uint32_t frames) {
        for (uint32_t i = 0; i < frames; ++i) {
            for (uint32_t ch = 0; ch < chan; ++ch) {
                float s = input[i * chan + ch];
                if (downSample) s = lpf[ch].process(s);
                buffer[(buffered + i) * chan + ch] = s;
            }
        }
        buffered += frames;
    }

    void shiftBuffer(uint32_t frames) {
        uint32_t remaining = buffered - frames;

        std::memmove(buffer.data(), buffer.data() + frames * chan, remaining * chan * sizeof(float));

        buffered = remaining;
    }

    inline float sample(int idx, uint32_t ch) const {
        if (idx < 0) return buffer[ch];

        if ((uint32_t)idx >= buffered)
            return buffer[(buffered - 1) * chan + ch];

        return buffer[idx * chan + ch];
    }

    static inline float hermite(float x0, float x1, float x2, float x3, float t) {
        float c0 = x1;
        float c1 = 0.5f * (x2 - x0);
        float c2 = x0 - 2.5f * x1 + 2.0f * x2 - 0.5f * x3;
        float c3 = 0.5f * (x3 - x0) + 1.5f * (x1 - x2);
        return ((c3*t + c2)*t + c1)*t + c0;
    }
};
