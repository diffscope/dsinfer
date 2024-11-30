#include "speaker_embed.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>

#include <stdcorelib/strings.h>
#include "acoustic_logger.h"

#ifdef _WIN32
#define DS_STRING_CONVERT(x) stdc::strings::conv<std::wstring>::from_utf8(x)
#else
#define DS_STRING_CONVERT(x) (x)
#endif

namespace dsinfer::dsinterp {

SpeakerEmbed::SpeakerEmbed() = default;

bool SpeakerEmbed::loadSpeakers(const std::vector<SpeakerPair> &speakers) {
    if (speakers.empty()) {
        return false;
    }
    for (const auto &[speaker, fullPath] : speakers) {

        if (!std::filesystem::exists(fullPath)) {
            // ERROR!
            acoustic_log().critical("emb file of speaker \"" + speaker + "\" does not exist!");
            return false;
        }
        auto size = std::filesystem::file_size(fullPath);
        if (size != SPK_EMBED_SIZE * sizeof(float)) {
            acoustic_log().critical("emb file size of speaker \"" + speaker + "\" must be exactly " + std::to_string(SPK_EMBED_SIZE) + " bytes!");
            return false;
        }
        std::ifstream inputFile(fullPath, std::ios::binary);
        if (!inputFile.is_open()) {
            acoustic_log().critical("emb file size of speaker \"" + speaker + "\" could not be opened!");
            return false;
        }
        SpeakerEmbedArray emb{};
        inputFile.read(reinterpret_cast<char *>(emb.data()), emb.size() * sizeof(float));
        m_emb[speaker] = emb;
        SpeakerEmbedMap mmmm;

        inputFile.close();
    }
    return true;
}

SpeakerEmbedArray SpeakerEmbed::getMixedEmb(const std::unordered_map<std::string, double> &mix) const {
    SpeakerEmbedArray arr{};
    for (const auto &item : mix) {
        auto it = m_emb.find(item.first);
        if (it != m_emb.end()) {
            const auto currentArr = it->second;
            for (size_t i = 0; i < SPK_EMBED_SIZE; i++) {
                arr[i] += static_cast<float>(currentArr[i] * item.second);
            }
        }
    }
    return arr;
}

std::unordered_map<std::string, double> SpeakerEmbed::parseMixString(const std::string &inputString) {
    std::unordered_map<std::string, double> result;
    std::vector<std::string> namesWithoutWeight;

    std::istringstream iss(inputString);
    std::string token;

    double weightSum = 0.0;

    while (std::getline(iss, token, '|')) {
        std::istringstream mapIss(token);
        std::string key;
        double value = 0.0;

        // Check if the token contains a ':' character
        if (token.find(':') == std::string::npos) {
            namesWithoutWeight.push_back(token);
        } else {
            while (std::getline(mapIss, token, ':')) {
                key = token;
                std::getline(mapIss, token, ':');
                value = std::stod(token);
                result[key] = value;
                weightSum += value;
            }
        }
    }

    if (!namesWithoutWeight.empty()) {
        weightSum = (weightSum > 1.0) ? 1.0 : weightSum;
        weightSum = 1.0 - weightSum;
        weightSum /= static_cast<double>(namesWithoutWeight.size());

        for (const auto &w : namesWithoutWeight) {
            result[w] = weightSum;
        }
    }
    return result;
}

SpeakerEmbedArray SpeakerEmbed::getMixedEmb(const std::string &inputString) const {
    return getMixedEmb(parseMixString(inputString));
}

}