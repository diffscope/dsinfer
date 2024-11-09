#ifndef DSINFER_ONNXDRIVER_SESSIONIMAGE_P_H
#define DSINFER_ONNXDRIVER_SESSIONIMAGE_P_H

#include <filesystem>

#include <dsinfer/error.h>

#include <onnxruntime_cxx_api.h>

namespace dsinfer::onnxdriver {

    class SessionImage {
    public:
        SessionImage();
        ~SessionImage();

        bool open(const std::filesystem::path &onnxPath, int hints,
                  std::string *errorMessage = nullptr);

    public:
        std::vector<std::string> inputNames;
        std::vector<std::string> outputNames;

        Ort::Env env;
        Ort::Session session;
    };

}

#endif // DSINFER_ONNXDRIVER_SESSIONIMAGE_P_H
