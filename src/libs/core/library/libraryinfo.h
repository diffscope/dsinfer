#ifndef LIBRARYINFO_H
#define LIBRARYINFO_H

#include <map>
#include <filesystem>

#include <dsinferCore/dsinfercoreglobal.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT LibraryInfo {
    public:
        LibraryInfo();
        ~LibraryInfo();

        enum Type {
            Singers = 1,
            Module = 2,
            UserType = 65535,
        };

    public:
        bool load(const std::filesystem::path &path);

    public:
        std::string id() const;
        std::string version() const;
        std::string compatVersion() const;

        std::string description() const;
        std::string vendor() const;
        std::string copyright() const;
        std::string url() const;

        Type type() const;
        std::string typeString() const;

    protected:
        class Impl;
        std::shared_ptr<Impl> _impl;
    };

}

#endif // LIBRARYINFO_H
