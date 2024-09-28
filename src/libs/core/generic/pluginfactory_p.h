#ifndef PLUGINFACTORY_P_H
#define PLUGINFACTORY_P_H

#include <set>
#include <shared_mutex>
#include <unordered_map>

#include <dsinfer/pluginfactory.h>
#include <dsinfer/sharedlibrary.h>

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
        mutable std::unordered_map<std::filesystem::path, SharedLibrary *> libraryInstances;
        mutable std::set<std::string> pluginsDirty;
        mutable std::unordered_map<std::string, std::unordered_map<std::string, Plugin *>>
            allPlugins;
        mutable std::shared_mutex plugins_mtx;
    };

}

#endif // PLUGINFACTORY_P_H
