#ifndef DSINFER_ENVIRONMENT_H
#define DSINFER_ENVIRONMENT_H

#include <map>
#include <filesystem>

#include <dsinfer/sessionmanager.h>
#include <dsinfer/inference.h>

#define dsEnv (dsinfer::Environment::instance())

namespace dsinfer {

    class DSINFER_EXPORT Environment {
    public:
        Environment();
        ~Environment();

        static Environment *instance();

    public:
        void load(const std::filesystem::path &path, ExecutionProvider ep, int deviceIndex);
        bool isLoaded() const;
        int deviceIndex() const;
        void setDeviceIndex(int index);

        SessionManager *sessionManager();
        std::filesystem::path libraryPath() const;
        ExecutionProvider executionProvider() const;
        std::string versionString() const;

        void addFeatureInfo(const std::filesystem::path &path,
                            const std::map<std::string, std::string> &env);
        void removeFeatureInfo(const std::string &id);

        Inference *createInference(const std::string &id, const std::string &args);

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        DSINFER_DISABLE_COPY(Environment)
    };

} // namespace dsinfer

#endif // DSINFER_ENVIRONMENT_H
