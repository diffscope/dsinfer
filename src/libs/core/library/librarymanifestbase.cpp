#include "librarymanifestbase.h"
#include "librarymanifestbase_p.h"

#include "dsinfer_pimpl.h"

namespace dsinfer {

    LibraryManifestBase::LibraryManifestBase(LibraryManifestBase::Type type,
                                             const std::filesystem::path &path)
        : LibraryManifestBase(new Impl(type, path)) {
    }

    LibraryManifestBase::~LibraryManifestBase() = default;

    LibraryManifestBase::Type LibraryManifestBase::type() const {
        __impl_t;
        return impl.type;
    }

    std::filesystem::path LibraryManifestBase::path() const {
        __impl_t;
        return impl.path;
    }

    LibraryManifestBase::LibraryManifestBase(LibraryManifestBase::Impl *impl) : _impl(impl) {
    }

}