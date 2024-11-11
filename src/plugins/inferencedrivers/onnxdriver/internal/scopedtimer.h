#ifndef DSINFER_ONNXDRIVER_SCOPEDTIMER_H
#define DSINFER_ONNXDRIVER_SCOPEDTIMER_H

#include <chrono>
#include <functional>

namespace dsinfer::onnxdriver {
    class ScopedTimer {
    public:
        using duration_t = std::chrono::duration<double>; // seconds
        using callback_t = std::function<void(const duration_t &)>;

        explicit ScopedTimer(callback_t callback, bool activated = true)
            : m_isActive(activated), m_callback(std::move(callback)), m_timeStart(std::chrono::steady_clock::now()) {
        }

        ~ScopedTimer() {
            if (!m_isActive) {
                return;
            }
            const auto elapsed = std::chrono::duration_cast<duration_t>(
                std::chrono::steady_clock::now() - m_timeStart);
            m_callback(elapsed);
        }

        void activate() {
            m_isActive = true;
        }

        void deactivate() {
            m_isActive = false;
        }

    private:
        bool m_isActive;
        callback_t m_callback;
        std::chrono::time_point<std::chrono::steady_clock> m_timeStart;
    };
}

#endif
