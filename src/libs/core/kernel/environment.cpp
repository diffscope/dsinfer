#include "environment.h"
#include "environment_p.h"

#include <mutex>

#include <stdcorelib/path.h>

#include "inferenceregistry.h"
#include "singerregistry.h"
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
        __stdc_decl_t;
        while (!loadedLibraryMap.libraries.empty()) {
            auto spec = loadedLibraryMap.libraries.back().spec;
            std::ignore = decl.closeLibrary(spec);
        }
    }

    void Environment::Impl::refreshLibraryIndexes() {
        cachedLibraryIndexesMap.clear();
        for (const auto &path : std::as_const(libraryPaths)) {
            try {
                for (const auto &entry : fs::directory_iterator(path)) {
                    const auto filename = entry.path().filename();
                    if (!entry.is_directory()) {
                        continue;
                    }

                    JsonObject obj;
                    Error error;
                    if (!LibrarySpec::Impl::readDesc(entry.path(), &obj, &error)) {
                        continue;
                    }

                    // Search id, version, compatVersion
                    std::string id_;
                    VersionNumber version_;
                    VersionNumber compatVersion_;

                    // id
                    {
                        auto it = obj.find("id");
                        if (it == obj.end()) {
                            continue;
                        }
                        id_ = it->second.toString();
                        if (!ContributeIdentifier::isValidId(id_)) {
                            continue;
                        }
                    }
                    // version
                    {
                        auto it = obj.find("version");
                        if (it == obj.end()) {
                            continue;
                        }
                        version_ = VersionNumber::fromString(it->second.toString());
                    }
                    // compatVersion
                    {
                        auto it = obj.find("compatVersion");
                        if (it != obj.end()) {
                            compatVersion_ = VersionNumber::fromString(it->second.toString());
                        } else {
                            compatVersion_ = version_;
                        }
                    }

                    // Store
                    cachedLibraryIndexesMap[id_][version_] = {fs::canonical(entry.path()),
                                                              compatVersion_};
                }
            } catch (...) {
            }
        }

        libraryPathsDirty = false;
    }

    Environment::Environment() : PluginFactory(*new Impl(this)) {
    }

    Environment::~Environment() = default;

    ContributeRegistry *Environment::registry(int type) const {
        __stdc_impl_t;
        return impl.registries[type];
    }

    void Environment::addLibraryPaths(const std::vector<std::filesystem::path> &paths) {
        __stdc_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
        for (const auto &path : paths) {
            if (!fs::is_directory(path)) {
                continue;
            }
            impl.libraryPaths.push_back(fs::canonical(path));
            if (!impl.libraryPathsDirty) {
                impl.libraryPathsDirty = true;
            }
        }
    }
    void Environment::setLibraryPaths(const std::vector<std::filesystem::path> &paths) {
        __stdc_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
        impl.libraryPaths.clear();
        for (const auto &path : paths) {
            if (!fs::is_directory(path)) {
                continue;
            }
            impl.libraryPaths.push_back(fs::canonical(path));
            if (!impl.libraryPathsDirty) {
                impl.libraryPathsDirty = true;
            }
        }
    }
    const std::vector<std::filesystem::path> &Environment::libraryPaths() const {
        __stdc_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx);
        return impl.libraryPaths;
    }

    LibrarySpec *Environment::openLibrary(const std::filesystem::path &path, bool noLoad,
                                          Error *error) {
        __stdc_impl_t;

        auto canonicalPath = stdc::path::canonical(path);
        if (canonicalPath.empty() || !fs::is_directory(canonicalPath)) {
            *error = {
                Error::FileNotFound,
                stdc::formatN(R"(invalid library path "%1")", path),
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
        if (!spec_d->parse(canonicalPath, impl.regSpecMap, &contributes, error)) {
            delete spec;
            deleteAll(contributes); // Maybe redundant
            return nullptr;
        }

        // Set parent
        for (const auto &contribute : std::as_const(contributes)) {
            contribute->_impl->parent = spec;
        }

        // Add to library's data space
        for (const auto &contribute : std::as_const(contributes)) {
            spec_d->contributes[contribute->type()][contribute->id()] = contribute;
        }

        if (noLoad) {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            impl.resourceLibraries.insert(spec);
            return spec;
        }

        const auto &removePending = [&impl, spec] {
            auto it = impl.pendingLibraries.find(spec->id());
            auto &versionSet = it->second;
            versionSet.erase(spec->version());
            if (versionSet.empty()) {
                impl.pendingLibraries.erase(it);
            }
        };

        // Check duplications
        do {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            auto &libMap = impl.loadedLibraryMap;
            Error error1;

            // Check if a library with same id and version but different path is loaded
            {
                auto it = libMap.idIndexes.find(spec->id());
                if (it != libMap.idIndexes.end()) {
                    const auto &versionMap = it->second;
                    auto it2 = versionMap.find(spec->version());
                    if (it2 != versionMap.end()) {
                        auto lib = *it2->second;
                        error1 = {
                            Error::FileDuplicated,
                            stdc::formatN(R"(duplicated library "%1[%2]" in "%3" is loaded)",
                                          spec->id(), spec->version().toString(), lib.spec->path()),
                        };
                        goto out_dup;
                    }
                }
            }

            // Check pending list
            {
                auto it = impl.pendingLibraries.find(spec->id());
                if (it != impl.pendingLibraries.end()) {
                    const auto &versionMap = it->second;
                    auto it2 = versionMap.find(spec->version());
                    if (it2 != versionMap.end()) {
                        error1 = {
                            Error::RecursiveDependency,
                            stdc::formatN(
                                R"(recursive depencency chain detected: library "%1[%2]" in %3 is being loaded)",
                                spec->id(), spec->version().toString(), it2->second),
                        };
                        goto out_dup;
                    }
                }
            }

            impl.pendingLibraries[spec->id()][spec->version()] = spec->path();
            break;

        out_dup:
            spec_d->err = error1;
            impl.resourceLibraries.insert(spec);
            return spec;
        } while (false);

        // Refresh dependency cache if needed
        if (impl.libraryPathsDirty) {
            std::unique_lock<std::shared_mutex> lock(impl.env_mtx);
            impl.refreshLibraryIndexes();
        }

        // Load dependencies
        std::vector<LibrarySpec *> dependencies;
        auto closeDependencies = [&dependencies, this]() {
            for (auto it = dependencies.rbegin(); it != dependencies.rend(); ++it) {
                std::ignore = closeLibrary(*it);
            }
        };
        auto searchDependencies = [&impl](const std::string &id,
                                          const VersionNumber &version) -> std::vector<fs::path> {
            std::vector<fs::path> res;
            auto it = impl.cachedLibraryIndexesMap.find(id);
            if (it == impl.cachedLibraryIndexesMap.end()) {
                return {};
            }

            // Search precise version
            const auto &versionMap = it->second;
            {
                auto it2 = versionMap.find(version);
                if (it2 != versionMap.end()) {
                    res.emplace_back(it2->second.path);
                }
            }

            // Test from high version to low version
            for (auto it2 = versionMap.rbegin(); it2 != versionMap.rend(); ++it2) {
                if (it2->first < version) {
                    break;
                }
                const auto &brief = it2->second;
                if (brief.compatVersion <= version) {
                    res.emplace_back(it2->second.path);
                }
            }
            return res;
        };
        do {
            Error error1;
            for (const auto &dep : std::as_const(spec->dependencies())) {
                VersionNumber depVersion;

                // Try to load all matched libraries
                bool success = false;
                auto depPaths = searchDependencies(dep.id, dep.version);
                for (auto it = depPaths.rbegin(); it != depPaths.rend(); ++it) {
                    const auto &depPath = *it;
                    Error error2;
                    auto depLib = openLibrary(depPath, true, &error2);
                    if (!depLib) {
                        continue; // ignore
                    }
                    dependencies.push_back(depLib);
                    success = true;
                    break;
                }

                if (success) {
                    continue;
                }

                if (!dep.required) {
                    continue; // ignore
                }

                // Not found
                error1 = {
                    Error::LibraryNotFound,
                    stdc::formatN(R"(required library "%1[%2]" not found)", dep.id,
                                  dep.version.toString()),
                };
                goto out_deps;
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
                    i--;
                    failed = true;
                    break;
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
                    i--;
                    failed = true;
                    break;
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
                for (i = (int) contributes.size(); i-- > 0;) {
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
        __stdc_impl_t;

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
        __stdc_impl_t;
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
        __stdc_impl_t;
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
        __stdc_impl_t;
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