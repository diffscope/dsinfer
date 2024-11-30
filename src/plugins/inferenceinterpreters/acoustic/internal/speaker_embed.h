#ifndef SPEAKER_EMBED_H
#define SPEAKER_EMBED_H

#include <array>
#include <vector>
#include <string>
#include <utility>
#include <unordered_map>
#include <filesystem>

namespace dsinfer::dsinterp {
    constexpr unsigned int SPK_EMBED_SIZE = 256;
    using SpeakerEmbedArray = std::array<float, SPK_EMBED_SIZE>;
    using SpeakerEmbedMap = std::unordered_map<std::string, SpeakerEmbedArray>;
    using SpeakerPair = std::pair<std::string, std::filesystem::path>;

    class SpeakerEmbed {
    private:
        SpeakerEmbedMap m_emb;
    public:
        SpeakerEmbed();

        bool loadSpeakers(const std::vector<SpeakerPair> &speakers);
        SpeakerEmbedArray getMixedEmb(const std::unordered_map<std::string, double> &mix) const;
        SpeakerEmbedArray getMixedEmb(const std::string &inputString) const;

        static std::unordered_map<std::string, double> parseMixString(const std::string &inputString);

        const SpeakerEmbedMap &getEmb() {
            return m_emb;
        }
    };


}
#endif // SPEAKER_EMBED_H