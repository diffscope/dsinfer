#include "onnxdriver.h"

namespace dsinfer {

    OnnxDriver::OnnxDriver() {
    }

    OnnxDriver::~OnnxDriver() {
    }

    bool OnnxDriver::initialize(const JsonValue &args, Error *error) {
        return false;
    }

    int64_t OnnxDriver::sessionCreate(const std::filesystem::path &path, const JsonValue &args,
                                      Error *error) {
        return 0;
    }

    bool OnnxDriver::sessionDestroy(int64_t handle, Error *error) {
        return false;
    }

    bool OnnxDriver::sessionRunning(int64_t handle) {
        return false;
    }

    int64_t OnnxDriver::taskCreate() {
        return 0;
    }

    void OnnxDriver::taskDestroy(int64_t handle) {
    }

    bool OnnxDriver::taskStart(int64_t handle, const JsonValue &input, Error *error) {
        return false;
    }

    bool OnnxDriver::taskStop(int64_t handle, Error *error) {
        return false;
    }

    int OnnxDriver::taskState(int64_t handle) {
        return 0;
    }

    bool OnnxDriver::taskResult(int64_t handle, JsonValue *result) {
        return false;
    }

}