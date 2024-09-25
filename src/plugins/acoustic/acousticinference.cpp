#include "acousticinference.h"

#include <dsinfer/private/inference_p.h>

namespace dsinfer {

    class AcousticInference::Impl : public Inference::Impl {
    public:
        explicit Impl(Environment *env) : Inference::Impl(env) {
        }
    };

    AcousticInference::AcousticInference(Environment *env) : Inference(*new Impl(env)) {
    }

    AcousticInference::~AcousticInference() = default;

    bool AcousticInference::initialize(const JsonObject &args, std::string *error) {
        return false;
    }

    bool AcousticInference::start(const JsonValue &input, std::string *error) {
        return false;
    }

    bool AcousticInference::stop() {
        return false;
    }

    Inference::State AcousticInference::state() const {
        return Inference::Terminated;
    }

    JsonObject AcousticInference::result() const {
        return {};
    }

}