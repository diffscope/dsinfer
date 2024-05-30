#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <filesystem>

#include <dsinferCore/dsinfercoreglobal.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT Session {
    public:
        Session();
        ~Session();

    public:
        bool load(const std::filesystem::path &path, std::string *errorMessage);
    };

}

#endif // SESSION_H
