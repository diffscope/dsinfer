#include "session.h"
#include "session_p.h"
#include "executionprovider_p.h"

#include <chrono>
#include <cstdio>
#include <sstream>
#include <unordered_set>
#include "env.h"
#include "logger.h"

namespace fs = std::filesystem;

namespace dsinfer {
    namespace onnxdriver {

        SessionSystem *SessionSystem::instance() {
            static SessionSystem _instance;
            return &_instance;
        }

        Session::Session() : _impl(std::make_unique<Impl>()) {
        }

        Session::~Session() {
            close();
        }

        Session::Session(Session &&other) noexcept {
            std::swap(_impl, other._impl);
        }

        Session &Session::operator=(Session &&other) noexcept {
            if (this == &other) {
                return *this;
            }
            std::swap(_impl, other._impl);
            return *this;
        }

        bool Session::open(const fs::path &path, bool preferCpu, std::string *errorMessage) {
            if (!_impl) {
                return false;
            }
            auto &impl = *_impl;
            // TODO: If the same session is already opened before, preferCpu will have no effect
            //       due to SessionSystem will return the existing SessionImage instead creating a new one. Should this be the desired behavior, or it needs to be fixed?

            LOG_DEBUG("[onnxdriver] Session - Try open " + path.string());
            fs::path canonicalPath;
            try {
                canonicalPath = fs::canonical(path);
                LOG_DEBUG("[onnxdriver] Session - The canonical path is " + canonicalPath.string());
            } catch (const std::exception &e) {
                if (errorMessage) {
                    *errorMessage = e.what();
                }
                return false;
            }

            if (!fs::is_regular_file(canonicalPath)) {
                if (errorMessage) {
                    *errorMessage = "Not a regular file";
                }
                return false;
            }

            auto mgr = SessionSystem::instance();
            auto it = mgr->sessionImageMap.find(canonicalPath);
            if (it == mgr->sessionImageMap.end()) {
                LOG_DEBUG(
                    "[onnxdriver] Session - The session image does not exist. Creating a new one...");
                impl.image = SessionImage::create(path, preferCpu, errorMessage);
            } else {
                LOG_DEBUG("[onnxdriver] Session - The session image already exists. Increasing the reference count...");
                impl.image = it->second;
                impl.image->ref();
            }

            return impl.image != nullptr;
        }

        bool Session::close() {
            if (!_impl) {
                return false;
            }
            auto &impl = *_impl;
            LOG_DEBUG("[onnxdriver] Session [%1] - close", path().filename());
            if (!impl.image)
                return false;

            if (impl.image->deref() == 0) {
                impl.image = nullptr;
            }
            return true;
        }

        fs::path Session::path() const {
            if (!_impl) {
                return {};
            }
            auto &impl = *_impl;
            return impl.image ? impl.image->path : fs::path();
        }

        bool Session::isOpen() const {
            if (!_impl) {
                return false;
            }
            auto &impl = *_impl;
            return impl.image != nullptr;
        }

        Session::State Session::state() const {
            if (!_impl) {
                return Session::State::Idle;
            }
            auto &impl = *_impl;
            return impl.state.load();
        }

        //template <typename T>
        //inline Ort::Value createOrtValueHelper(Tensor &tensor, const Ort::MemoryInfo &memoryInfo) {
        //    T *dataBuffer;
        //    auto dataSize = tensor.getDataBuffer<T>(&dataBuffer);
        //    return Ort::Value::CreateTensor<T>(memoryInfo, dataBuffer, dataSize,
        //                                       tensor.shape.data(), tensor.shape.size());
        //}

