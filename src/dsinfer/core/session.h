#ifndef SESSION_H
#define SESSION_H

#include <cstddef>
#include <string>
#include <vector>
#include <filesystem>

#include <dsinfer/dsinfer_global.h>
#include <dsinfer/dsinfer_common.h>

namespace dsinfer {

    class SessionPrivate;

    class DSINFER_EXPORT Session {
    public:
        Session(Session &&other) noexcept;
        Session &operator=(Session &&other) noexcept;
        ~Session();

        DSINFER_DISABLE_COPY(Session)

    public:
        bool isLoaded() const;
        size_t useCount() const;
        std::vector<std::string> inputNames() const;
        std::vector<std::string> outputNames() const;

    protected:
        friend class SessionManager;
        friend class SharedSession;
        // Each Session can only be contructed by SessionManager
        explicit Session(const std::filesystem::path &path, bool forceOnCpu = false);
        std::unique_ptr<SessionPrivate> d;
    };

}

#endif // SESSION_H
