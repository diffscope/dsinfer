#ifndef DISPLAYTEXT_H
#define DISPLAYTEXT_H

#include <dsinfer/jsonvalue.h>

namespace dsinfer {

    class DSINFER_EXPORT DisplayText {
    public:
        DisplayText();
        DisplayText(const std::string &text);
        explicit DisplayText(const JsonValue &value);
        ~DisplayText();

        DisplayText(const DisplayText &rhs);
        DisplayText(DisplayText &&rhs) noexcept;

        DisplayText &operator=(const DisplayText &rhs);
        DisplayText &operator=(DisplayText &&rhs) noexcept;

        DisplayText &operator=(const std::string &text);
        DisplayText &operator=(const JsonValue &value);

    public:
        std::string text() const;
        std::string text(const std::string &locale) const;

        bool isEmpty() const;

    protected:
        class Impl;
        std::shared_ptr<Impl> _impl;
    };

}

#endif // DISPLAYTEXT_H
