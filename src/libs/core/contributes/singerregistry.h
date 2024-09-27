#ifndef SINGERREGISTRY_H
#define SINGERREGISTRY_H

#include <dsinfer/contributeregistry.h>
#include <dsinfer/singerspec.h>

namespace dsinfer {

    class DSINFER_EXPORT SingerRegistry : public ContributeRegistry {
    public:
        ~SingerRegistry();

    public:
        SingerSpec *findSinger(const std::string &id, const VersionNumber &version = {}) const;
        std::vector<SingerSpec *> findSingers(const std::string &id) const;
        std::vector<SingerSpec *> singers() const;

    protected:
        std::string specKey() const override;
        ContributeSpec *parseSpec(const std::filesystem::path &basePath, const JsonValue &config,
                                  Error *error) const override;
        bool loadSpec(ContributeSpec *spec, ContributeSpec::State state, Error *error) override;

    protected:
        class Impl;
        SingerRegistry(Environment *env);

        friend class Environment;
    };

}

#endif // SINGERREGISTRY_H
