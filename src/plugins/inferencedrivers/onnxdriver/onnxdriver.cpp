#include "onnxdriver.h"

namespace dsinfer {

    OnnxDriver::OnnxDriver() {
    }

    OnnxDriver::~OnnxDriver() {
    }

    bool OnnxDriver::initialize(const JsonObject &args, Error *error) const {
        return false;
    }

    int64_t OnnxDriver::sessionCreate(const std::filesystem::path &path, const JsonObject &args,
                                      Error *error) const {
        return 0;
    }

    bool OnnxDriver::sessionDestroy(int64_t handle, Error *error) const {
        return false;
    }

    bool OnnxDriver::sessionRunning(int64_t handle) const {
        return false;
    }

    int64_t OnnxDriver::taskCreate() const {
        return 0;
    }

    void OnnxDriver::taskDestroy(int64_t handle) const {
    }

    bool OnnxDriver::taskStart(int64_t handle, const JsonValue &input, Error *error) const {
        return false;
    }

    bool OnnxDriver::taskStop(int64_t handle, Error *error) const {
        return false;
    }

    int OnnxDriver::taskState(int64_t handle) const {
        return 0;
    }

    bool OnnxDriver::taskResult(int64_t handle, JsonValue *result) const {
        return false;
    }

}