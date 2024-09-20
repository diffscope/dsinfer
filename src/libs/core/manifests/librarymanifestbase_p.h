#ifndef DSINFERCORE_LIBRARYMANIFESTBASE_P_H
#define DSINFERCORE_LIBRARYMANIFESTBASE_P_H

#include <dsinferCore/librarymanifestbase.h>

namespace dsinfer {

    class LibraryManifestBase::Impl {
    public:
        explicit Impl(Type type, const std::filesystem::path &path) : type(type), path(path) {
        }
        virtual ~Impl() = default;

        Type type;
        std::filesystem::path path;
    };

}

#endif // DSINFERCORE_LIBRARYMANIFESTBASE_P_H
