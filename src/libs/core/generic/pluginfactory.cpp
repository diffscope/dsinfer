#include "pluginfactory.h"
#include "pluginfactory_p.h"

#include <utility>
#include <cstring>
#include <mutex>

#include <stdcorelib/pimpl.h>

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
                    if (libraryInstances.count(entryPath) || !stdc::Library::isLibrary(entryPath)) {
                        continue;
                    }

                    stdc::Library so;
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
                    libraryInstances[entryPath] = new stdc::Library(std::move(so));
                }
            }
        }

        if (plugins.empty()) {
            allPlugins.erase(iid);
        }
    }

    /*!
        Constructs a plugin factory.
    */
    PluginFactory::PluginFactory() : _impl(new Impl(this)) {
    }

    /*!
        Destructs a plugin factory.
    */
    PluginFactory::~PluginFactory() = default;

    /*!
        Adds a static plugin to the internal plugin map.
    */
    void PluginFactory::addStaticPlugin(Plugin *plugin) {
        __stdc_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        impl.staticPlugins.emplace(plugin);
        impl.pluginsDirty.insert(plugin->iid());
    }

    /*!
        Returns the static plugins.
    */
    std::vector<Plugin *> PluginFactory::staticPlugins() const {
        __stdc_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.plugins_mtx);
        return {impl.staticPlugins.begin(), impl.staticPlugins.end()};
    }

    /*!
        Adds a plugin searching path of interface id \a iid.
    */
    void PluginFactory::addPluginPath(const char *iid, const std::filesystem::path &path) {
        __stdc_impl_t;
        if (!fs::is_directory(path)) {
            return;
        }
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        impl.pluginPaths[iid].push_back(fs::canonical(path));
        impl.pluginsDirty.insert(iid);
    }

    /*!
        Sets the plugin searching paths of interface id \a iid.
    */
    void PluginFactory::setPluginPaths(const char *iid,
                                       const std::vector<std::filesystem::path> &paths) {
        __stdc_impl_t;
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

    /*!
        Returns the plugin searching paths of interface id \a iid.
    */
    const std::vector<std::filesystem::path> &PluginFactory::pluginPaths(const char *iid) const {
        __stdc_impl_t;

        std::shared_lock<std::shared_mutex> lock(impl.plugins_mtx);
        auto it = impl.pluginPaths.find(iid);
        if (it == impl.pluginPaths.end()) {
            static std::vector<std::filesystem::path> _dummy;
            return _dummy;
        }
        return it->second;
    }

    /*!
        Returns the plugin instance of the matching identifiers if successfully found and loaded.
    */
    Plugin *PluginFactory::plugin(const char *iid, const char *key) const {
        __stdc_impl_t;

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

    /*!
        \internal
    */
    PluginFactory::PluginFactory(Impl &impl) : _impl(&impl) {
    }

}