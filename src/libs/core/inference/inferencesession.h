#ifndef DSINFER_INFERENCESESSION_H
#define DSINFER_INFERENCESESSION_H

#include <filesystem>

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer {

    class DSINFER_EXPORT InferenceSession {
    public:
        InferenceSession();
        virtual ~InferenceSession();

    public:
        virtual bool open(const std::filesystem::path &path, const JsonValue &args,
                          Error *error) = 0;
        virtual bool close(Error *error) = 0;
        virtual bool isOpen() const = 0;

    public:
        virtual int64_t id() const = 0;
        virtual bool isRunning() const = 0;
    };

}

#endif // DSINFER_INFERENCESESSION_H
