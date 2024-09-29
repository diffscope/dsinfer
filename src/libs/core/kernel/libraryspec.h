#ifndef LIBRARYSPEC_H
#define LIBRARYSPEC_H

#include <string>
#include <vector>

#include <dsinfer/displaytext.h>
#include <dsinfer/versionnumber.h>
#include <dsinfer/environment.h>

namespace dsinfer {

    struct DSINFER_EXPORT LibraryDependency {
        std::string id;
        VersionNumber version;
        bool required;

        inline LibraryDependency(bool required = true) : required(required) {
        }

        bool operator==(const LibraryDependency &other) const;
    };

    class ContributeSpec;

    class DSINFER_EXPORT LibrarySpec {
    public:
        ~LibrarySpec();

        LibrarySpec(LibrarySpec &&other) noexcept;
        LibrarySpec &operator=(LibrarySpec &&other) noexcept;

    public:
        std::filesystem::path path() const;

        std::string id() const;

        VersionNumber version() const;
        VersionNumber compatVersion() const;

        DisplayText description() const;
        DisplayText vendor() const;
        DisplayText copyright() const;
        std::filesystem::path readme() const;
        std::string url() const;

        std::vector<ContributeSpec *> contributes(int type) const;
        ContributeSpec *contribute(int type, const std::string &id) const;

        const std::vector<LibraryDependency> &dependencies() const;

    public:
        Error error() const;
        bool isLoaded() const;

        Environment *env() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        explicit LibrarySpec(Environment *env);

        friend class Environment;
    };

}

#endif // LIBRARYSPEC_H
