#include "sessionsystem.h"

#include <mutex>

namespace dsinfer::onnxdriver {

    static SessionSystem *g_sessionSystem = nullptr;

    SessionSystem::SessionSystem() {
        g_sessionSystem = this;
    }

    SessionSystem::~SessionSystem() {
        g_sessionSystem = nullptr;
    }

    SessionSystem *SessionSystem::instance() {
        return g_sessionSystem;
    }

    bool SessionSystem::addImage(const std::filesystem::path &path, SessionImage *image,
                                 bool overwrite) {
        std::unique_lock<std::shared_mutex> lock(mtx);

        if (!overwrite && sessionImageMap.find(path) != sessionImageMap.end()) {
            return false;
        }

        sessionImageMap[path] = image;
        return true;
    }

    bool SessionSystem::removeImage(const std::filesystem::path &path) {
        std::unique_lock<std::shared_mutex> lock(mtx);

        if (auto it = sessionImageMap.find(path); it != sessionImageMap.end()) {
            sessionImageMap.erase(it);
            return true; // Successfully removed
        }

        return false; // Key does not exist
    }

    SessionImage *SessionSystem::getImage(const std::filesystem::path &path) {
        std::shared_lock<std::shared_mutex> lock(mtx);

        if (auto it = sessionImageMap.find(path); it != sessionImageMap.end()) {
            return it->second; // Return the pointer to the image
        }

        return nullptr; // Key does not exist, return nullptr
    }
}