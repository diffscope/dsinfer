#include "singerregistry.h"
#include "contributeregistry_p.h"

namespace dsinfer {

    class SingerRegistry::Impl : public ContributeRegistry::Impl {
    public:
        explicit Impl(Environment *env) : ContributeRegistry::Impl(ContributeSpec::Singer, env) {
        }
    };

    SingerRegistry::~SingerRegistry() = default;

    SingerSpec *SingerRegistry::findSinger(const std::string &id,
                                           const VersionNumber &version) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        // TODO
        return nullptr;
    }

    std::vector<SingerSpec *> SingerRegistry::findSingers(const std::string &id) const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        // TODO
        return {};
    }

    std::vector<SingerSpec *> SingerRegistry::singers() const {
        __dsinfer_impl_t;
        std::shared_lock<std::shared_mutex> lock(impl.env_mtx());
        // TODO
        return {};
    }

    std::string SingerRegistry::specKey() const {
        static std::string _key("singers");
        return _key;
    }

    ContributeSpec *SingerRegistry::parseSpec(const std::filesystem::path &basePath,
                                              const JsonObject &config, Error *error) const {
        __dsinfer_impl_t;
        // TODO
        return nullptr;
    }

    void SingerRegistry::unload(dsinfer::ContributeSpec *spec) {
        __dsinfer_impl_t;
        // TODO
    }

    SingerRegistry::SingerRegistry(Environment *env) : ContributeRegistry(*new Impl(env)) {
    }

}