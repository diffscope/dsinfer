#ifndef DSINFERCORE_SINGERMANIFEST_H
#define DSINFERCORE_SINGERMANIFEST_H

#include <map>
#include <vector>
#include <filesystem>

#include <dsinferCore/librarymanifestbase.h>
#include <dsinferCore/json_cxxapi.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT SingerInfo {
    public:
        SingerInfo();
        ~SingerInfo();

        struct PresetItem {
            struct Inference {
                std::string id;
                JsonObject arguments;
            };
            std::string id;
            Inference inference;
        };

    public:
        std::filesystem::path path() const;

    public:
        std::filesystem::path avatar() const;
        std::filesystem::path background() const;
        std::filesystem::path demoAudio() const;

        std::vector<PresetItem> preset() const;

    protected:
        class Impl;
        std::shared_ptr<Impl> _impl;
    };

    class DSINFER_CORE_EXPORT SingerManifest : public LibraryManifestBase {
    public:
        SingerManifest();

    public:
        std::vector<SingerInfo> singers() const;
    };

}

#endif // DSINFERCORE_SINGERMANIFEST_H
