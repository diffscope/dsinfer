#ifndef SINGERREGISTRY_H
#define SINGERREGISTRY_H

#include <dsinfer/contributeregistry.h>
#include <dsinfer/singerspec.h>

namespace dsinfer {

    class DSINFER_EXPORT SingerRegistry : public ContributeRegistry {
    public:
        ~SingerRegistry();

    public:
        std::vector<SingerSpec *> findSingers(const ContributeIdentifier &identifier) const;
        std::vector<SingerSpec *> singers() const;

    protected:
        std::string specKey() const override;
        ContributeSpec *parseSpec(const std::filesystem::path &basePath, const JsonValue &config,
                                  Error *error) const override;
        bool loadSpec(ContributeSpec *spec, ContributeSpec::State state, Error *error) override;

    protected:
        class Impl;
        explicit SingerRegistry(Environment *env);

        friend class Environment;
    };

}

#endif // SINGERREGISTRY_H
