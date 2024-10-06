#ifndef ONNXDRIVER_EXECUTIONPROVIDER_P_H
#define ONNXDRIVER_EXECUTIONPROVIDER_P_H

#include <onnxruntime_cxx_api.h>

namespace dsinfer {
    namespace onnxdriver {
        bool initCUDA(Ort::SessionOptions &options, int deviceIndex,
                      std::string *errorMessage = nullptr);
        bool initDirectML(Ort::SessionOptions &options, int deviceIndex,
                          std::string *errorMessage = nullptr);
    }
}

#endif // ONNXDRIVER_EXECUTIONPROVIDER_P_H