#include "preprocess.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <numeric>

#include <stdcorelib/path.h>

#include "project.h"
#include "speaker_embed.h"

namespace dsinfer::dsinterp {
    static std::vector<float> getSpkMix(const SpeakerEmbed &spkEmb, const std::vector<std::string> &speakers,
                                        const SpeakerMixCurve &spkMix, double frameLength, int64_t targetLength) {
        // Required to choose a speaker.
        std::vector<float> spk_embed;
        int64_t spkEmbedArraySize = targetLength * SPK_EMBED_SIZE;
        spk_embed.resize(spkEmbedArraySize);
        if (spkMix.empty()) {
            // Use the first one by default.
            auto emb = spkEmb.getMixedEmb({{speakers[0], 1.0}});
            for (size_t i = 0; i < spkEmbedArraySize; ++i) {
                spk_embed[i] = emb[i % SPK_EMBED_SIZE];
            }
        } else {
            auto spkMixResampled = spkMix.resample(frameLength, targetLength);
            for (int64_t i = 0; i < targetLength; ++i) {
                std::unordered_map<std::string, double> mix;
                double mixSum = std::accumulate(spkMixResampled.spk.begin(), spkMixResampled.spk.end(), 0.0,
                                                [i](double value, const auto &speakerItem) {
                                                    return value + speakerItem.second.samples[i];
                                                });
                if (mixSum == 0) {
                    mixSum = 1;
                }
                int64_t speakerIndex = 0;
                for (const auto &speakerItem: spkMixResampled.spk) {
                    // If SampleCurve::resample guarantees the size of returned array is at least `targetLength`,
                    // subscripting will not go out of range here.
                    mix[speakerItem.first] = speakerItem.second.samples[i] / mixSum;
                    ++speakerIndex;
                }
                auto emb = spkEmb.getMixedEmb(mix);
                int64_t y = i * SPK_EMBED_SIZE;
                for (int64_t j = 0; j < SPK_EMBED_SIZE; ++j) {
                    spk_embed[y + j] = emb[j];
                }
            }
        }
        return spk_embed;
    }

    JsonValue parsePhonemeTokens(const Segment &dsSegment, const std::unordered_map<std::string, int64_t> &name2token) {
        std::vector<int64_t> tokens;
        tokens.reserve(dsSegment.phoneCount());
        for (const auto &word: dsSegment.words) {
            for (const auto &phone: word.phones) {
                // tokens
                std::string tokenWithLang =
                        (phone.language.empty() || phone.token == "SP" || phone.token == "AP")
                            ? phone.token
                            : (phone.language + '/' + phone.token);
                if (const auto it = name2token.find(tokenWithLang); it != name2token.end()) {
                    // first try finding the phoneme with the language tag (lang/phoneme)
                    tokens.push_back(it->second);
                } else if (const auto it2 = name2token.find(phone.token); it2 != name2token.end()) {
                    // then try finding the phoneme without the language tag (phoneme)
                    tokens.push_back(it2->second);
                } else {
                    // TODO: error handling
                    tokens.push_back(0);
                }
            }
        }
        const std::array<int64_t, 2> shape{1, static_cast<int64_t>(tokens.size())};
        return create_tensor<int64_t>("tokens", tokens.data(), tokens.size(), shape.data(), shape.size());
    }


    JsonValue parsePhonemeLanguages(const Segment &dsSegment, const std::unordered_map<std::string, int64_t> &languages) {
        std::vector<int64_t> lang;
        lang.reserve(dsSegment.phoneCount());
        for (const auto &word: dsSegment.words) {
            for (const auto &phone: word.phones) {
                // tokens
                if (const auto it = languages.find(phone.language); it != languages.end()) {
                    lang.push_back(it->second);
                } else {
                    // TODO: error handling
                    lang.push_back(0);
                }
            }
        }
        const std::array<int64_t, 2> shape{1, static_cast<int64_t>(lang.size())};
        return create_tensor<int64_t>("languages", lang.data(), lang.size(), shape.data(), shape.size());
    }

    JsonValue parsePhonemeDurations(const Segment &dsSegment, double frameLength, int64_t &outTargetLength) {
        auto phoneCount = dsSegment.phoneCount();

        std::vector<int64_t> durations;
        durations.reserve(phoneCount);

        double phoneDurSum = 0.0;

        for (size_t currWordIndex = 0; currWordIndex < dsSegment.words.size(); ++currWordIndex) {
            const auto &word = dsSegment.words[currWordIndex];
            auto wordDuration = word.duration();

            for (size_t i = 0; i < word.phones.size(); ++i) {
                // durations
                {
                    bool currPhoneIsTheLastPhone = (i == word.phones.size() - 1);
                    auto currPhoneStart = phoneDurSum +
                                          word.phones[i].start;
                    auto nextPhoneStart = phoneDurSum +
                                          (currPhoneIsTheLastPhone ? wordDuration : word.phones[i + 1].start);
                    if (currPhoneIsTheLastPhone && (currWordIndex + 1 < dsSegment.words.size())) {
                        // If current word is not the last word
                        const auto &nextWord = dsSegment.words[currWordIndex + 1];
                        if (!nextWord.phones.empty()) {
                            nextPhoneStart += nextWord.phones[0].start;
                        }
                    }
                    int64_t currPhoneStartFrames = std::llround(currPhoneStart / frameLength);
                    int64_t nextPhoneStartFrames = std::llround(nextPhoneStart / frameLength);
                    durations.push_back(nextPhoneStartFrames - currPhoneStartFrames);
                }
            }
            phoneDurSum += wordDuration;
        }

        outTargetLength = std::accumulate(durations.begin(), durations.end(), int64_t{0});
        const std::array<int64_t, 2> shape{1, static_cast<int64_t>(durations.size())};
        return create_tensor<int64_t>("durations", durations.data(), durations.size(), shape.data(), shape.size());
    }

