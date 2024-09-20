#ifndef LIBRARYINFO_H
#define LIBRARYINFO_H

#include <map>
#include <vector>
#include <filesystem>
#include <functional>

#include <dsinferCore/librarymanifestbase.h>
#include <dsinferCore/json_cxxapi.h>

namespace dsinfer {

    struct DSINFER_CORE_EXPORT LibraryDependency {
        std::string id;
        std::string version;
        bool required;

        inline LibraryDependency(bool required = true) : required(required) {
        }

        bool operator==(const LibraryDependency &other) const;
    };

    class DSINFER_CORE_EXPORT LibrarySpec {
    public:
        LibrarySpec();
        ~LibrarySpec();

    public:
        std::string id() const;
        std::string version() const;
        std::string compatVersion() const;
        std::string description() const;
        std::string vendor() const;
        std::string copyright() const;
        std::string url() const;

        std::vector<LibraryDependency> dependencies() const;
        JsonObject properties() const;

        std::vector<LibraryManifestBase *> content() const;

    public:
        LibrarySpec read(const std::filesystem::path &path);

    protected:
        class Impl;
        std::shared_ptr<Impl> _impl;
    };

}

#endif // LIBRARYINFO_H
