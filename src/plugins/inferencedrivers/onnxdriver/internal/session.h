#ifndef ONNXDRIVER_SESSION_H
#define ONNXDRIVER_SESSION_H

#include <map>
#include <memory>
#include <filesystem>
#include <functional>

#include "tensormap.h"

namespace dsinfer {
    namespace onnxdriver {

        class Session {
        public:
            Session();
            ~Session();

            Session(Session &&other) noexcept;
            Session &operator=(Session &&other) noexcept;

            enum State {
                Idle,
                Running,
                Failed,
                Terminated,
            };

        public:
            bool open(const std::filesystem::path &path, bool preferCpu, std::string *errorMessage);
            bool close();

            std::vector<std::string> inputNames() const;
            std::vector<std::string> outputNames() const;

            ValueMap run(ValueMap &inputTensorMap, std::string *errorMessage = nullptr);

            void terminate();

            std::filesystem::path path() const;
            bool isOpen() const;
            State state() const;

        protected:
            class Impl;
            std::unique_ptr<Impl> _impl;
        };
    }
}

#endif // ONNXDRIVER_SESSION_H
