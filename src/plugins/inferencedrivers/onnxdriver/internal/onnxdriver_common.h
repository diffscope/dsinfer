#ifndef DSINFER_ONNXDRIVER_ONNXDRIVER_COMMON_H
#define DSINFER_ONNXDRIVER_ONNXDRIVER_COMMON_H

namespace dsinfer::onnxdriver {

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

    enum SessionHint {
        SH_NoHint,
        SH_PreferCPUHint = 0x1,
    };

}

#endif // DSINFER_ONNXDRIVER_ONNXDRIVER_COMMON_H
