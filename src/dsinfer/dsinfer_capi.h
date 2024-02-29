#ifndef DSINFER_CAPI_H
#define DSINFER_CAPI_H

#include <stdint.h>

#include <dsinfer/dsinfer_global.h>

#ifdef __cplusplus
extern "C" {
#endif

enum DSINFER_ErrorType {
    ET_Invalid = -1,
    ET_Default = 0,
    ET_LoadError,
    ET_InferenceError,
};

enum DSINFER_ErrorCode {
    EC_Invalid = -1,
    EC_Success = 0,
    EC_OnnxRuntimeLoadFailed,
    EC_OnnxRuntimeAlreadyLoaded,
    EC_EnvInitializeFailed,
    EC_EnvAlreadyInitialized,
    EC_EPInitializeFailed,
};

struct DSINFER_Status {
    int type;
    int code;
    char message[1]; // a null-terminated string
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
    EP_CoreML = 4,
};

enum DSINFER_ModelType {
    MT_Vocoder,
    MT_Acoustic,
    MT_Pitch,
    MT_PhonemeDuration,
    MT_Variance,
};


/**
 * @brief Create the status
 *
 * @param type DSINFER_ErrorType
 * @param code DSINFER_ErrorCode
 * @param message Error Message (maximum 2048 characters, longer texts will be truncated)
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_create_status(int type, int code, const char *message);


/**
 * @brief Release the status
 *
 * @param status Status to release
 */
DSINFER_EXPORT void dsinfer_release_status(DSINFER_Status *status);


/**
 * @brief Initialize the library.
 *
 * @param path Ort runtime path
 * @param ep   Given execution provider
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_init(const char *path, DSINFER_ExecutionProvider ep);

/**
 * @brief Finialize the library.
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_quit();


/**
 * @brief Load model from filesystem.
 *
 * @param path Model configuration path
 * @param gpu_id Select GPU ID. If you want to force the model to run on CPU, set it to -1
 */
DSINFER_EXPORT DSINFER_Status *dsinfer_load_model(const char *path, int gpu_id,
                                                  DSINFER_Model **model);


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
} // extern "C"
#endif

#endif // DSINFER_CAPI_H
