#include "environment.h"
#include "environment_p.h"

#include "inferenceregistry.h"
#include "singerregistry.h"
#include "format.h"
#include "algorithms.h"

#include "libraryspec_p.h"
#include "contributespec_p.h"

namespace fs = std::filesystem;

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
        closeAllLoadedLibraries();
        deleteAll(registries);
    }

    void Environment::Impl::closeAllLoadedLibraries() {
        __dsinfer_decl_t;
        while (!loadedLibraryMap.libraries.empty()) {
            auto spec = loadedLibraryMap.libraries.back().spec;
            std::ignore = decl.closeLibrary(spec);
        }
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

    static fs::path getCanonicalPath(const fs::path &path) {
        try {
            return fs::canonical(path);
        } catch (...) {
        }
        return {};
    }

    LibrarySpec *Environment::openLibrary(const std::filesystem::path &path, bool noLoad,
                                          Error *error) {
        __dsinfer_impl_t;

        auto canonicalPath = getCanonicalPath(path);
        if (canonicalPath.empty()) {
            *error = {
                Error::FileNotFound,
                formatTextN("invalid library path \"%1\"", path),
            };
            return nullptr;
        }

        // Check library path
        if (!noLoad) {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            auto &libMap = impl.loadedLibraryMap;
            auto it = libMap.pathIndexes.find(canonicalPath);
            if (it != libMap.pathIndexes.end()) {
                auto &lib = *it->second;
                lib.ref++;
                return lib.spec;
            }
        }

        // Parse spec
        auto spec = new LibrarySpec(this);
        std::vector<ContributeSpec *> contributes;

        auto spec_d = spec->_impl.get();
        if (!spec_d->parse(path, impl.regSpecMap, &contributes, error)) {
            delete spec;
            deleteAll(contributes); // Maybe redundant
            return nullptr;
        }

        // Set parent
        for (const auto &contribute : std::as_const(contributes)) {
            contribute->_impl->parent = spec;
        }

        if (noLoad) {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            impl.resourceLibraries.insert(spec);
            return spec;
        }

        // Check duplication
        const auto &removePending = [&impl, spec] {
            auto it = impl.pendingLibraries.find(spec->id());
            auto &versionSet = it->second;
            versionSet.erase(spec->version());
            if (versionSet.empty()) {
                impl.pendingLibraries.erase(it);
            }
        };
        do {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            auto &libMap = impl.loadedLibraryMap;

            // Check loaded libraries
            {
                auto it = libMap.idIndexes.find(spec->id());
                if (it != libMap.idIndexes.end()) {
                    const auto &versionSet = it->second;
                    if (versionSet.count(spec->version())) {
                        goto out_dup;
                    }
                }
            }

            // Check pending list
            {
                auto it = impl.pendingLibraries.find(spec->id());
                if (it != impl.pendingLibraries.end()) {
                    const auto &versionSet = it->second;
                    if (versionSet.count(spec->version())) {
                        goto out_dup;
                    }
                }
            }

            impl.pendingLibraries[spec->id()].insert(spec->version());
            break;

        out_dup:
            spec_d->err = {
                Error::FileDuplicated,
                formatTextN(R"(another library with same identifier "%1[%2]" is loaded)",
                            spec->id(), spec->version().toString()),
            };
            impl.resourceLibraries.insert(spec);
            return spec;
        } while (false);

        // Load dependencies
        std::vector<LibrarySpec *> dependencies;
        auto closeDependencies = [&dependencies, this]() {
            for (auto it = dependencies.rbegin(); it != dependencies.rend(); ++it) {
                std::ignore = closeLibrary(*it);
            }
        };
        do {
            Error error1;
            std::unordered_set<fs::path> dependencyPaths{spec->path()};
            for (const auto &dep : std::as_const(spec->dependencies())) {
                std::shared_lock<std::shared_mutex> lock(impl.env_mtx);
                auto depPath = LibrarySpec::Impl::searchDependency(impl.libraryPaths, dep);
                lock.unlock();
                if (depPath.empty()) {
                    error1 = {
                        Error::LibraryNotFound,
                        formatTextN("specified library \"%1[%2]\" not found", dep.id,
                                    dep.version.toString()),
                    };
                    goto out_deps;
                }

                if (dependencyPaths.count(depPath)) {
                    error1 = {
                        Error::LibraryNotFound,
                        formatTextN("recursive dependency chain detected: %1", depPath),
                    };
                    goto out_deps;
                }

                Error error2;
                auto depLib = openLibrary(depPath, true, &error2);
                if (!depLib) {
                    error1 = {
                        Error::LibraryNotFound,
                        formatTextN("failed to load dependency \"%1\": %2", depPath,
                                    error2.message()),
                    };
                    goto out_deps;
                }
                dependencies.push_back(depLib);
                dependencyPaths.emplace(depPath);
            }

            break;

        out_deps:
            closeDependencies();
            spec_d->err = error1;

            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            removePending();
            impl.resourceLibraries.insert(spec);
            return spec;
        } while (false);

        // Initialize
        {
            Error error1;
            bool failed = false;
            int i = 0;
            for (; i < contributes.size(); ++i) {
                const auto &contribute = contributes[i];
                const auto &type = contribute->type();
                const auto &reg = impl.registries[type];
                if (!reg->loadSpec(contribute, ContributeSpec::Initialized, &error1)) {
                    failed = true;
                }
                contribute->_impl->state = ContributeSpec::Initialized;
            }

            if (failed) {
                // Delete
                for (; i >= 0; --i) {
                    const auto &contribute = contributes[i];
                    const auto &type = contribute->type();
                    const auto &reg = impl.registries[type];
                    Error error2;
                    std::ignore = reg->loadSpec(contribute, ContributeSpec::Deleted, &error2);
                    contribute->_impl->state = ContributeSpec::Deleted;
                }

                closeDependencies();
                spec_d->err = error1;

                std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
                removePending();
                impl.resourceLibraries.insert(spec);
                return spec;
            }
        }

        // Get ready
        {
            Error error1;
            bool failed = false;
            int i = 0;
            for (; i < contributes.size(); ++i) {
                const auto &contribute = contributes[i];
                const auto &type = contribute->type();
                const auto &reg = impl.registries[type];
                if (!reg->loadSpec(contribute, ContributeSpec::Ready, &error1)) {
                    failed = true;
                }
                contribute->_impl->state = ContributeSpec::Ready;
            }

            if (failed) {
                // Finish
                for (; i >= 0; --i) {
                    const auto &contribute = contributes[i];
                    const auto &type = contribute->type();
                    const auto &reg = impl.registries[type];
                    Error error2;
                    std::ignore = reg->loadSpec(contribute, ContributeSpec::Finished, &error2);
                    contribute->_impl->state = ContributeSpec::Finished;
                }

                // Delete
                for (i = (int) contributes.size(); i >= 0; --i) {
                    const auto &contribute = contributes[i];
                    const auto &type = contribute->type();
                    const auto &reg = impl.registries[type];
                    Error error2;
                    std::ignore = reg->loadSpec(contribute, ContributeSpec::Deleted, &error2);
                    contribute->_impl->state = ContributeSpec::Deleted;
                }

                closeDependencies();
                spec_d->err = error1;

                std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
                removePending();
                impl.resourceLibraries.insert(spec);
                return spec;
            }
        }

        spec_d->loaded = true;

        // Add to link map
        {
            Impl::LibraryData lib;
            lib.spec = spec;
            lib.ref = 1;
            lib.contributes = std::move(contributes);
            lib.linked = std::move(dependencies);

            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            removePending();
            auto &libMap = impl.loadedLibraryMap;
            auto it = libMap.libraries.insert(libMap.libraries.end(), lib);
            libMap.pathIndexes[spec->path()] = it;
            libMap.idIndexes[spec->id()][spec->version()] = it;
            libMap.pointerIndexes[spec] = it;
        }
        return spec;
    }

    bool Environment::closeLibrary(LibrarySpec *spec) {
        __dsinfer_impl_t;

        if (!spec->isLoaded()) {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            auto it = impl.resourceLibraries.find(spec);
            if (it == impl.resourceLibraries.end()) {
                return false;
            }

            impl.resourceLibraries.erase(it);
            delete spec;
            return true;
        }

        Impl::LibraryData libToClose;
        {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            auto &libMap = impl.loadedLibraryMap;
            auto it = libMap.pointerIndexes.find(spec);
            if (it == libMap.pointerIndexes.end()) {
                return false;
            }

            auto it1 = it->second;
            auto &lib = *it1;
            lib.ref--;
            if (lib.ref != 0) {
                return true;
            }
            libToClose = std::move(lib);

            libMap.libraries.erase(it1);
            libMap.pointerIndexes.erase(it);
            libMap.pathIndexes.erase(spec->path());

            // Remove id indexes
            auto it2 = libMap.idIndexes.find(spec->id());
            auto &versionMap = it2->second;
            versionMap.erase(spec->version());
            if (versionMap.empty()) {
                libMap.idIndexes.erase(it2);
            }
        }

        // Finish and delete
        {
            // Finish
            for (auto it = libToClose.contributes.rbegin(); it != libToClose.contributes.rend();
                 ++it) {
                const auto &contribute = *it;
                const auto &type = contribute->type();
                const auto &reg = impl.registries[type];
                Error error2;
                std::ignore = reg->loadSpec(contribute, ContributeSpec::Finished, &error2);
                contribute->_impl->state = ContributeSpec::Finished;
            }

            // Delete
            for (auto it = libToClose.contributes.rbegin(); it != libToClose.contributes.rend();
                 ++it) {
                const auto &contribute = *it;
                const auto &type = contribute->type();
                const auto &reg = impl.registries[type];
                Error error2;
                std::ignore = reg->loadSpec(contribute, ContributeSpec::Deleted, &error2);
                contribute->_impl->state = ContributeSpec::Deleted;
            }
        }

        // Unload dependencies
        for (auto it = libToClose.linked.rbegin(); it != libToClose.linked.rend(); ++it) {
            closeLibrary(*it);
        }
        delete spec;
        return true;
    }

    LibrarySpec *Environment::findLibrary(const std::string &id,
                                          const VersionNumber &version) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx);
        auto &libMap = impl.loadedLibraryMap;
        auto it = libMap.idIndexes.find(id);
        if (it == libMap.idIndexes.end()) {
            return nullptr;
        }

        auto &versionMap = it->second;
        auto it2 = versionMap.find(version);
        if (it2 == versionMap.end()) {
            return nullptr;
        }
        return (*it2->second).spec;
    }

    std::vector<LibrarySpec *> Environment::findLibraries(const std::string &id) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx);
        auto &libMap = impl.loadedLibraryMap;
        auto it = libMap.idIndexes.find(id);
        if (it == libMap.idIndexes.end()) {
            return {};
        }

        auto &versionMap = it->second;
        std::vector<LibrarySpec *> res;
        res.reserve(versionMap.size());
        for (const auto &pair : versionMap) {
            res.push_back(pair.second->spec);
        }
        return res;
    }

    std::vector<LibrarySpec *> Environment::libraries() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx);
        auto &list = impl.loadedLibraryMap.libraries;

        std::vector<LibrarySpec *> res;
        res.reserve(list.size());
        for (const auto &lib : list) {
            res.push_back(lib.spec);
        }
        return res;
    }

}