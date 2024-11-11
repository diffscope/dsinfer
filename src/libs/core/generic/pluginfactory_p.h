#ifndef DSINFER_PLUGINFACTORY_P_H
#define DSINFER_PLUGINFACTORY_P_H

#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include <stdcorelib/library.h>

#include <dsinfer/pluginfactory.h>

namespace dsinfer {

    class DSINFER_EXPORT PluginFactory::Impl {
    public:
        explicit Impl(PluginFactory *decl);
        virtual ~Impl();

        using Decl = PluginFactory;
        PluginFactory *_decl;

    public:
        void scanPlugins(const char *iid) const;

        std::unordered_map<std::string, std::vector<std::filesystem::path>> pluginPaths;
        std::unordered_set<Plugin *> staticPlugins;
        mutable std::unordered_map<std::filesystem::path::string_type, stdc::Library *>
            libraryInstances;
        mutable std::unordered_set<std::string> pluginsDirty;
        mutable std::unordered_map<std::string, std::unordered_map<std::string, Plugin *>>
            allPlugins;
        mutable std::shared_mutex plugins_mtx;
    };

}

#endif // DSINFER_PLUGINFACTORY_P_H
