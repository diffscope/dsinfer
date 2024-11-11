#ifndef PHONEMEDICTIONARY_H
#define PHONEMEDICTIONARY_H

#include <filesystem>
#include <string>
#include <string_view>

#include <stdcorelib/vla.h>

#include <dsutils/dsutilsglobal.h>

namespace dsutils {

    class DSUTILS_EXPORT PhonemeDictionary {
    public:
        PhonemeDictionary();
        ~PhonemeDictionary();

        bool load(const std::filesystem::path &path, std::string *error = nullptr);
        stdc::VarLengthArray<std::string_view> find(const char *key) const;

        void print_front(int size) const;

    protected:
        class Impl;
        std::shared_ptr<Impl> _impl;
    };
}

#endif // PHONEMEDICTIONARY_H
