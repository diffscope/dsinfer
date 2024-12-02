#include "acousticinference.h"

#include <filesystem>
#include <random>

#include <dsinfer/private/inference_p.h>
#include <dsinfer/environment.h>
#include <dsinfer/inferencespec.h>
#include <dsinfer/inferencecontext.h>
#include <dsinfer/inferencesession.h>
#include <dsinfer/inferenceregistry.h>
#include <dsinfer/contributeregistry.h>
#include <dsinfer/inferencedriver.h>
#include <nlohmann/detail/input/binary_reader.hpp>
#include <stdcorelib/path.h>

#include <stduuid/uuid.h>

#include "acoustic_logger.h"
#include "preprocess.h"
#include "internal/project.h"
#include "internal/json_utils.h"
#include "internal/json_serializer.h"

namespace dsinfer {

    static const std::string acousticConfigClass = "acoustic configuration";

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
        class ScopedStateUpdater;

        explicit Impl(const InferenceSpec *spec) : Inference::Impl(spec) {
        }

        bool useCpuHint = false;
        float depth = 1.0f;
        std::atomic<State> state = State::Terminated;
        int64_t steps = 20;
        InferenceDriver *driver = nullptr;
        std::unique_ptr<InferenceSession> session;
        std::unique_ptr<InferenceTask> task;
        JsonValue result;
    };

    class AcousticInference::Impl::ScopedStateUpdater {
    public:
        inline explicit ScopedStateUpdater(Impl *impl, State targetState = State::Failed)
            : m_impl(impl), m_targetState(targetState) {
        }

        inline ~ScopedStateUpdater() {
            m_impl->state = m_targetState;
        }

        inline void setTargetState(State targetState) {
            m_targetState = targetState;
        }

    private:
        Impl *m_impl;
        State m_targetState;
    };

    AcousticInference::AcousticInference(const InferenceSpec *spec) : Inference(*new Impl(spec)) {
    }

    AcousticInference::~AcousticInference() = default;

    bool AcousticInference::initialize(const JsonValue &args, Error *error) {
        __stdc_impl_t;
        if (!(args.isUndefined() || args.isNull() || args.isObject())) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                    "Invalid format of AcousticInference::initialize: json value is not object");
            }
            return false;
        }

        const auto spec = impl.spec;
        if (!spec) {
            if (error) {
                *error = Error(Error::SessionError, "Inference spec is null");
            }
            return false;
        }

        const auto env = spec->env();
        if (!env) {
            if (error) {
                *error = Error(Error::SessionError, "Environment is null");
            }
            return false;
        }

        const auto inferenceReg = env->registry(ContributeSpec::Inference)->cast<InferenceRegistry>();
        if (!inferenceReg) {
            if (error) {
                *error = Error(Error::SessionError, "Inference registry is null");
            }
            return false;
        }

        impl.driver = inferenceReg->driver();
        if (!impl.driver) {
            if (error) {
                // TODO: error type
                *error = Error(Error::LibraryNotFound,
                    "Inference driver is not loaded");
            }
            return false;
        }

        impl.useCpuHint = args["useCpuHint"].toBool(false);
        impl.steps = args["steps"].toInt64(impl.steps);
        impl.depth = static_cast<float>(args["depth"].toDouble(impl.depth));

        impl.session = std::unique_ptr<InferenceSession>(impl.driver->createSession());
        if (!impl.session) {
            if (error) {
                *error = Error(Error::SessionError, "Failed to create session");
            }
            return false;
        }

        std::string model;
        if (!get_input(acousticConfigClass, spec->configuration(), "model", error, model)) {
            return false;
        }
        const auto modelPath = spec->path() / stdc::path::from_utf8(model);
        if (!impl.session->open(modelPath, JsonObject{{"useCpuHint", impl.useCpuHint}}, error)) {
            return false;
        }

        impl.task = std::unique_ptr<InferenceTask>(impl.driver->createTask());
        if (!impl.task) {
            if (error) {
                *error = Error(Error::SessionError, "Failed to create task");
            }
            return false;
        }
        return true;
    }

    bool AcousticInference::start(const JsonValue &input, Error *error) {
        __stdc_impl_t;

        Impl::ScopedStateUpdater stateUpdater(&impl, State::Failed);
        impl.state = State::Running;

        const auto spec = impl.spec;
        if (!spec) {
            if (error) {
                *error = Error(Error::SessionError, "Inference spec is null");
            }
            return false;
        }

        if (!impl.driver) {
            if (error) {
                // TODO: error type
                *error = Error(Error::LibraryNotFound,
                    "Inference driver is not loaded");
            }
            return false;
        }

        if (!impl.session) {
            if (error) {
                *error = Error(Error::SessionError, "Inference session is not created");
            }
            return false;
        }

        if (!impl.session->isOpen()) {
            if (error) {
                *error = Error(Error::SessionError, "Inference session is not open");
            }
            return false;
        }

        if (!impl.task) {
            if (error) {
                *error = Error(Error::SessionError, "Inference task is not created");
            }
            return false;
        }

        const auto config = spec->configuration();
        const auto schema = spec->schema();

        // TODO: process input and run inference
        dsinterp::Segment segment;
        if (!dsinterp::from_json(input, segment, error)) {
            return false;
        }

        int sampleRate;
        if (!get_input(acousticConfigClass, config, "sampleRate", error, sampleRate)) {
            return false;
        }
        if (sampleRate == 0) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of " + acousticConfigClass + ": Invalid sampleRate");
            }
            return false;
        }

        int hopSize;
        if (!get_input(acousticConfigClass, config, "hopSize", error, hopSize)) {
            return false;
        }
        if (hopSize == 0) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Invalid format of " + acousticConfigClass + ": Invalid hopSize");
            }
            return false;
        }

        double frameLength = static_cast<double>(hopSize) / static_cast<double>(sampleRate);

        JsonArray inputParams;

        auto readFileOrObject = [&](const std::string &key, std::unordered_map<std::string, int64_t> &outMap, Error *error_) -> bool {
            const auto it = config.find(key);
            if (it == config.end()) {
                if (error_) {
                    *error_ = Error(Error::InvalidFormat, "Invalid format of " + acousticConfigClass +
                             ": missing key \"" + key + "\" of string (filename) or object");
                }
                return false;
            }
            const auto &j = it->second;
            if (j.isString()) {
                std::filesystem::path path = spec->path() / stdc::path::from_utf8(j.toString());
                if (!dsinterp::readJsonFileHelper(path, key, outMap, error_)) {
                    return false;
                }
            } else if (j.isObject()) {
                if (!dsinterp::readObjectHelper(j.toObject(), key, outMap, error_)) {
                    return false;
                }
            } else {
                if (error_) {
                    *error_ = Error(Error::InvalidFormat, "Invalid format of " + key +
                                    ": must be string (filename) or object");
                }
                return false;
            }
            return true;
        };


        // onnx input value: tokens
        std::unordered_map<std::string, int64_t> phonemesMap;
        if (!readFileOrObject("phonemes", phonemesMap, error)) {
            return false;
        }
        inputParams.push_back(dsinterp::parsePhonemeTokens(segment, phonemesMap));

        // onnx input value: languages
        bool useLangId = false;
        if (const auto it = config.find("useLangId"); it != config.end()) {
            useLangId = it->second.toBool(false);
        }
        if (useLangId) {
            std::unordered_map<std::string, int64_t> langIdMap;
            if (!readFileOrObject("languages", langIdMap, error)) {
                return false;
            }
            inputParams.push_back(dsinterp::parsePhonemeLanguages(segment, langIdMap));
        }

        // onnx input value: durations
        int64_t targetLength = 0;
        inputParams.push_back(dsinterp::parsePhonemeDurations(segment, frameLength, targetLength));

        if (targetLength == 0) {
            if (error) {
                *error = Error(Error::InvalidFormat, "Target length is 0");
            }
            return false;
        }

        // join the array to a string
        auto join = [](const auto &vec, const std::string &delimiter) -> std::string {
            std::ostringstream oss;
            for (size_t i = 0; i < vec.size(); ++i) {
                oss << vec[i];
                if (i < vec.size() - 1) {
                    oss << delimiter;
                }
            }
            return oss.str();
        };

        // onnx input value: variance parameters
        // (`targetLength` depends on `durations` calculation)
        if (const auto it1 = schema.find("varianceControls"); it1 != schema.end()) {
            std::vector<std::string> missingParameters;
            auto items = it1->second.toArray();
            for (const auto &item: items) {
                auto paramName = item.toString();
                if (paramName.empty()) {
                    continue;
                }
                if (auto value = dsinterp::parseVarianceParameter(segment, paramName.c_str(), paramName.c_str(), frameLength,
                                                     targetLength); !value.isUndefined()) {
                    inputParams.push_back(std::move(value));
                } else {
                    missingParameters.push_back(std::move(paramName));
                }
            }
            if (!missingParameters.empty()) {
                std::stringstream ss;
                for (const auto &missingParameter: missingParameters) {
                    ss << missingParameter << ",";
                }
                if (error) {
                    *error = Error(Error::InvalidFormat,
                        "Missing variance parameters: " + join(missingParameters, ", "));
                }
                return false;
            }
        }

        // onnx input value: transition parameters
        // (`targetLength` depends on `durations` calculation)
        if (const auto it1 = schema.find("transitionControls"); it1 != schema.end()) {
            // for transition parameters, missing values are allowed
            // because they can be filled with default values
            auto items = it1->second.toArray();
            for (const auto &item: items) {
                auto paramName = item.toString();
                if (paramName.empty()) {
                    continue;
                }
                float defaultValue = 0;
                if (paramName == "velocity") {
                    defaultValue = 1;
                }
                inputParams.push_back(dsinterp::parseTransitionParameter(
                    segment, paramName.c_str(), paramName.c_str(), defaultValue, frameLength, targetLength));
            }
        }

        // onnx input value: spk_embed
        // (`targetLength` depends on `durations` calculation)
        bool useSpeakerEmbedding = false;
        if (const auto it1 = config.find("useSpeakerEmbedding"); it1 != config.end()) {
            useSpeakerEmbedding = it1->second.toBool(false);
        }
        if (useSpeakerEmbedding) {
            if (const auto it1 = config.find("speakers"); it1 != config.end()) {
                std::vector<dsinterp::SpeakerPair> speakerPairs;
                std::vector<std::string> speakers;
                if (!it1->second.isObject()) {
                    if (error) {
                        *error = Error(Error::InvalidFormat,
                        "Invalid format of " + acousticConfigClass + ": \"speakers\" must be object type");
                    }
                    return false;
                }
                const auto object = it1->second.toObject();
                speakerPairs.reserve(object.size());
                speakers.reserve(object.size());
                for (const auto &[key, val]: object) {
                    speakerPairs.emplace_back(key, stdc::path::from_utf8(val.toString()));
                    speakers.emplace_back(key);
                }
                dsinterp::SpeakerEmbed spkEmb;
                if (!spkEmb.loadSpeakers(speakerPairs)) {
                    if (error) {
                        *error = Error(Error::InvalidFormat,
                            "Could not load speakers!");
                    }
                    return false;
                }
                inputParams.push_back(dsinterp::parseSpeakerMix(spkEmb, speakers, segment.speakers, frameLength, targetLength));
            } else {
                if (error) {
                    *error = Error(Error::InvalidFormat, "useSpeakerEmbedding is true, but could not find \"speakers\"");
                }
                return false;
            }
        } // if (useSpeakerEmbedding)

        // onnx input value: steps/speedup
        int64_t stepsOrSpeedup = input["steps"].toInt64(impl.steps);
        bool useContAccel = false;
        if (const auto it1 = config.find("useContinuousAcceleration"); it1 != config.end()) {
            useContAccel = it1->second.toBool(false);
        }
        if (useContAccel) {
            inputParams.push_back(dsinterp::create_tensor_from_scalar<int64_t>("steps", stepsOrSpeedup));
        } else {
            stepsOrSpeedup = dsinterp::getSpeedupFromSteps(stepsOrSpeedup);
            inputParams.push_back(dsinterp::create_tensor_from_scalar<int64_t>("speedup", stepsOrSpeedup));
        }

        // onnx input value: depth
        // (if not `useContinuousAcceleration`, this value depends on `speedup` calculation)
        bool useVariableDepth = false;
        if (const auto it1 = config.find("useVariableDepth"); it1 != config.end()) {
            useVariableDepth = it1->second.toBool(false);
        }

        // If found "depth" in input, use it if valid. Otherwise, use the depth specified in initialization
        float depth = static_cast<float>(input["depth"].toDouble(impl.depth));
        if (useVariableDepth) {
            if (useContAccel) {
                if (const auto it1 = config.find("maxDepth");
                    it1 != config.end() && (it1->second.isDouble() || it1->second.isInt())) {
                    depth = (std::min)(static_cast<float>(it1->second.toDouble(depth)), depth);
                } else {
                    if (error) {
                        *error = Error(Error::InvalidFormat, "maxDepth is not set or not a floating point number");
                    }
                    return false;
                }
                inputParams.push_back(dsinterp::create_tensor_from_scalar<float>("depth", depth));
            } else {
                int64_t maxDepth;
                if (const auto it1 = config.find("maxDepth"); it1 != config.end() && it1->second.isInt()) {
                    maxDepth = it1->second.toInt64();
                } else {
                    if (error) {
                        *error = Error(Error::InvalidFormat, "maxDepth is not set or not an integer");
                    }
                    return false;
                }
                inputParams.push_back(
                    dsinterp::create_tensor_from_scalar<int64_t>(
                        "depth", dsinterp::getIntDepth(depth, maxDepth, stepsOrSpeedup)));
            }
        }

        // onnx input value: f0
        // (`targetLength` depends on `durations` calculation)
        auto f0 = dsinterp::parseF0AsVector(segment, frameLength, targetLength);
        inputParams.push_back(dsinterp::parseF0(f0));

        // TODO: store the mel-freq tensor with UUID
        auto uuid = generate_uuid();
        acoustic_log().info("UUID: %1", uuid);

        bool ok = impl.task->start(JsonObject{
                             {"session", impl.session->id()},
                             {"context", segment.context},
                             {"input", std::move(inputParams)},
                             {
                                 "output", JsonArray{
                                     JsonObject{
                                         {"name", "mel"}, {"format", "reference"}
                                     }
                                 }
                             },
                         }, error);
        if (!ok) {
            return false;
        }
        auto result = impl.task->result();
        auto mel = result[0]["data"]["value"].toString();
        if (mel.empty()) {
            if (error) {
                *error = Error(Error::SessionError, "Inference failed: could not get mel");
            }
            return false;
        }

        uint8_t *f0_bytes = reinterpret_cast<uint8_t *>(f0.data());
        impl.result = JsonObject{
            {"mel", std::move(mel)},
            {"f0", std::vector<uint8_t>(f0_bytes, f0_bytes + f0.size() * sizeof(float))},
        };

        stateUpdater.setTargetState(State::Idle);
        return true;
    }

    bool AcousticInference::startAsync(const JsonValue &input,
                                       const std::function<void(const JsonValue &, const Error &)> &callback,
                                       Error *error) {
        return false;
    }

    bool AcousticInference::stop() {
        __stdc_impl_t;
        Error error;
        if (impl.task->stop(&error)) {
            impl.state = State::Terminated;
            return true;
        }
        return false;
    }

    Inference::State AcousticInference::state() const {
        __stdc_impl_t;
        return impl.state;
    }

    JsonValue AcousticInference::result() const {
        __stdc_impl_t;
        return impl.result;
    }

}