#ifndef PLUGINFACTORY_H
#define PLUGINFACTORY_H

#include <vector>
#include <filesystem>
#include <string>

#include <dsinfer/plugin.h>

namespace dsinfer {

    class DSINFER_EXPORT PluginFactory {
    public:
        PluginFactory();
        virtual ~PluginFactory();

    public:
        void addStaticPlugin(Plugin *plugin);
        std::vector<Plugin *> staticPlugins() const;

        void addPluginPath(const std::string &iid, const std::filesystem::path &path);
        void setPluginPaths(const std::string &iid,
                            const std::vector<std::filesystem::path> &paths);
        const std::vector<std::filesystem::path> &pluginPaths(const std::string &iid) const;

    public:
        Plugin *plugin(const std::string &iid, const std::string &key) const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        explicit PluginFactory(Impl &impl);
    };

}

#endif // PLUGINFACTORY_H
