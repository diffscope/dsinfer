#ifndef INFERENCEREGISTRY_H
#define INFERENCEREGISTRY_H

#include <dsinfer/contributeregistry.h>
#include <dsinfer/inferencedriver.h>
#include <dsinfer/inferencespec.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceRegistry : public ContributeRegistry {
    public:
        ~InferenceRegistry();

    public:
        InferenceSpec *findInference(const std::string &id,
                                     const VersionNumber &version = {}) const;
        std::vector<InferenceSpec *> findInferences(const std::string &id) const;
        std::vector<InferenceSpec *> inferences() const;

        InferenceDriver *driver() const;
        InferenceDriver *takeDriver();
        void setDriver(InferenceDriver *driver);

    protected:
        std::string specKey() const override;
        ContributeSpec *parseSpec(const std::filesystem::path &basePath, const JsonValue &config,
                                  Error *error) const override;
        bool loadSpec(ContributeSpec *spec, ContributeSpec::State state, Error *error) override;

    protected:
        class Impl;
        InferenceRegistry(Environment *env);

        friend class Environment;
    };

}

#endif // INFERENCEREGISTRY_H
