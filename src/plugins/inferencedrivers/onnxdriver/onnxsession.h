#ifndef ONNXSESSION_H
#define ONNXSESSION_H

#include <dsinfer/inferencesession.h>

namespace dsinfer {

    class OnnxSession : public InferenceSession {
    public:
        OnnxSession();
        ~OnnxSession();

    public:
        bool open(const std::filesystem::path &path, const JsonObject &args,
                  Error *error) override;
        bool close(Error *error) override;

    public:
        int64_t id() const override;
        bool isRunning() const override;
    };

}

#endif // ONNXSESSION_H
