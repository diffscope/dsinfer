#ifndef DSINFER_CAPI_H
#define DSINFER_CAPI_H

#include <stdint.h>

#include <dsinfer/dsinfer_global.h>

#ifdef __cplusplus
extern "C" {
#endif

enum DSINFER_ExecutionProvider {
    EP_CPU = 1,
    EP_DirectML = 2,
    EP_CUDA = 3,
    EP_CoreML_4,
};

enum DSINFER_Error {
    OnnxRuntimeNotFound = -1,
    EPInitializeFailed = -2,
};

enum DSINFER_ModelType {
    MT_Linguistic,
    MT_PhonemeDuration,
    MT_Pitch,
    MT_Variance,
    MT_Vocoder,
};

DSINFER_EXPORT int dsinfer_init(DSINFER_ExecutionProvider ep);

DSINFER_EXPORT int dsinfer_load_model(const char *path);

#ifdef __cplusplus
}
#endif

#endif // DSINFER_CAPI_H
