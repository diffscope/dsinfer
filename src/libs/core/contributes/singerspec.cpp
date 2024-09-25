#include "singerspec.h"

#include <utility>
#include "contributespec_p.h"

namespace fs = std::filesystem;

namespace dsinfer {

    class SingerSpec::Impl : public ContributeSpec::Impl {
    public:
        explicit Impl(LibrarySpec *parent)
            : ContributeSpec::Impl(ContributeSpec::CT_Singer, parent) {
        }

    public:
        bool read(std::filesystem::path &basePath, const dsinfer::JsonObject &obj,
                  std::string *error) override {
            return false;
        }

        std::filesystem::path path;

        std::string name;
        fs::path avatar;
        fs::path background;
        fs::path demoAudio;

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

    const std::vector<SingerImport> &SingerSpec::imports() const {
        __dsinfer_impl_t;
        return impl.imports;
    }

    std::vector<Inference *> SingerSpec::createInferences(std::string *error) const {
        __dsinfer_impl_t;

        return {};
    }

    SingerSpec::SingerSpec(LibrarySpec *parent) : ContributeSpec(*new Impl(parent)) {
    }

}