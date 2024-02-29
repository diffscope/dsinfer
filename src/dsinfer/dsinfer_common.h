#ifndef DSINFER_DSINFER_COMMON_H
#define DSINFER_DSINFER_COMMON_H

#include <dsinfer/dsinfer_global.h>

namespace dsinfer {

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

    enum ModelType {
        MT_Acoustic,
        MT_Pitch,
        MT_PhonemeDuration,
        MT_Variance,
        MT_Linguistic,
        MT_Vocoder,
    };

    constexpr const int kMaxStrLen = 2048;

}

#endif // DSINFER_DSINFER_COMMON_H
