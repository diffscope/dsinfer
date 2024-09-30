#include "pluginfactory.h"
#include "pluginfactory_p.h"

namespace fs = std::filesystem;

namespace dsinfer {

    PluginFactory::Impl::Impl(PluginFactory *decl) : _decl(decl) {
    }

    PluginFactory::Impl::~Impl() {
        // Unload all libraries
        for (const auto &item : std::as_const(libraryInstances)) {
            delete item.second;
        }
    }

    void PluginFactory::Impl::scanPlugins(const char *iid) const {
        auto &plugins = allPlugins[iid];
        for (const auto &plugin : staticPlugins) {
            if (strcmp(iid, plugin->iid()) == 0) {
                std::ignore = plugins.insert(std::make_pair(plugin->key(), plugin));
            }
        }

        auto it = pluginPaths.find(iid);
        if (it != pluginPaths.end()) {
            for (const auto &pluginPath : it->second) {
                for (const auto &entry : fs::directory_iterator(pluginPath)) {
                    const auto &entryPath = fs::canonical(entry.path());
                    if (libraryInstances.count(entryPath) || !SharedLibrary::isLibrary(entryPath)) {
                        continue;
                    }

                    SharedLibrary so;
                    if (!so.open(entryPath)) {
                        continue;
                    }

                    using PluginGetter = Plugin *(*) ();
                    auto getter =
                        reinterpret_cast<PluginGetter>(so.resolve("dsinfer_plugin_instance"));
                    if (!getter) {
                        continue;
                    }

                    auto plugin = getter();
                    if (!plugin || strcmp(iid, plugin->iid()) != 0 ||
                        !plugins.insert(std::make_pair(plugin->key(), plugin)).second) {
                        continue;
                    }
                    libraryInstances[entryPath] = new SharedLibrary(std::move(so));
                }
            }
        }

        if (plugins.empty()) {
            allPlugins.erase(iid);
        }
    }

    PluginFactory::PluginFactory() : _impl(new Impl(this)) {
    }

    PluginFactory::~PluginFactory() = default;

    void PluginFactory::addStaticPlugin(Plugin *plugin) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        impl.staticPlugins.emplace(plugin);
        impl.pluginsDirty.insert(plugin->iid());
    }

    std::vector<Plugin *> PluginFactory::staticPlugins() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.plugins_mtx);
        return {impl.staticPlugins.begin(), impl.staticPlugins.end()};
    }

    void PluginFactory::addPluginPath(const char *iid, const std::filesystem::path &path) {
        __dsinfer_impl_t;
        if (!fs::is_directory(path)) {
            return;
        }
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        impl.pluginPaths[iid].push_back(fs::canonical(path));
        impl.pluginsDirty.insert(iid);
    }

    void PluginFactory::setPluginPaths(const char *iid,
                                       const std::vector<std::filesystem::path> &paths) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        if (paths.empty()) {
            impl.pluginPaths.erase(iid);
        } else {
            std::vector<fs::path> realPaths;
            realPaths.reserve(paths.size());
            for (const auto &path : paths) {
                if (fs::is_directory(path)) {
                    realPaths.push_back(fs::canonical(path));
                }
            }
            impl.pluginPaths[iid] = realPaths;
        }
        impl.pluginsDirty.insert(iid);
    }

    const std::vector<std::filesystem::path> &PluginFactory::pluginPaths(const char *iid) const {
        __dsinfer_impl_t;

        std::shared_lock<std::shared_mutex> lock(impl.plugins_mtx);
        auto it = impl.pluginPaths.find(iid);
        if (it == impl.pluginPaths.end()) {
            static std::vector<std::filesystem::path> _dummy;
            return _dummy;
        }
        return it->second;
    }

    Plugin *PluginFactory::plugin(const char *iid, const char *key) const {
        __dsinfer_impl_t;

        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        if (impl.pluginsDirty.count(iid)) {
            impl.scanPlugins(iid);
        }

        auto it = impl.allPlugins.find(iid);
        if (it == impl.allPlugins.end()) {
            return nullptr;
        }

        const auto &pluginsMap = it->second;
        auto it2 = pluginsMap.find(key);
        if (it2 == pluginsMap.end()) {
            return nullptr;
        }
        return it2->second;
    }

    PluginFactory::PluginFactory(Impl &impl) : _impl(&impl) {
    }

}