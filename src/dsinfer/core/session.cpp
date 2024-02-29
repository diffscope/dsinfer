#include "session.h"

#include <onnxruntime_cxx_api.h>

namespace dsinfer {

    class Session::Impl {
    public:
        bool loaded = false;
    };

    Session::Session() : _impl(std::make_unique<Impl>()) {
    }

    Session::~Session() {
        free();
    }

    void Session::load(const std::filesystem::path &path) {
        if (_impl->loaded) {
            return;
        }
    }

    void Session::free() {
        if (!_impl->loaded) {
            return;
        }
    }

}