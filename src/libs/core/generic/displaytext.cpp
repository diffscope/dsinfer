#include "displaytext.h"

#include <map>
#include <optional>

namespace dsinfer {

    class DisplayText::Impl {
    public:
        std::string defaultText;
        std::optional<std::map<std::string, std::string>> texts;

        void assign(const JsonValue &value) {
            if (value.isString()) {
                defaultText = value.toString();
                return;
            }
            if (!value.isObject()) {
                return;
            }
            const auto &obj = value.toObject();
            std::string defaultText_;
            std::map<std::string, std::string> texts_;
            for (const auto &item : obj) {
                if (item.first == "_") {
                    defaultText = item.second.toString();
                    continue;
                }
                texts_[item.first] = item.second.toString();
            }

            if (!texts_.empty()) {
                static const char *candidates[] = {
                    "en", "en_US", "en_us", "en_GB", "en_gb",
                };
                for (const auto &item : candidates) {
                    if (!defaultText_.empty()) {
                        break;
                    }
                    auto it = texts_.find(item);
                    if (it != texts_.end()) {
                        defaultText_ = it->second;
                    }
                }
                if (defaultText_.empty()) {
                    return;
                }
                texts = std::move(texts_);
            }
            defaultText = std::move(defaultText_);
        }
    };

    DisplayText::DisplayText() : _impl(new Impl()) {
    }

    DisplayText::DisplayText(const std::string &text) : _impl(new Impl()) {
        _impl->defaultText = text;
    }

    DisplayText::DisplayText(const JsonValue &value) : _impl(new Impl()) {
        _impl->assign(value);
    }

    DisplayText::DisplayText(const std::string &defaultText,
                             const std::map<std::string, std::string> &texts)
        : _impl(new Impl()) {
        _impl->defaultText = defaultText;
        _impl->texts = texts;
    }

    DisplayText::~DisplayText() = default;

    DisplayText::DisplayText(const DisplayText &rhs) = default;

    DisplayText::DisplayText(DisplayText &&rhs) noexcept = default;

    DisplayText &DisplayText::operator=(const DisplayText &rhs) = default;

    DisplayText &DisplayText::operator=(DisplayText &&rhs) noexcept = default;

    DisplayText &DisplayText::operator=(const std::string &text) {
        __stdc_impl_t;
        impl.defaultText = text;
        return *this;
    }

    DisplayText &DisplayText::operator=(const JsonValue &value) {
        __stdc_impl_t;
        impl.assign(value);
        return *this;
    }

    std::string DisplayText::text() const {
        return _impl->defaultText;
    }

    std::string DisplayText::text(const std::string &locale) const {
        if (!_impl->texts) {
            return {};
        }
        auto it = _impl->texts->find(locale);
        if (it == _impl->texts->end()) {
            return {};
        }
        return it->second;
    }

    bool DisplayText::isEmpty() const {
        return _impl->defaultText.empty();
    }
}