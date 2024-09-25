#ifndef PLUGINFACTORY_P_H
#define PLUGINFACTORY_P_H

#include <map>
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
        void scanPlugins(const std::string &iid) const;

        std::map<std::string, std::vector<std::filesystem::path>> pluginPaths;
        std::vector<Plugin *> staticPlugins;
        mutable std::set<std::string> pluginsDirty;
        mutable std::map<std::string, std::map<std::string, Plugin *>> allPlugins;
        mutable std::shared_mutex plugins_mtx;
    };

}

#endif // PLUGINFACTORY_P_H
