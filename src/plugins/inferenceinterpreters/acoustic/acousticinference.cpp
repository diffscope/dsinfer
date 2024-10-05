#include "acousticinference.h"

#include <dsinfer/private/inference_p.h>

namespace dsinfer {

    class AcousticInference::Impl : public Inference::Impl {
    public:
        explicit Impl(const InferenceSpec *spec) : Inference::Impl(spec) {
        }
    };

    AcousticInference::AcousticInference(const InferenceSpec *spec) : Inference(*new Impl(spec)) {
    }

    AcousticInference::~AcousticInference() = default;

    bool AcousticInference::initialize(const JsonObject &args, Error *error) {
        return false;
    }

    bool AcousticInference::start(const JsonValue &input, Error *error) {
        return false;
    }

    bool AcousticInference::stop() {
        return false;
    }

    Inference::State AcousticInference::state() const {
        return Inference::Terminated;
    }

    JsonValue AcousticInference::result() const {
        return {};
    }

}