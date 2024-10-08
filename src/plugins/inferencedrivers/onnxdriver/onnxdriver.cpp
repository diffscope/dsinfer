#include "onnxdriver.h"

#include <mutex>
#include <numeric>
#include <shared_mutex>

#include <onnxruntime_cxx_api.h>

#include <dsinfer/format.h>
#include <dsinfer/error.h>

#include "internal/env.h"
#include "internal/session.h"
#include "internal/tensormap.h"
#include "internal/tensorparser.h"

namespace dsinfer {

    class InferTask {
    public:
        enum TaskState: int {
            Invalid = -1,
            Idle = 0,
            Running = 1,
            Failed = 2,
            Terminated = 3,
        };

        std::unordered_map<int64_t, onnxdriver::Session *> sessionMap;
        JsonValue value;
        TaskState state = Idle;
    };


    class OnnxDriver::Impl {
    public:
        bool initialize(const JsonValue &args_, Error *error) {
            std::unique_lock<std::shared_mutex> lock(drv_mtx);

            bool isSuccess = true;

            std::filesystem::path libPath;
            onnxdriver::ExecutionProvider ep;
            int deviceIndex;

            auto args = args_.toObject();
            if (auto it = args.find("lib"); it != args.end()) {
                if (it->second.isString()) {
#ifdef _WIN32
                    auto libPathStr = it->second.toString();
                    libPath = utf8ToWide(libPathStr.c_str());
#else
                    libPath = it->second.toString();
#endif
                } else {
                    isSuccess = false;
                }
            } else {
                isSuccess = false;
            }
            if (!isSuccess) {
                if (error) {
                    *error = Error(Error::InvalidFormat, "Must specify \"lib\" in args!");
                }
                return false;
            }

            if (auto it = args.find("ep"); it != args.end()) {
                if (it->second.isString()) {
                    auto epString = it->second.toString();
                    if (epString == "dml") {
                        ep = onnxdriver::EP_DirectML;
                    } else if (epString == "cuda") {
                        ep = onnxdriver::EP_CUDA;
                    } else if (epString == "coreml") {
                        ep = onnxdriver::EP_CoreML;
                    } else {
                        ep = onnxdriver::EP_CPU;
                    }
                } else {
                    ep = onnxdriver::EP_CPU;
                }
            } else {
                ep = onnxdriver::EP_CPU;
            }

            if (auto it = args.find("deviceIndex"); it != args.end()) {
                if (it->second.isDouble()) {
                    deviceIndex = static_cast<int>(it->second.toDouble());
                } else {
                    deviceIndex = 0;
                }
            } else {
                deviceIndex = 0;
            }

            std::string errorMessage;
            isSuccess = env.load(libPath, ep, &errorMessage);
            if (!isSuccess) {
                if (error) {
                    *error = Error(Error::LibraryNotFound, errorMessage);
                }
                return false;
            }
            env.setDeviceIndex(deviceIndex);
            return true;
        }

        int64_t sessionCreate(const std::filesystem::path &path, const JsonValue &args_,
                              Error *error) {
            bool preferCpu = false;
            auto args = args_.toObject();
            if (auto it = args.find("preferCpu"); it != args.end()) {
                if (it->second.isBool()) {
                    preferCpu = it->second.toBool();
                }
            }
            std::string errorMessage;
            auto session = std::make_unique<onnxdriver::Session>();
            if (!session->open(path, preferCpu, &errorMessage)) {
                if (error) {
                    *error = Error(Error::FeatureNotSupported, errorMessage); // TODO: more error types
                }
                return 0;
            }

            std::unique_lock<std::shared_mutex> lock(drv_mtx);
            int64_t sessionHandle = ++sessionHandleCounter;
            sessionMap.emplace(sessionHandle, std::move(session));
            return sessionHandle;
        }

        bool sessionDestroy(int64_t handle, Error *error) {
            std::unique_lock<std::shared_mutex> lock(drv_mtx);
            if (auto it = sessionMap.find(handle); it != sessionMap.end()) {
                it->second->terminate();
                it->second->close();
                sessionMap.erase(it);
                return true;
            }
            if (error) {
                *error = Error(Error::FeatureNotSupported, "Session handle not found!"); // TODO: more error types
            }
            return false;
        }

        bool sessionRunning(int64_t handle) {
            std::shared_lock<std::shared_mutex> lock(drv_mtx);
            if (auto it = sessionMap.find(handle); it != sessionMap.end()) {
                return it->second->isOpen();
            }
            return false;
        }

        int64_t taskCreate() {
            std::unique_lock<std::shared_mutex> lock(drv_mtx);
            int64_t taskHandle = ++taskHandleCounter;
            taskMap.emplace(taskHandle, std::make_unique<InferTask>());
            return taskHandle;
        }

        void taskDestroy(int64_t handle) {
            std::unique_lock<std::shared_mutex> lock(drv_mtx);
            if (auto it = taskMap.find(handle); it != taskMap.end()) {
                taskMap.erase(it);
            }
        }

