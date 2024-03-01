#include "dsinfer_capi.h"

#include <cstddef>
#include <cstring>
#include <new>

#include <loadso/system.h>

#include "dsinfer_common.h"
#include "environment.h"

DSINFER_Status *dsinfer_create_status(int type, int code, const char *message) {
    size_t clen(nullptr == message ? 0 : strnlen(message, dsinfer::kMaxStrLen));
    auto status = [](size_t clen) -> DSINFER_Status * {
        // See ONNXRuntime's implementation of OrtStatus (https://github.com/microsoft/onnxruntime)
        auto buf = new (std::nothrow) uint8_t[sizeof(DSINFER_Status) + clen];
        if (buf == nullptr) {
            return nullptr;
        }
        return new (buf) DSINFER_Status;
    }(clen);
    if (status == nullptr) {
        return nullptr;
    }
    status->type = type;
    status->code = code;
    if (message) {
        std::memcpy(status->message, message, clen);
        status->message[clen] = '\0';
    }
    return status;
}

void dsinfer_release_status(DSINFER_Status *status) {
    delete[] reinterpret_cast<uint8_t *>(status);
}

DSINFER_Status *dsinfer_init(const char *path, DSINFER_ExecutionProvider ep) {
    auto env = dsinfer::Environment::instance();
    if (!env) {
        env = new dsinfer::Environment();
        try {
            env->load(LoadSO::System::MultiToPathString(path),
                      static_cast<dsinfer::ExecutionProvider>(ep));
        } catch (const std::exception &e) {
            delete env;
            return dsinfer_create_status(EC_OnnxRuntimeLoadFailed, ET_LoadError, e.what());
        }
    }
    return dsinfer_create_status(EC_Success, ET_Default, nullptr);
}

DSINFER_Status *dsinfer_quit() {
    delete dsinfer::Environment::instance();
    return dsinfer_create_status(EC_Success, ET_Default, nullptr);
}

DSINFER_Status *dsinfer_load_model(const char *path, int gpu_id, DSINFER_Model **model) {
    return nullptr;
}

DSINFER_Status *dsinfer_release_model(DSINFER_Model *model) {
    return nullptr;
}

DSINFER_Status *dsinfer_start_inference(int handle, DSINFER_Arguments *args,
                                        DSINFER_InferenceContext **context) {
    return nullptr;
}

DSINFER_Status *dsinfer_release_inference(DSINFER_InferenceContext *context) {
    return nullptr;
}
