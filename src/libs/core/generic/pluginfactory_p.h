#ifndef PLUGINFACTORY_P_H
#define PLUGINFACTORY_P_H

#include <unordered_map>
#include <set>
#include <shared_mutex>

#include <dsinfer/pluginfactory.h>

namespace dsinfer {

    class PluginFactory::Impl {
    public:
        explicit Impl(PluginFactory *decl);
        virtual ~Impl();

        using Decl = PluginFactory;
        PluginFactory *_decl;

    public:
        void scanPlugins(const char *iid) const;

        std::unordered_map<std::string, std::vector<std::filesystem::path>> pluginPaths;
        std::vector<Plugin *> staticPlugins;
        mutable std::set<std::string> pluginsDirty;
        mutable std::unordered_map<std::string, std::unordered_map<std::string, Plugin *>>
            allPlugins;
        mutable std::unordered_map<std::filesystem::path, std::pair<std::string, std::string>>
            pluginCache;
        mutable std::shared_mutex plugins_mtx;
    };

}

#endif // PLUGINFACTORY_P_H
