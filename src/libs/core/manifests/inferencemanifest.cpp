#include "inferencemanifest.h"

#include "librarymanifestbase_p.h"
#include "dsinfer_pimpl.h"

namespace dsinfer {

    InferenceInfo::InferenceInfo() {
    }
    InferenceInfo::~InferenceInfo() {
    }
    std::filesystem::path InferenceInfo::path() const {
        return std::filesystem::path();
    }
    std::string InferenceInfo::id() const {
        return std::string();
    }
    std::string InferenceInfo::className() const {
        return std::string();
    }
    int InferenceInfo::level() const {
        return 0;
    }
    JsonObject InferenceInfo::arguments() const {
        return JsonObject();
    }
    JsonObject InferenceInfo::internalAttributes() const {
        return JsonObject();
    }
    InferenceInfo InferenceInfo::read(const std::filesystem::path &path) {
        return InferenceInfo();
    }

    class InferenceManifest::Impl : public LibraryManifestBase::Impl {
    public:
        Impl(Type type, const std::filesystem::path &path) : LibraryManifestBase::Impl(type, path) {
        }

        std::vector<InferenceInfo> inferences;
    };

    InferenceManifest::InferenceManifest(const std::filesystem::path &path)
        : LibraryManifestBase(new Impl(Inference, path)) {
    }

    std::vector<InferenceInfo> InferenceManifest::inferences() const {
        __impl_t;
        return impl.inferences;
    }

}