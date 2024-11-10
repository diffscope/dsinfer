#include "onnxtask.h"

#include <atomic>
#include <mutex>
#include <unordered_set>
#include <random>

#include <dsinfer/dsinferglobal.h>
#include <stduuid/uuid.h>

#include "onnxsession.h"
#include "onnxsession_p.h"
#include "onnxcontext.h"
#include "onnxcontext_p.h"
#include "internal/onnxdriver_logger.h"
#include "internal/valueparser.h"
#include "internal/idutil.h"

namespace dsinfer {

    static IdManager<OnnxTask> &idManager() {
        static IdManager<OnnxTask> manager;
        return manager;
    }

    static inline std::string generate_uuid() {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size>{};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        uuids::uuid_random_generator gen{generator};
        return uuids::to_string(gen());
    }

    class OnnxTask::Impl {
    public:
        class ScopedStateUpdater {
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

        int64_t taskId = 0;
        std::atomic<State> state = State::Terminated;
        OnnxSession *sessionObj = nullptr;
        OnnxContext *contextObj = nullptr;
        std::vector<JsonValue> result;
    };

    OnnxTask::OnnxTask() : _impl(std::make_unique<Impl>()) {
        __stdc_impl_t;
        auto taskId = idManager().add(this);
        impl.taskId = taskId;
        onnxdriver_log().debug("OnnxTask [%1] - new task created", taskId);
    }

    OnnxTask::~OnnxTask() {
        __stdc_impl_t;

        idManager().remove(impl.taskId);
    }

    bool OnnxTask::initialize(const JsonValue &args, Error *error) {
        __stdc_impl_t;
        impl.result.clear();
        impl.state = State::Idle;
        return true;
    }