    std::vector<float> parseF0AsVector(const Segment &dsSegment, double frameLength, int64_t targetLength, double a4freq) {
        const auto it = dsSegment.parameters.find("pitch");
        if (it == dsSegment.parameters.end()) {
            return {JsonValue::Undefined};
        }

        const auto &param = it->second;
        auto samples = param.sample_curve.resample(frameLength, targetLength);
        if (param.tag != "pitch") {
            return {JsonValue::Undefined};
        }

        std::vector<float> f0(samples.size());
        std::transform(samples.begin(), samples.end(), f0.begin(), [a4freq](double midiPitch) {
            constexpr double semitonesInOctave = 12.0;
            constexpr double midiPitchOffset = 69.0;
            return static_cast<float>(a4freq * std::pow(2.0, (midiPitch - midiPitchOffset) / semitonesInOctave));
        });

        return f0;
    }

    JsonValue parseF0(const Segment &dsSegment, double frameLength, int64_t targetLength, double a4freq) {
        std::vector<float> f0 = parseF0AsVector(dsSegment, frameLength, targetLength, a4freq);

        const std::array<int64_t, 2> shape{1, static_cast<int64_t>(f0.size())};
        return create_tensor<float>("f0", f0.data(), f0.size(), shape.data(), shape.size());
    }

    JsonValue parseF0(const std::vector<float> &f0) {
        const std::array<int64_t, 2> shape{1, static_cast<int64_t>(f0.size())};
        return create_tensor<float>("f0", f0.data(), f0.size(), shape.data(), shape.size());
    }

    JsonValue parseVarianceParameter(const Segment &dsSegment, const char *parameter, const char *modelParameter,
                                     double frameLength, int64_t targetLength) {
        const auto it = dsSegment.parameters.find(parameter);
        if (it == dsSegment.parameters.end()) {
            return {JsonValue::Undefined};
        }

        const auto &param = it->second;
        if (param.tag != parameter) {
            return {JsonValue::Undefined};
        }

        auto samples = param.sample_curve.resample(frameLength, targetLength);
        const std::vector<float> samplesFloat(samples.begin(), samples.end());
        const std::array<int64_t, 2> shape{1, static_cast<int64_t>(samples.size())};
        return create_tensor<float>(modelParameter, samplesFloat.data(), samplesFloat.size(),
                                    shape.data(), shape.size());
    }

    JsonValue parseTransitionParameter(const Segment &dsSegment, const char *parameter, const char *modelParameter,
                                       float defaultValue,
                                       double frameLength, int64_t targetLength) {
        // gender, velocity
        const auto it = dsSegment.parameters.find(parameter);
        if (it == dsSegment.parameters.end()) {
            // missing parameter, using constant default value
            const std::vector<float> data(targetLength, defaultValue);
            const std::array<int64_t, 2> shape{1, static_cast<int64_t>(data.size())};
            return create_tensor<float>(modelParameter, data.data(), data.size(), shape.data(), shape.size());
        }

        const auto &param = it->second;
        if (param.tag != parameter) {
            return {JsonValue::Undefined};
        }

        auto samples = param.sample_curve.resample(frameLength, targetLength);
        const std::vector<float> samplesFloat(samples.begin(), samples.end());
        const std::array<int64_t, 2> shape{1, static_cast<int64_t>(samples.size())};
        return create_tensor<float>(modelParameter, samplesFloat.data(), samplesFloat.size(),
                                    shape.data(), shape.size());
    }

    JsonValue parseSpeakerMix(const SpeakerEmbed &spkEmb, const std::vector<std::string> &speakers,
                              const SpeakerMixCurve &spkMix, double frameLength, int64_t targetLength) {
        const auto data = getSpkMix(spkEmb, speakers, spkMix, frameLength, targetLength);
        std::array<int64_t, 3> shape = {int64_t{1}, targetLength, static_cast<int64_t>(SPK_EMBED_SIZE)};
        return create_tensor("spk_embed", data.data(), data.size(), shape.data(), shape.size());
    }

    bool readObjectHelper(const JsonObject &object, const std::string &type, std::unordered_map<std::string, int64_t> &out, Error *error) {
        out.reserve(object.size());
        for (const auto &[key, val] : object) {
            if (!val.isInt()) {
                if (error) {
                    *error = Error(Error::InvalidFormat,
                        "Invalid format of " + type + ": object value must be integer!");
                }
                return false;
            }
            out[key] = val.toInt64();
        }
        return true;
    }

    bool readJsonFileHelper(const std::filesystem::path &path, const std::string &type, std::unordered_map<std::string, int64_t> &out, Error *error) {
        std::ifstream phonemesFile(path);
        if (!phonemesFile.is_open()) {
            if (error) {
                *error = Error(Error::FileNotFound, "Could not open file '" + stdc::path::to_utf8(path) + "'");
            }
            return false;
        }
        std::stringstream s;
        s << phonemesFile.rdbuf();
        std::string errMsg;
        auto json = JsonValue::fromJson(s.str(), true, &errMsg);
        if (!errMsg.empty()) {
            if (error) {
                *error = Error(Error::InvalidFormat, std::move(errMsg));
            }
            return false;
        }
        if (!json.isObject()) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of " + type + ": JSON file does not contain a valid object");
            }
            return false;
        }
        return readObjectHelper(json.toObject(), type, out, error);
    }
}
