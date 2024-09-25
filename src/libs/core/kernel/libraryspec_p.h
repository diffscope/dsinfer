#ifndef LIBRARYSPEC_P_H
#define LIBRARYSPEC_P_H

#include <map>

#include <dsinfer/libraryspec.h>

namespace dsinfer {

    class LibrarySpec::Impl {
    public:
        explicit Impl(Environment *env) : env(env) {
        }
        ~Impl();

        Environment *env;

        std::string id;

        VersionNumber version;
        VersionNumber compatVersion;

        std::string description;
        std::string vendor;
        std::string copyright;
        std::string url;

        std::map<int, std::vector<ContributeSpec *>> contributes;

        std::vector<LibraryDependency> dependencies;
    };

}

#endif // LIBRARYSPEC_P_H
