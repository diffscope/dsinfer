#include "acousticinference.h"

#include <random>

#include <dsinfer/private/inference_p.h>

#include <stduuid/uuid.h>

#include "acoustic_logger.h"

namespace dsinfer {

    static std::string generate_uuid() {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        auto id1 = uuids::uuid_random_generator{generator}();
        return uuids::to_string(id1);
    }

    class AcousticInference::Impl : public Inference::Impl {
    public:
        explicit Impl(const InferenceSpec *spec) : Inference::Impl(spec) {
        }
    };

    AcousticInference::AcousticInference(const InferenceSpec *spec) : Inference(*new Impl(spec)) {
    }

    AcousticInference::~AcousticInference() = default;

    bool AcousticInference::initialize(const JsonValue &args, Error *error) {
        return false;
    }

    bool AcousticInference::start(const JsonValue &input, Error *error) {

        // TODO: store the mel-freq tensor with UUID
        auto uuid = generate_uuid();
        acoustic_log().info("UUID: %1", uuid);

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