        ValueMap SessionRunHelper_Value(
                SessionImage *image,
                Ort::RunOptions &runOptions,
                                        ValueMap &inputTensorMap,
                std::atomic<Session::State> &state,
                std::string *errorMessage) {

            //static_assert(std::is_same_v<ValueMapType, ValueMap> ||
            //                  std::is_same_v<ValueMapType, ValueRefMap>,
            //              "ValueMapType should be ValueMap or ValueRefMap");

            state.store(Session::State::Running);
            auto filename = image ? image->path.filename() : "";
            LOG_INFO("[onnxdriver] Session [%1] - Running inference", filename);
            auto timeStart = std::chrono::steady_clock::now();

            if (!image) {
                if (errorMessage) {
                    *errorMessage = "Session is not open";
                }
                state.store(Session::State::Failed);
                return {};
            }

            if (inputTensorMap.empty()) {
                if (errorMessage) {
                    *errorMessage = "Input map is empty";
                }
                state.store(Session::State::Failed);
                return {};
            }

            const auto &requiredInputNames = image->inputNames;
            std::ostringstream msgStream;
            msgStream << '[' << filename << ']' << ' ';

            // Check for missing and extra input names. If found, return empty map and the error message.
            {
                bool flagMissing = false;
                // Check for missing input names

                for (const auto &requiredInputName : requiredInputNames) {
                    if (inputTensorMap.find(requiredInputName) == inputTensorMap.end()) {
                        if (flagMissing) {
                            // It isn't the first missing input name. Append a comma separator.
                            msgStream << ',' << ' ';
                        } else {
                            // It's the first missing input name. Append the message intro.
                            msgStream << "Missing input name(s): ";
                            flagMissing = true;
                        }
                        msgStream << '"' << requiredInputName << '"';
                    }
                }

                // Check for extra input names
                bool flagExtra = false;
                std::unordered_set<std::string> requiredSet(requiredInputNames.begin(),
                                                            requiredInputNames.end());
                for (auto &it : std::as_const(inputTensorMap)) {
                    auto &actualInputName = it.first;
                    if (requiredSet.find(actualInputName) == requiredSet.end()) {
                        if (flagExtra) {
                            msgStream << ',' << ' ';
                        } else {
                            if (flagMissing) {
                                msgStream << ';' << ' ';
                            }
                            msgStream << "Extra input names(s): ";
                            flagExtra = true;
                        }
                        msgStream << '"' << actualInputName << '"';
                    }
                }

                if (flagMissing || flagExtra) {
                    if (errorMessage) {
                        *errorMessage = msgStream.str();
                    }
                    state.store(Session::State::Failed);
                    return {};
                }
            }

            try {
                auto memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

                Ort::IoBinding binding(image->session);
                for (auto &[name, tensor] : inputTensorMap) {
                    binding.BindInput(name.c_str(), *tensor);
                }

                const auto &outputNames = image->outputNames;
                for (const auto &name : outputNames) {
                    binding.BindOutput(name.c_str(), memInfo);
                }

                runOptions.UnsetTerminate();
                image->session.Run(runOptions, binding);

                ValueMap outTensorMap;
                auto outputValues = binding.GetOutputValues();
                for (size_t i = 0; i < outputValues.size(); ++i) {
                    outTensorMap.emplace(
                        outputNames[i],
                        makeSharedValue(std::move(outputValues[i]))
                    );
                }
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() - timeStart)
                                   .count();
                auto elapsedSeconds = elapsed / 1000;
                auto elapsedMs = static_cast<int>(elapsed % 1000);
                char elapsedMsStr[4];
                snprintf(elapsedMsStr, sizeof(elapsedMsStr), "%03d", elapsedMs);
                LOG_INFO("[onnxdriver] Session [%1] - Finished inference in %2.%3 seconds", filename,
                         elapsedSeconds, elapsedMsStr);
                state.store(Session::State::Idle);
                return outTensorMap;
            } catch (const Ort::Exception &err) {
                if (errorMessage) {
                    *errorMessage = err.what();
                }
            }
            state.store(Session::State::Failed);
            return {};
        }

        std::vector<std::string> Session::inputNames() const {
            if (!_impl) {
                return {};
            }
            auto &impl = *_impl;
            if (!impl.image) {
                return {};
            }
            return impl.image->inputNames;
        }

        std::vector<std::string> Session::outputNames() const {
            if (!_impl) {
                return {};
            }
            auto &impl = *_impl;
            if (!impl.image) {
                return {};
            }
            return impl.image->outputNames;
        }

        void Session::terminate() {
            if (!_impl) {
                return;
            }
            auto &impl = *_impl;
            impl.runOptions.SetTerminate();
            impl.state.store(Session::State::Terminated);
        }

        ValueMap Session::run(ValueMap &inputTensorMap, std::string *errorMessage) {
            if (!_impl) {
                return {};
            }
            auto &impl = *_impl;
            return SessionRunHelper_Value(impl.image, impl.runOptions, inputTensorMap,
                                               impl.state, errorMessage);
        }


        Ort::Session createOrtSession(const Ort::Env &env, const std::filesystem::path &modelPath,
                                      bool preferCpu, std::string *errorMessage) {
            try {
                Ort::SessionOptions sessOpt;

                auto onnxdriverEnv = Env::instance();
                auto ep = onnxdriverEnv->executionProvider();
                auto deviceIndex = onnxdriverEnv->deviceIndex();

                std::string initEPErrorMsg;
                if (!preferCpu) {
                    switch (ep) {
                        case EP_DirectML: {
                            if (!initDirectML(sessOpt, deviceIndex, &initEPErrorMsg)) {
                                // log warning: "Could not initialize DirectML: {initEPErrorMsg}, falling back to CPU."
                                LOG_WARNING("[onnxdriver] Could not initialize DirectML: %1, falling back to CPU.",
                                            initEPErrorMsg);
                            } else {
                                LOG_INFO("[onnxdriver] Use DirectML. Device index: %1", deviceIndex);
                            }
                            break;
                        }
                        case EP_CUDA: {
                            if (!initCUDA(sessOpt, deviceIndex, &initEPErrorMsg)) {
                                // log warning: "Could not initialize CUDA: {initEPErrorMsg}, falling back to CPU."
                                LOG_WARNING("[onnxdriver] Could not initialize CUDA: %1, falling back to CPU.",
                                            initEPErrorMsg);
                            } else {
                                LOG_INFO("[onnxdriver] Use CUDA. Device index: %1", deviceIndex);
                            }
                            break;
                        }
                        default: {
                            // log info: "Use CPU."
                            LOG_INFO("[onnxdriver] Use CPU.");
                            break;
                        }
                    }
                } else {
                    LOG_INFO("[onnxdriver] The model prefers to use CPU. [%1]", modelPath.filename());
                }

#ifdef _WIN32
                auto pathStr = modelPath.wstring();
#else
                auto pathStr = modelPath.string();
#endif
                return Ort::Session{env, pathStr.c_str(), sessOpt};
            } catch (const Ort::Exception &e) {
                if (errorMessage) {
                    *errorMessage = e.what();
                }
            }
            return Ort::Session{nullptr};
        }
    }
}