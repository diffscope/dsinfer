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
        using callback_t = std::function<void(const ValueMap &outputTensorMap, const Error &error)>;
        using callback_shared_t = std::function<void(const SharedValueMap &outputTensorMap, const Error &error)>;

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

        void runAsync(const ValueMap &inputTensorMap, callback_t callback);
        void runAsync(const SharedValueMap &inputTensorMap, callback_shared_t callback);

        void terminate();

        std::filesystem::path path() const;
        bool isOpen() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // DSINFER_ONNXDRIVER_SESSION_H
