#ifndef CONTRIBUTESPEC_P_H
#define CONTRIBUTESPEC_P_H

#include <map>
#include <utility>

#include <dsinfer/contributespec.h>

namespace dsinfer {

    class ContributeSpec::Impl {
    public:
        Impl(int type, LibrarySpec *parent) : type(type), parent(parent) {
        }
        virtual ~Impl() = default;

    public:
        virtual bool read(std::filesystem::path &basePath, const JsonObject &obj,
                          Error *error) = 0;

    public:
        int type;
        LibrarySpec *parent;

        std::string id;
    };

}

#endif // CONTRIBUTESPEC_P_H
