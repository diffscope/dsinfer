#ifndef IINTERPRETER_H
#define IINTERPRETER_H

#include <string>

#include <dsinferCore/libraryinfo.h>

namespace dsinfer {

    class LibraryInfo;

    class DSINFER_CORE_EXPORT IInterpreter {
    public:
        IInterpreter();
        virtual ~IInterpreter();

        IInterpreter(const IInterpreter &) = delete;
        IInterpreter &operator=(const IInterpreter &) = delete;

    public:
        virtual const char *key() const = 0;
        virtual int level() const = 0;

        virtual bool load(const LibraryInfo &info, std::string *errorMessage);
    };

}

#define DSINFER_EXPORT_INTERPRETER(T)                                                              \
    extern "C" DSINFER_CORE_DECL_EXPORT T *dsinfer_interpreter_instance() {                        \
        static T _instance;                                                                        \
        return &_instance;                                                                         \
    }

#endif // IINTERPRETER_H