    bool OnnxTask::start(const JsonValue &input, Error *error) {
        __stdc_impl_t;
        // When leaving the function, the state will be automatically set to Failed,
        // unless calling setTargetState.
        Impl::ScopedStateUpdater stateUpdater(&impl, State::Failed);
        impl.state = State::Running;

        if (!input.isObject()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid task input format: input value is not object");
            }
            return false;
        }
        auto obj = input.toObject();
        int64_t sessionId = 0;
        int64_t contextId = 0;
        if (auto it = obj.find("session"); it != obj.end()) {
            sessionId = it->second.toInt64();
        } else {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid task input format: \"session\" is missing");
            }
            return false;
        }
        if (auto it = obj.find("context"); it != obj.end()) {
            contextId = it->second.toInt64();
        } else {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid task input format: \"context\" is missing");
            }
            return false;
        }
        impl.sessionObj = OnnxSession::getSession(sessionId);
        impl.contextObj = OnnxContext::getContext(contextId);
        if (!impl.sessionObj) {
            if (impl.contextObj) {
                // only session does not exist
                if (error) {
                    *error = Error(Error::InvalidFormat, // TODO: error type
                                   "Session " + std::to_string(sessionId) + " does not exist");
                }
            } else {
                // both session and context do not exist
                if (error) {
                    *error = Error(Error::InvalidFormat, // TODO: error type
                                   "Session " + std::to_string(sessionId) + " and " + "Context " +
                                       std::to_string(contextId) + " do not exist");
                }
            }
            return false;
        } else if (!impl.contextObj) {
            // only context does not exist
            if (error) {
                *error = Error(Error::InvalidFormat, // TODO: error type
                               "Context " + std::to_string(contextId) + " does not exist");
            }
            return false;
        }

        if (!impl.sessionObj->isOpen()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Session " + std::to_string(sessionId) + " is not open");
            }
            return false;
        }
        auto it_input = obj.find("input");
        if ((it_input == obj.end()) || !it_input->second.isArray()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid task input format: \"input\" is missing or not an array");
            }
            return false;
        }

        auto it_output = obj.find("output");
        if ((it_output == obj.end()) || !it_output->second.isArray()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid task input format: \"output\" is missing or not an array");
            }
            return false;
        }

        auto inputArr = it_input->second.toArray();
        auto outputArr = it_output->second.toArray();

        onnxdriver::SharedValueMap valueMap;
        for (const auto &inputData : std::as_const(inputArr)) {
            auto inputDataObj = inputData.toObject();
            auto it_name = inputDataObj.find("name");
            if (it_name == inputDataObj.end() || !it_name->second.isString()) {
                if (error) {
                    *error = Error(Error::InvalidFormat, "Invalid task input format: \"name\" in "
                                                         "the input data is missing or not string");
                }
                return false;
            }
            if (checkStringValue(inputDataObj, "format", "reference")) {
                if (auto it_content = inputDataObj.find("data"); it_content != inputDataObj.end()) {
                    auto key = it_content->second["value"].toString();
                    auto ortValueObj = impl.contextObj->_impl->getOrtValue(key);
                    if (!ortValueObj) {
                        if (error) {
                            *error = Error(Error::InvalidFormat, // TODO: error type
                                           "Referenced value not found using key " + key);
                        }
                        return false;
                    }
                    auto inputName = it_name->second.toString();
                    if (inputName.empty()) {
                        if (error) {
                            *error = Error(Error::InvalidFormat, "Input name is empty");
                        }
                        return false;
                    }
                    valueMap[inputName] = ortValueObj;
                } else {
                    if (error) {
                        *error =
                            Error(Error::InvalidFormat,
                                  R"(Please specify key in object["data"]["value"] string field.)");
                    }
                    return false;
                }
            } else {
                auto inputValue = onnxdriver::parseInputContent(inputDataObj, error);
                if (!inputValue) {
                    return false;
                }
                valueMap[it_name->second.toString()] =
                    onnxdriver::makeSharedValue(std::move(inputValue));
            }
        }

        auto sessionResult = impl.sessionObj->_impl->session.run(valueMap, error);

        if (sessionResult.empty()) {
            return false;
        }

        for (const auto &outputData : std::as_const(outputArr)) {
            auto outputDataObj = outputData.toObject();
            auto name = outputDataObj["name"].toString();
            if (auto it = sessionResult.find(name); it != sessionResult.end()) {
                auto format = outputDataObj["format"].toString();
                if (format == "bytes") {
                    Error err_;
                    auto jVal = onnxdriver::serializeTensorAsBytes(*it->second, &err_);
                    if (!err_.ok()) {
                        if (error) {
                            *error = std::move(err_);
                        }
                        return false;
                    }
                    impl.result.emplace_back(JsonObject{
                        {"name",   name   },
                        {"format", "bytes"},
                        {"data",   jVal   }
                    });
                } else if (format == "array") {
                    Error err_;
                    auto jVal = onnxdriver::serializeTensorAsArray(*it->second, &err_);
                    if (!err_.ok()) {
                        if (error) {
                            *error = std::move(err_);
                        }
                        return false;
                    }
                    impl.result.emplace_back(JsonObject{
                        {"name",   name   },
                        {"format", "array"},
                        {"data",   jVal   }
                    });
                } else if (format == "reference") {
                    auto uuidKey = generate_uuid();
                    impl.contextObj->_impl->insertOrtValue(uuidKey, it->second);
                    impl.result.emplace_back(JsonObject{
                        {"name",   name                          },
                        {"format", "reference"                   },
                        {"data",   JsonObject{{"value", uuidKey}}}
                    });
                }
            } else {
                if (error) {
                    *error = Error(Error::InvalidFormat,
                                   "output name \"" + name + "\" is not found in onnx output");
                }
                return false;
            }
        }
        stateUpdater.setTargetState(State::Idle);
        return true;
    }

    bool OnnxTask::startAsync(const JsonValue &input,
                              const std::function<void(const JsonValue &)> &callback,
                              Error *error) {
        return false;
    }

    bool OnnxTask::stop(Error *error) {
        __stdc_impl_t;
        if (!impl.sessionObj) {
            return false;
        }
        impl.sessionObj->_impl->session.terminate();
        impl.state = State::Terminated;
        return true;
    }

    int64_t OnnxTask::id() const {
        __stdc_impl_t;
        return impl.taskId;
    }

    InferenceTask::State OnnxTask::state() const {
        __stdc_impl_t;
        return impl.state;
    }

    JsonValue OnnxTask::result() const {
        __stdc_impl_t;
        return JsonArray(impl.result);
    }

}