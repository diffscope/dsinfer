#include "libraryspec.h"

#include <fstream>
#include <utility>

#include "dsinfer_pimpl.h"

namespace dsinfer {

    bool LibraryDependency::operator==(const LibraryDependency &other) const {
        return id == other.id && version == other.version && required == other.required;
    }

    class LibrarySpec::Impl {
    public:
        std::string id;
        std::string version;
        std::string compatVersion;

        std::string description;
        std::string vendor;
        std::string copyright;
        std::string url;

        std::vector<LibraryDependency> dependencies;
        JsonObject properties;

        std::vector<LibraryManifestBase *> content;

        ~Impl() {
            for (const auto &item : std::as_const(content)) {
                delete item;
            }
        }
    };

    LibrarySpec::LibrarySpec() {
    }

    LibrarySpec::~LibrarySpec() {
    }

    std::string LibrarySpec::id() const {
        __impl_t;
        return impl.id;
    }

    std::string LibrarySpec::version() const {
        __impl_t;
        return impl.version;
    }

    std::string LibrarySpec::compatVersion() const {
        __impl_t;
        return impl.compatVersion;
    }

    std::string LibrarySpec::description() const {
        __impl_t;
        return impl.description;
    }

    std::string LibrarySpec::vendor() const {
        __impl_t;
        return impl.vendor;
    }

    std::string LibrarySpec::copyright() const {
        __impl_t;
        return impl.copyright;
    }

    std::string LibrarySpec::url() const {
        __impl_t;
        return impl.url;
    }

    std::vector<LibraryDependency> LibrarySpec::dependencies() const {
        __impl_t;
        return impl.dependencies;
    }

    JsonObject LibrarySpec::properties() const {
        __impl_t;
        return impl.properties;
    }

    std::vector<LibraryManifestBase *> LibrarySpec::content() const {
        __impl_t;
        return impl.content;
    }

    LibrarySpec LibrarySpec::read(const std::filesystem::path &path) {
        return LibrarySpec();
    }

}