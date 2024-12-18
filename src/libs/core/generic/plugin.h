#ifndef DSINFER_PLUGIN_H
#define DSINFER_PLUGIN_H

#include <filesystem>

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    class DSINFER_EXPORT Plugin {
    public:
        virtual ~Plugin();

    public:
        virtual const char *iid() const = 0;
        virtual const char *key() const = 0;

    public:
        std::filesystem::path path() const;
    };

}

#define DSINFER_EXPORT_PLUGIN(PLUGIN_NAME)                                                         \
    extern "C" STDCORELIB_DECL_EXPORT dsinfer::Plugin *dsinfer_plugin_instance() {                 \
        static PLUGIN_NAME _instance;                                                              \
        return &_instance;                                                                         \
    }

#endif // DSINFER_PLUGIN_H