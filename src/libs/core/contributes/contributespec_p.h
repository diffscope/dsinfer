#ifndef DSINFER_CONTRIBUTESPEC_P_H
#define DSINFER_CONTRIBUTESPEC_P_H

#include <dsinfer/contributespec.h>

namespace dsinfer {

    class ContributeSpec::Impl {
    public:
        explicit Impl(int type) : type(type), state(Invalid) {
        }
        virtual ~Impl() = default;

    public:
        virtual bool read(const std::filesystem::path &basePath, const JsonObject &obj,
                          Error *error);

    public:
        int type;
        ContributeSpec::State state;
        LibrarySpec *parent = nullptr;

        VersionNumber fmtVersion;
        std::string id;
    };

}

#endif // DSINFER_CONTRIBUTESPEC_P_H
