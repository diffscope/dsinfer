#ifndef DSINFER_ONNXDRIVER_SESSION_H
#define DSINFER_ONNXDRIVER_SESSION_H

#include <map>
#include <memory>
#include <filesystem>
#include <functional>

#include <dsinfer/error.h>

#include "valuemap.h"

namespace dsinfer::onnxdriver {

    class Session {
    public:
        Session();
        ~Session();

        Session(const Session &) = delete;
        Session &operator=(const Session &) = delete;

        Session(Session &&other) noexcept;
        Session &operator=(Session &&other) noexcept;

    public:
        bool open(const std::filesystem::path &path, int hints, Error *error);
        bool close();

        const std::vector<std::string> &inputNames() const;
        const std::vector<std::string> &outputNames() const;

        ValueMap run(const ValueMap &inputTensorMap, Error *error = nullptr);
        SharedValueMap run(const SharedValueMap &inputTensorMap, Error *error = nullptr);

        void terminate();

        std::filesystem::path path() const;
        bool isOpen() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // DSINFER_ONNXDRIVER_SESSION_H
