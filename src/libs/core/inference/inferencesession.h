#ifndef SESSION_H
#define SESSION_H

#include <filesystem>

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceSession {
    public:
        InferenceSession();
        virtual ~InferenceSession();

    public:
        virtual bool open(const std::filesystem::path &path, const JsonObject &args,
                          Error *error) = 0;
        virtual bool close(Error *error) = 0;

    public:
        virtual int64_t id() const = 0;
        virtual bool isRunning() const = 0;
    };

}

#endif // SESSION_H
