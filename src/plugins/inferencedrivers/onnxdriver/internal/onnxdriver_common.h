#ifndef FLOWONNX_COMMON_H
#define FLOWONNX_COMMON_H

namespace dsinfer {
    namespace onnxdriver {

        enum ErrorType {
            ET_Invalid = -1,
            ET_Default = 0,
            ET_LoadError,
            ET_InferenceError,
        };

        enum ErrorCode {
            EC_Invalid = -1,
            EC_Success = 0,
            EC_OnnxRuntimeLoadFailed,
            EC_OnnxRuntimeAlreadyLoaded,
            EC_EnvInitializeFailed,
            EC_EnvAlreadyInitialized,
            EC_EPInitializeFailed,
        };

        enum ExecutionProvider {
            EP_CPU = 1,
            EP_DirectML = 2,
            EP_CUDA = 3,
            EP_CoreML = 4,
        };

    }
}

#endif // FLOWONNX_COMMON_H
