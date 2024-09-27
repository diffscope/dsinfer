#ifndef CONTRIBUTESPEC_P_H
#define CONTRIBUTESPEC_P_H

#include <map>
#include <utility>

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

        std::string id;
    };

}

#endif // CONTRIBUTESPEC_P_H
