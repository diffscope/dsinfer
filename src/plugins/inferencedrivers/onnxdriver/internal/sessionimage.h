#ifndef DSINFR_ONNXDRIVER_SESSIONIMAGE_P_H
#define DSINFR_ONNXDRIVER_SESSIONIMAGE_P_H

#include <shared_mutex>

#include <dsinfer/error.h>

#include <onnxruntime_cxx_api.h>

#include "onnxdriver_logger.h"
#include "session.h"
#include "sessionsystem.h"

namespace dsinfer::onnxdriver {

    class SessionImage {
    public:
        static SessionImage *create(const std::filesystem::path &onnxPath, bool preferCpu, std::string *errorMessage = nullptr);
        int ref();
        int deref();
    protected:
        explicit SessionImage(std::filesystem::path path);
        bool init(bool preferCpu, std::string *errorMessage = nullptr);
    public:
        std::filesystem::path path;
        int count;

        std::vector<std::string> inputNames;
        std::vector<std::string> outputNames;

        Ort::Env env;
        Ort::Session session;
    };

}

#endif // DSINFR_ONNXDRIVER_SESSIONIMAGE_P_H
