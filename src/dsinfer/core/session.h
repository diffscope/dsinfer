#ifndef SESSION_H
#define SESSION_H

#include <filesystem>

#include <dsinfer/dsinfer_global.h>
#include <dsinfer/dsinfer_common.h>

namespace dsinfer {

    class DSINFER_EXPORT Session {
    public:
        Session();
        ~Session();

    public:
        void load(const std::filesystem::path &path, int deviceIndex = -1);
        void free();
        void terminate();
        void unsetTerminate();
        [[nodiscard]] ModelType type() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        DSINFER_DISABLE_COPY(Session)
    };

}

#endif // SESSION_H
