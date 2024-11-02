#ifndef ONNXSESSION_H
#define ONNXSESSION_H

#include <dsinfer/inferencesession.h>

namespace dsinfer {

    class OnnxSession : public InferenceSession {
    public:
        OnnxSession();
        ~OnnxSession();

    public:
        bool open(const std::filesystem::path &path, const JsonObject &args, Error *error) override;
        bool close(Error *error) override;

    public:
        int64_t id() const override;
        static OnnxSession *getSession(int64_t sessionId);

        bool isRunning() const override;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;

        friend class OnnxTask;
    };

}

#endif // ONNXSESSION_H