        bool taskStart(int64_t handle, const JsonValue &input, Error *error) {
            std::unique_lock<std::shared_mutex> lock(drv_mtx);
            auto it = taskMap.find(handle);
            if (it == taskMap.end()) {
                if (error) {
                    *error = Error(Error::InvalidFormat, "task handle not found"); // TODO: more error types
                }
                return false;
            }

            auto &task = it->second;

            if (!input.isArray()) {
                task->state = InferTask::Failed;
                return false;
            }

            // TODO: Currently only supports single model. Need to implement infer map.
            auto inputArray = input.toArray();
            if (inputArray.empty()) {
                task->state = InferTask::Failed;
                return false;
            }

            auto &item = inputArray[0];

            auto sessionId = item["session"].toInt();
            auto it_session = sessionMap.find(sessionId);
            if (it_session == sessionMap.end()) {
                if (error) {
                    *error = Error(Error::InvalidFormat, "session handle not found"); // TODO: more error types
                }
                task->state = InferTask::Failed;
                return false;
            }
            auto &sessionObj = it_session->second;
            if (!sessionObj || !sessionObj->isOpen()) {
                if (error) {
                    *error = Error(Error::InvalidFormat, "session is not open"); // TODO: more error types
                }
                task->state = InferTask::Failed;
                return false;
            }

            task->sessionMap.emplace(sessionId, sessionObj.get());

            onnxdriver::ValueMap inputMap;
            for (const auto &inputItem: item["input"].toArray()) {
                Error err_;
                auto val = onnxdriver::deserializeTensor(inputItem, &err_);
                if (!err_.ok()) {
                    task->state = InferTask::Failed;
                    if (error) {
                        *error = err_;
                    }
                    return false;
                }
                inputMap.emplace(inputItem["name"].toString(), onnxdriver::makeSharedValue(std::move(val)));
            }

            std::string errorMessage;
            auto result = sessionObj->run(inputMap, &errorMessage);
            if (result.empty()) {
                if (error) {
                    *error = Error(Error::InvalidFormat, "inference failed: " + errorMessage); // TODO: more error types
                }
                task->state = InferTask::Failed;
                return false;
            }

            JsonArray inferResult;
            for (const auto &[inputName, tensor] : std::as_const(result)) {
                Error err;
                JsonObject obj = onnxdriver::serializeTensor(*tensor, &err).toObject();
                if (!err.ok()) {
                    if (error) {
                        *error = std::move(err);
                    }
                    return false;
                }
                obj["name"] = inputName;
                inferResult.emplace_back(std::move(obj));
            }
            task->value = std::move(inferResult);

            task->state = InferTask::Idle;
            return true;
        }

        bool taskStop(int64_t handle, Error *error) {
            std::unique_lock<std::shared_mutex> lock(drv_mtx);
            if (auto it = taskMap.find(handle); it != taskMap.end()) {
                auto &task = it->second;
                if (!task->sessionMap.empty()) {
                    return false;
                }
                if (task->state != InferTask::Running) {
                    return false;
                }
                for (auto [_, session]: task->sessionMap) {
                    if (session) {
                        session->terminate();
                    }
                }
                task->state = InferTask::Terminated;
                return true;
            }
            return false;
        }

        int taskState(int64_t handle) {
            std::shared_lock<std::shared_mutex> lock(drv_mtx);
            if (auto it = taskMap.find(handle); it != taskMap.end()) {
                auto &task = it->second;
                return task->state;
            }
            return InferTask::Invalid;
        }

        bool taskResult(int64_t handle, JsonValue *result) {
            if (!result) {
                return false;
            }
            std::shared_lock<std::shared_mutex> lock(drv_mtx);
            if (auto it = taskMap.find(handle); it != taskMap.end()) {
                auto &task = it->second;
                *result = task->value;
                return true;
            }
            return false;
        }

        onnxdriver::Env env;
        std::unordered_map<int64_t, std::unique_ptr<onnxdriver::Session>> sessionMap;
        std::unordered_map<int64_t, std::unique_ptr<InferTask>> taskMap;
        int64_t sessionHandleCounter = 0;
        int64_t taskHandleCounter = 0;
        mutable std::shared_mutex drv_mtx;
    };

    OnnxDriver::OnnxDriver(): _impl(std::make_unique<Impl>()) {
    }

    OnnxDriver::~OnnxDriver() = default;

    bool OnnxDriver::initialize(const JsonValue &args, Error *error) {
        __dsinfer_impl_t;
        return impl.initialize(args, error);
    }

    int64_t OnnxDriver::sessionCreate(const std::filesystem::path &path, const JsonValue &args,
                                      Error *error) {
        __dsinfer_impl_t;
        return impl.sessionCreate(path, args, error);
    }

    bool OnnxDriver::sessionDestroy(int64_t handle, Error *error) {
        __dsinfer_impl_t;
        return impl.sessionDestroy(handle, error);
    }

    bool OnnxDriver::sessionRunning(int64_t handle) {
        __dsinfer_impl_t;
        return impl.sessionRunning(handle);
    }

    int64_t OnnxDriver::taskCreate() {
        __dsinfer_impl_t;
        return impl.taskCreate();
    }

    void OnnxDriver::taskDestroy(int64_t handle) {
        __dsinfer_impl_t;
        impl.taskDestroy(handle);
    }

    bool OnnxDriver::taskStart(int64_t handle, const JsonValue &input, Error *error) {
        __dsinfer_impl_t;
        return impl.taskStart(handle, input, error);
    }

    bool OnnxDriver::taskStop(int64_t handle, Error *error) {
        __dsinfer_impl_t;
        return impl.taskStop(handle, error);
    }

    int OnnxDriver::taskState(int64_t handle) {
        __dsinfer_impl_t;
        return impl.taskState(handle);
    }

    bool OnnxDriver::taskResult(int64_t handle, JsonValue *result) {
        __dsinfer_impl_t;
        return impl.taskResult(handle, result);
    }
}