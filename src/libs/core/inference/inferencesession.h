#ifndef SESSION_H
#define SESSION_H

#include <dsinfer/jsonvalue.h>
#include <dsinfer/environment.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceSession {
    public:
        explicit InferenceSession(Environment *env);
        ~InferenceSession();

    public:
        bool open(const std::filesystem::path &path, const JsonObject &args, Error *error);
        bool close(Error *error);

    public:
        int64_t id() const;
        bool isRunning() const;

    public:
        Environment *env() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // SESSION_H
