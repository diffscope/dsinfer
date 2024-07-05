#ifndef ACOUSTICINTERPRETER_H
#define ACOUSTICINTERPRETER_H

#include <dsinferCore/iinterpreter.h>

namespace dsinfer {

    class AcousticInterpreter : public IInterpreter {
    public:
        AcousticInterpreter();

    public:
        const char *key() const override;
        int level() const override;

        bool load(const LibrarySpec &info, std::string *errorMessage) override;
        Inference *create() const override;
    };

}

#endif // ACOUSTICINTERPRETER_H
