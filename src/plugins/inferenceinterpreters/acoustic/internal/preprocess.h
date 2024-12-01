#ifndef PREPROCESS_H
#define PREPROCESS_H

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer::dsinterp {
    struct Segment;
    class SpeakerEmbed;
    class SpeakerMixCurve;

    JsonValue parsePhonemeTokens(const Segment &dsSegment, const std::unordered_map<std::string, int64_t> &name2token);

    JsonValue parsePhonemeLanguages(const Segment &dsSegment, const std::unordered_map<std::string, int64_t> &languages);

    JsonValue parsePhonemeDurations(const Segment &dsSegment, double frameLength, int64_t &outTargetLength);

    std::vector<float> parseF0AsVector(const Segment &dsSegment, double frameLength, int64_t targetLength, double a4freq = 440.0);

    JsonValue parseF0(const std::vector<float> &f0);

    JsonValue parseF0(const Segment &dsSegment, double frameLength, int64_t targetLength, double a4freq = 440.0);

    JsonValue parseVarianceParameter(const Segment &dsSegment, const char *parameter, const char *modelParameter,
                                     double frameLength, int64_t targetLength);

    JsonValue parseTransitionParameter(const Segment &dsSegment, const char *parameter, const char *modelParameter,
                                       float defaultValue,
                                       double frameLength, int64_t targetLength);

    JsonValue parseSpeakerMix(const SpeakerEmbed &spkEmb, const std::vector<std::string> &speakers,
                              const SpeakerMixCurve &spkMix, double frameLength, int64_t targetLength);

    bool readObjectHelper(const JsonObject &object, const std::string &type, std::unordered_map<std::string, int64_t> &out, Error *error);

    bool readJsonFileHelper(const std::filesystem::path &path, const std::string &type, std::unordered_map<std::string, int64_t> &out, Error *error);

    template <typename T>
    inline JsonValue create_tensor(const char *name,
                                   const T *data_buffer, size_t data_size,
                                   const int64_t *shape_buffer, size_t shape_size) {
        const uint8_t *bytes_buffer = reinterpret_cast<const uint8_t *>(data_buffer);
        size_t bytes_size = data_size * sizeof(T);
        const char *type;
        if constexpr (std::is_same_v<T, float>) {
            type = "float";
        } else if constexpr (std::is_same_v<T, int64_t>) {
            type = "int64";
        } else if constexpr (std::is_same_v<T, bool>) {
            type = "bool";
        } else {
            static_assert(!std::is_same_v<T, T>, "Unexpected type in create_tensor.");
        }

        JsonArray shape;
        shape.reserve(shape_size);
        for (size_t i = 0; i < shape_size; ++i) {
            shape.emplace_back(shape_buffer[i]);
        }
        return JsonObject{
                {"name", name},
                {"format", "bytes"},
                {
                    "data", JsonObject{
                        {"type", type},
                        {"shape", shape},
                        {"value", std::vector<uint8_t>(bytes_buffer, bytes_buffer + bytes_size)},
                    }
                }
        };
    }

    template <typename T>
    inline JsonValue create_tensor_from_scalar(const char *name, T value) {
        const std::array<T, 1> data{value};
        constexpr std::array<int64_t, 1> shape{1};
        return create_tensor<T>(name, data.data(), data.size(), shape.data(), shape.size());
    }

    constexpr int64_t getSpeedupFromSteps(int64_t steps) {
        int64_t speedup = 10;
        if (steps > 0) {
            speedup = 1000 / steps;
            if (speedup < 1) {
                speedup = 1;
            }
            else if (speedup > 1000) {
                speedup = 1000;
            }
            while (((1000 % speedup) != 0) && (speedup > 1)) {
                --speedup;
            }
        }
        return speedup;
    }

    inline int64_t getIntDepth(float depth, int64_t maxDepth, int64_t speedup) {
        int64_t dsDepthInt64 = std::llround(depth * 1000);
        dsDepthInt64 = (std::min)(dsDepthInt64, maxDepth);

        // make sure depth can be divided by speedup
        return dsDepthInt64 / speedup * speedup;
    }
}

#endif // PREPROCESS_H