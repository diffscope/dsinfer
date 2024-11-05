#ifndef PHONEMEDICTIONARY_H
#define PHONEMEDICTIONARY_H

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <dsutils/dsutilsglobal.h>

namespace dsutils {

    class DSUTILS_EXPORT PhonemeDictionary {
    public:
        PhonemeDictionary() = default;
        ~PhonemeDictionary() = default;

        bool load(const std::filesystem::path &path, std::string *error = nullptr);
        const auto &get() const {
            return m_map;
        }

        struct Entry {
            int offset;
            int count;
        };
        void readEntry(Entry entry, std::string_view out[], int cnt) const;

    protected:
        std::vector<char> m_filebuf;
        std::unordered_map<std::string_view, Entry> m_map;
    };
}

#endif // PHONEMEDICTIONARY_H
