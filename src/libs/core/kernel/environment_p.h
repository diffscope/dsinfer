#ifndef ENVIRONMENT_P_H
#define ENVIRONMENT_P_H

#include <map>
#include <unordered_set>

#include <dsinfer/environment.h>
#include <dsinfer/contributespec.h>
#include <dsinfer/private/pluginfactory_p.h>

namespace dsinfer {

    class Environment::Impl : public PluginFactory::Impl {
    public:
        explicit Impl(Environment *decl);
        ~Impl();

        using Decl = Environment;

    public:
        void closeAllLoadedLibraries();

        std::vector<ContributeRegistry *> registries;
        std::map<std::string, ContributeRegistry *> regSpecMap;

        std::vector<std::filesystem::path> libraryPaths;

        struct LibraryData {
            LibrarySpec *spec = nullptr;
            int ref = 0;
            std::vector<ContributeSpec *> contributes;
            std::vector<LibrarySpec *> linked;
        };
        class LibraryMap {
        public:
            std::list<LibraryData> libraries;
            std::unordered_map<std::filesystem::path, decltype(libraries)::iterator> pathIndexes;
            std::unordered_map<std::string,
                               std::unordered_map<VersionNumber, decltype(libraries)::iterator>>
                idIndexes;
            std::unordered_map<LibrarySpec *, decltype(libraries)::iterator> pointerIndexes;

        };
        LibraryMap loadedLibraryMap;
        std::unordered_set<LibrarySpec *> resourceLibraries;

        // temp
        std::unordered_map<std::string, std::unordered_set<VersionNumber>> pendingLibraries;

        mutable std::shared_mutex env_mtx;
    };

}

#endif // ENVIRONMENT_P_H
