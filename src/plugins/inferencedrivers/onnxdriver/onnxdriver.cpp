#include "onnxdriver.h"

#include "onnxsession.h"
#include "onnxtask.h"
#include "onnxcontext.h"

namespace dsinfer {

    OnnxDriver::OnnxDriver() = default;

    OnnxDriver::~OnnxDriver() = default;

    bool OnnxDriver::initialize(const JsonValue &args, Error *error) {
        return true;
    }
    InferenceSession *OnnxDriver::createSession() {
        return new OnnxSession();
    }
    InferenceTask *OnnxDriver::createTask() {
        return new OnnxTask();
    }
    InferenceContext *OnnxDriver::createContext() {
        return new OnnxContext();
    }

}