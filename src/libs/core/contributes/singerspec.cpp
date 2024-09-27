#include "singerspec.h"

#include <utility>
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
        fs::path avatar;
        fs::path background;
        fs::path demoAudio;
        fs::path dictionary;

        std::vector<SingerImport> imports;

        JsonObject schema;
        JsonObject configuration;
    };

    SingerSpec::~SingerSpec() = default;

    std::filesystem::path SingerSpec::path() const {
        __dsinfer_impl_t;
        return impl.path;
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

    std::filesystem::path SingerSpec::dictionary() const {
        __dsinfer_impl_t;
        return impl.dictionary;
    }

    const std::vector<SingerImport> &SingerSpec::imports() const {
        __dsinfer_impl_t;
        return impl.imports;
    }

    std::vector<Inference *> SingerSpec::createInferences(Error *error) const {
        __dsinfer_impl_t;
        return {};
    }

    SingerSpec::SingerSpec() : ContributeSpec(*new Impl()) {
    }

}