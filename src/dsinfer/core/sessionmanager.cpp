#include "sessionmanager.h"
#include "session_p.h"

#include <unordered_map>
#include <mutex>

#include <dsinfer/session.h>

namespace dsinfer {

    class SessionManager::Impl {
    public:
        Impl() : maxId(0) {}

        SessionHandle create(const std::filesystem::path &path) {
            std::lock_guard<std::mutex> lock(mtx);

            SessionHandle currentId;
            if (maxId >= std::numeric_limits<SessionHandle>::max()) {
                currentId = 0;
            }
            else {
                currentId = ++maxId;
            }
            if (currentId == 0) {
                return 0;
            }
            pool.emplace(currentId, Session(path));
            return currentId;
        }

        bool remove(SessionHandle id) {
            std::lock_guard<std::mutex> lock(mtx);

            if (auto it = pool.find(id); it != pool.end()) {
                if (it->second.isLoaded()) {
                    return false;
                }
                pool.erase(it);
                return true;
            }
            return false;
        }

        SharedSession get(SessionHandle id) {
            if (auto it = pool.find(id); it != pool.end()) {
                auto &session = it->second;
                return SharedSession(session);
            }
            return SharedSession(nullptr);
        }

        SessionHandle maxId;
        std::mutex mtx;
        std::unordered_map<SessionHandle, Session> pool;
    };

    SessionManager::SessionManager() : _impl(std::make_unique<Impl>()) {}

    SessionManager::~SessionManager() = default;

    SessionHandle SessionManager::create(const std::filesystem::path &path) {
        return _impl->create(path);
    }

    bool SessionManager::remove(SessionHandle id) {
        return _impl->remove(id);
    }

    SharedSession SessionManager::get(SessionHandle id) {
        return _impl->get(id);
    }

    size_t SessionManager::count() const {
        return _impl->pool.size();
    }

}