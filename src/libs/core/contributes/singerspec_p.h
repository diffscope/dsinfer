#ifndef SINGERSPEC_P_H
#define SINGERSPEC_P_H

#include <dsinfer/singerspec.h>
#include <dsinfer/private/contributespec_p.h>

namespace dsinfer {

    class SingerSpec::Impl : public ContributeSpec::Impl {
    public:
        Impl() : ContributeSpec::Impl(ContributeSpec::Singer) {
        }

    public:
        bool read(const std::filesystem::path &basePath, const dsinfer::JsonObject &obj,
                  Error *error) override;

        std::filesystem::path path;

        std::string name;
        std::string model;

        std::filesystem::path avatar;
        std::filesystem::path background;
        std::filesystem::path demoAudio;

        std::vector<SingerImport> imports;

        JsonObject configuration;
    };

}

#endif // SINGERSPEC_P_H
