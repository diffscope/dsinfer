#ifndef PITCHINTERPRETER_H
#define PITCHINTERPRETER_H

#include <dsinferCore/iinterpreter.h>

namespace dsinfer {

    class LibraryInfo;

    class PitchInterpreter : public IInterpreter {
    public:
        PitchInterpreter();

    public:
        const char *key() const override;
        int level() const override;

        bool load(const LibraryInfo &info, std::string *errorMessage) override;
    };

}

#endif // PITCHINTERPRETER_H
