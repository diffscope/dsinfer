#include "libraryinfo.h"

#include <fstream>

#include "dsinfer_pimpl.h"

namespace dsinfer {

    class LibraryInfo::Impl {
    public:
        std::string id;
        std::string version;
        std::string compatVersion;

        std::string description;
        std::string vendor;
        std::string copyright;
        std::string url;

        LibraryInfo::Type type;
        std::string typeString;
        std::vector<LibraryDependency> dependencies;
    };

    LibraryInfo::LibraryInfo() {
    }

    LibraryInfo::~LibraryInfo() {
    }

    bool LibraryInfo::load(const std::filesystem::path &path) {
        __impl_t;

        // TODO: Read file
        return true;
    }

    std::string LibraryInfo::id() const {
        __impl_t;
        return impl.id;
    }

    std::string LibraryInfo::version() const {
        __impl_t;
        return impl.version;
    }

    std::string LibraryInfo::compatVersion() const {
        __impl_t;
        return impl.compatVersion;
    }

    std::string LibraryInfo::description() const {
        __impl_t;
        return impl.description;
    }

    std::string LibraryInfo::vendor() const {
        __impl_t;
        return impl.vendor;
    }

    std::string LibraryInfo::copyright() const {
        __impl_t;
        return impl.copyright;
    }

    std::string LibraryInfo::url() const {
        __impl_t;
        return impl.url;
    }

    LibraryInfo::Type LibraryInfo::type() const {
        __impl_t;
        return impl.type;
    }

    std::string LibraryInfo::typeString() const {
        __impl_t;
        return impl.typeString;
    }

    const std::vector<LibraryDependency> &LibraryInfo::dependencies() const {
        __impl_t;
        return impl.dependencies;
    }

}