#include "environment.h"
#include "environment_p.h"

#include "inferenceregistry.h"
#include "singerregistry.h"
#include "format.h"
#include "algorithms.h"

#include "libraryspec_p.h"

namespace dsinfer {

    Environment::Impl::Impl(Environment *decl) : PluginFactory::Impl(decl) {
        registries.resize(2);
        registries[ContributeSpec::Inference] = new InferenceRegistry(decl);
        registries[ContributeSpec::Singer] = new SingerRegistry(decl);

        for (const auto &reg : std::as_const(registries)) {
            regSpecMap.insert(std::make_pair(reg->specKey(), reg));
        }
    }

    Environment::Impl::~Impl() {
        deleteAll(registries);
    }

    Environment::Environment() : PluginFactory(*new Impl(this)) {
    }

    Environment::~Environment() = default;

    ContributeRegistry *Environment::registry(int type) const {
        __dsinfer_impl_t;
        return impl.registries[type];
    }

    void Environment::addLibraryPath(const std::filesystem::path &path) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
        impl.libraryPaths.push_back(path);
    }
    void Environment::setLibraryPaths(const std::vector<std::filesystem::path> &paths) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
        impl.libraryPaths = paths;
    }
    const std::vector<std::filesystem::path> &Environment::libraryPaths() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx);
        return impl.libraryPaths;
    }

    LibrarySpec *Environment::openLibrary(const std::filesystem::path &path, Error *error) {
        __dsinfer_impl_t;
        auto spec = new LibrarySpec(this);
        if (!spec->_impl->read(path, impl.regSpecMap, error)) {
            delete spec;
            return nullptr;
        }
        return spec;
    }

    void Environment::closeLibrary(LibrarySpec *lib) {
        __dsinfer_impl_t;

        std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
    }

    LibrarySpec *Environment::findLibrary(const std::string &id,
                                          const VersionNumber &version) const {
        (void) this;
        return nullptr;
    }

    std::vector<LibrarySpec *> Environment::findLibraries(const std::string &id) const {
        (void) this;
        return {};
    }

    std::vector<LibrarySpec *> Environment::libraries() const {
        (void) this;
        return {};
    }
}