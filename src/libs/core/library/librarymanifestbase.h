#ifndef DSINFERCORE_LIBRARYMANIFESTBASE_H
#define DSINFERCORE_LIBRARYMANIFESTBASE_H

#include <string>
#include <memory>
#include <filesystem>

#include <dsinferCore/dsinfercoreglobal.h>

namespace dsinfer {

    class LibraryManifestBase {
    public:
        enum Type {
            Singer = 1,
            Inference = 2,
        };

        LibraryManifestBase(Type type, const std::filesystem::path &path);
        virtual ~LibraryManifestBase();

    public:
        Type type() const;
        std::filesystem::path path() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
        explicit LibraryManifestBase(Impl *impl);
    };

}

#endif // DSINFERCORE_LIBRARYMANIFESTBASE_H
