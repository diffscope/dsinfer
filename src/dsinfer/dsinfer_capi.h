#ifndef DSINFER_CAPI_H
#define DSINFER_CAPI_H

#include <stdint.h>

#include <dsinfer/dsinfer_global.h>

#ifdef __cplusplus
extern "C" {
#endif

enum DSINFER_ErrorCode {
    LoadError,
    InferenceError,
};

struct DSINFER_Status {
    int type;
    int code;
    const char *message;
};

struct DSINFER_Model {
    const char *path;
};

struct DSINFER_InferenceContext {
    volatile int finished;
    const void *data;
    int size;
};

struct DSINFER_Arguments {
    const char *json;
    void (*callback)(void *userdata, DSINFER_InferenceContext *context);
    void *userdata;
};

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
    MT_Vocoder,
    MT_Acoustic,
    MT_Pitch,
    MT_PhonemeDuration,
    MT_Variance,
};


/**
 * @brief Release the release
 *
 * @param status Status to release
 */
DSINFER_EXPORT void release_status(DSINFER_Status *status);


/**
 * @brief Initialize the library.
 *
 * @param ep Given execution provider
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_init(DSINFER_ExecutionProvider ep);


/**
 * @brief Finalize the library.
 *
 */
DSINFER_EXPORT void dsinfer_quit();


/**
 * @brief Load model from filesystem.
 *
 * @param path Configuration path
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_load_model(const char *path, DSINFER_Model **model);


/**
 * @brief Release the model.
 *
 * @param model Model context
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_release_model(DSINFER_Model *model);


/**
 * @brief Start the inference.
 *
 * @param context Model context reference
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_start_inference(int handle, DSINFER_Arguments *args,
                                                       DSINFER_InferenceContext **context);


/**
 * @brief Release an inference.
 *
 * @param context Inference context
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_release_inference(DSINFER_InferenceContext *context);

#ifdef __cplusplus
}
#endif

#endif // DSINFER_CAPI_H
