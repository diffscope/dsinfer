#ifndef PITCHINTERPRETER_H
#define PITCHINTERPRETER_H

#include <dsinferCore/iinterpreter.h>

namespace dsinfer {

    class LibrarySpec;

    class PitchInterpreter : public IInterpreter {
    public:
        PitchInterpreter();

    public:
        const char *key() const override;
        int level() const override;

        bool load(const LibrarySpec &info, std::string *errorMessage) override;
        Inference *create() const override;
    };

}

#endif // PITCHINTERPRETER_H
