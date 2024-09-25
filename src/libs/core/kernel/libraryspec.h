#ifndef LIBRARYSPEC_H
#define LIBRARYSPEC_H

#include <string>
#include <vector>

#include <dsinfer/versionnumber.h>
#include <dsinfer/environment.h>

namespace dsinfer {

    struct DSINFER_EXPORT LibraryDependency {
        std::string id;
        VersionNumber version;
        bool required;

        inline explicit LibraryDependency(bool required = true) : required(required) {
        }

        bool operator==(const LibraryDependency &other) const;
    };

    class ContributeSpec;

    class DSINFER_EXPORT LibrarySpec {
    public:
        ~LibrarySpec();

    public:
        std::string id() const;

        VersionNumber version() const;
        VersionNumber compatVersion() const;

        std::string description() const;
        std::string vendor() const;
        std::string copyright() const;
        std::string url() const;

        const std::vector<ContributeSpec *> &contributes(int type) const;
        ContributeSpec *contribute(int type, const std::string &id) const;

        const std::vector<LibraryDependency> &dependencies() const;

    public:
        bool hasError() const;
        std::string errorMessage() const;

        Environment *env() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        explicit LibrarySpec(Environment *env);

        friend class Environment;
    };

}

#endif // LIBRARYSPEC_H
