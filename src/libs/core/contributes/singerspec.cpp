#include "singerspec.h"

#include "contributespec_p.h"

namespace fs = std::filesystem;

namespace dsinfer {

    class SingerSpec::Impl : public ContributeSpec::Impl {
    public:
        Impl() : ContributeSpec::Impl(ContributeSpec::Singer) {
        }

    public:
        bool read(const std::filesystem::path &basePath, const dsinfer::JsonObject &obj,
                  Error *error) override {
            return false;
        }

        std::filesystem::path path;

        std::string name;
        std::string model;

        fs::path avatar;
        fs::path background;
        fs::path demoAudio;

        std::vector<SingerImport> imports;

        JsonObject configuration;
    };

    SingerSpec::~SingerSpec() = default;

    std::filesystem::path SingerSpec::path() const {
        __dsinfer_impl_t;
        return impl.path;
    }

    std::string SingerSpec::model() const {
        __dsinfer_impl_t;
        return impl.model;
    }

    std::string SingerSpec::name() const {
        __dsinfer_impl_t;
        return impl.name;
    }

    std::filesystem::path SingerSpec::avatar() const {
        __dsinfer_impl_t;
        return impl.avatar;
    }

    std::filesystem::path SingerSpec::background() const {
        __dsinfer_impl_t;
        return impl.background;
    }

    std::filesystem::path SingerSpec::demoAudio() const {
        __dsinfer_impl_t;
        return impl.demoAudio;
    }

    const std::vector<SingerImport> &SingerSpec::imports() const {
        __dsinfer_impl_t;
        return impl.imports;
    }

    JsonObject SingerSpec::configuration() const {
        __dsinfer_impl_t;
        return impl.configuration;
    }

    std::vector<Inference *> SingerSpec::createInferences(Error *error) const {
        __dsinfer_impl_t;
        return {};
    }

    SingerSpec::SingerSpec() : ContributeSpec(*new Impl()) {
    }

}