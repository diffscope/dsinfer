#ifndef INTERPRETERLOADER_P_H
#define INTERPRETERLOADER_P_H

#include <memory>
#include <filesystem>

#include <dsinferCore/iinterpreter.h>

namespace dsinfer {

    class InterpreterLoader {
    public:
        InterpreterLoader();
        ~InterpreterLoader();

    public:
        void addLibraryPath(const std::filesystem::path &path);

    public:
        IInterpreter *find(const std::string &key) const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
        
        DSINFER_DISABLE_COPY(InterpreterLoader)
    };

}

#endif // INTERPRETERLOADER_P_H
