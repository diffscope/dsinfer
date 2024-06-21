#ifndef SESSIONBRIDGE_H
#define SESSIONBRIDGE_H

#include <filesystem>

#include <dsinferCore/dsinfercoreglobal.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT SessionBridge {
    public:
        virtual ~SessionBridge() = default;

        enum SessionAttribute {
            SA_ErrorMessage,
            SA_Load,
            SA_TaskCount,
        };

        enum TaskAttribute {
            TA_ErrorMessage,
            TA_Input,
            TA_Output,
            TA_Running,
        };

    public:
        virtual int sessionCreate(const std::filesystem::path &path) const = 0;
        virtual int sessionDestroy(int id) const = 0;
        virtual int sessionAttributeGet(int id, int attr, std::string *out) const = 0;
        virtual int sessionAttributeSet(int id, int attr, const std::string &in) const = 0;

        virtual int taskCreate(const std::string &graph) const = 0;
        virtual int taskDestroy(int id) const = 0;
        virtual int taskRun(int id) const = 0;
        virtual int taskAttributeGet(int id, int attr, std::string *out) const = 0;
        virtual int taskAttributeSet(int id, int attr, const std::string &in) const = 0;
    };

}

#endif // SESSIONBRIDGE_H
