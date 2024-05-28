#ifndef DSINFER_SESSION_P_H
#define DSINFER_SESSION_P_H

#include <cstddef>
#include <mutex>
#include <filesystem>

#include <dsinfer/dsinfer_common.h>

#include <onnxruntime_cxx_api.h>

namespace dsinfer {
    class SessionPrivate {
    public:
        SessionPrivate();
        ~SessionPrivate();
        bool load();
        void free();

        size_t refCount;
        std::mutex mtx;
        std::filesystem::path onnxPath;
        bool forceOnCpu;
        std::vector<std::string> inputNames, outputNames;

        Ort::Env m_env;
        Ort::Session m_session;
    };
}

#endif
