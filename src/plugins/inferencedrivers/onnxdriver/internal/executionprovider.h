#ifndef DSINFR_ONNXDRIVER_EXECUTIONPROVIDER_P_H
#define DSINFR_ONNXDRIVER_EXECUTIONPROVIDER_P_H

#include <onnxruntime_cxx_api.h>

namespace dsinfer::onnxdriver {
    bool initCUDA(Ort::SessionOptions &options, int deviceIndex, std::string *errorMessage = nullptr);
    bool initDirectML(Ort::SessionOptions &options, int deviceIndex, std::string *errorMessage = nullptr);
}

#endif // DSINFR_ONNXDRIVER_EXECUTIONPROVIDER_P_H
