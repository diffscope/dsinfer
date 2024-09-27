#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <filesystem>

#include <dsinfer/error.h>
#include <dsinfer/pluginfactory.h>
#include <dsinfer/versionnumber.h>

namespace dsinfer {

    class LibrarySpec;

    class ContributeRegistry;

    class DSINFER_EXPORT Environment : public PluginFactory {
    public:
        Environment();
        ~Environment();

    public:
        ContributeRegistry *registry(int type) const;

    public:
        void addLibraryPath(const std::filesystem::path &path);
        void setLibraryPaths(const std::vector<std::filesystem::path> &paths);
        const std::vector<std::filesystem::path> &libraryPaths() const;

    public:
        LibrarySpec *openLibrary(const std::filesystem::path &path, Error *error);
        void closeLibrary(LibrarySpec *lib);
        LibrarySpec *findLibrary(const std::string &id, const VersionNumber &version = {}) const;
        std::vector<LibrarySpec *> findLibraries(const std::string &id) const;
        std::vector<LibrarySpec *> libraries() const;

    protected:
        class Impl;

        friend class ContributeRegistry;
        friend class LibrarySpec;
    };

}

#endif // ENVIRONMENT_H
