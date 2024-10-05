#include "pitchinference.h"

#include <dsinfer/private/inference_p.h>

namespace dsinfer {

    class PitchInference::Impl : public Inference::Impl {
    public:
        explicit Impl(const InferenceSpec *spec) : Inference::Impl(spec) {
        }
    };

    PitchInference::PitchInference(const InferenceSpec *spec) : Inference(*new Impl(spec)) {
    }

    PitchInference::~PitchInference() = default;

    bool PitchInference::initialize(const JsonObject &args, Error *error) {
        return false;
    }

    bool PitchInference::start(const JsonValue &input, Error *error) {
        return false;
    }

    bool PitchInference::stop() {
        return false;
    }

    Inference::State PitchInference::state() const {
        return Inference::Terminated;
    }

    JsonValue PitchInference::result() const {
        return {};
    }

}