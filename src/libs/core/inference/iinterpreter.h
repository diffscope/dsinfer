#ifndef IINTERPRETER_H
#define IINTERPRETER_H

#include <string>

#include <dsinferCore/libraryspec.h>
#include <dsinferCore/inference.h>

namespace dsinfer {

    class LibrarySpec;

    class DSINFER_CORE_EXPORT IInterpreter {
    public:
        IInterpreter();
        virtual ~IInterpreter();

    public:
        virtual const char *key() const = 0;
        virtual int level() const = 0;

        virtual bool load();
        virtual Inference *create() const = 0;

    protected:
        DSINFER_DISABLE_COPY(IInterpreter)
    };

}

#define DSINFER_EXPORT_INTERPRETER(T)                                                              \
    extern "C" DSINFER_CORE_DECL_EXPORT T *dsinfer_interpreter_instance() {                        \
        static T _instance;                                                                        \
        return &_instance;                                                                         \
    }

#endif // IINTERPRETER_H
