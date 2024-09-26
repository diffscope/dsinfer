#include "pluginfactory.h"
#include "pluginfactory_p.h"

namespace dsinfer {

    PluginFactory::Impl::Impl(PluginFactory *decl) : _decl(decl) {
    }

    PluginFactory::Impl::~Impl() = default;

    void PluginFactory::Impl::scanPlugins(const char *iid) const {

    }

    PluginFactory::PluginFactory() : _impl(new Impl(this)) {
    }

    PluginFactory::~PluginFactory() = default;

    void PluginFactory::addStaticPlugin(Plugin *plugin) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        impl.staticPlugins.push_back(plugin);
        impl.pluginsDirty.insert(plugin->iid());
    }

    std::vector<Plugin *> PluginFactory::staticPlugins() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.plugins_mtx);
        return impl.staticPlugins;
    }

    void PluginFactory::addPluginPath(const char *iid, const std::filesystem::path &path) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        impl.pluginPaths[iid].push_back(path);
        impl.pluginsDirty.insert(iid);
    }

    void PluginFactory::setPluginPaths(const char *iid,
                                       const std::vector<std::filesystem::path> &paths) {
        __dsinfer_impl_t;
        std::unique_lock<std::shared_mutex> lock(impl.plugins_mtx);
        if (paths.empty()) {
            impl.pluginPaths.erase(iid);
        } else {
            impl.pluginPaths[iid] = paths;
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