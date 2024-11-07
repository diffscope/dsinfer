#ifndef PHONEMEDICTIONARY_H
#define PHONEMEDICTIONARY_H

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include <sparsepp/spp.h>

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
        void readEntry(Entry entry, std::string_view out[]) const;

        struct hash {
            size_t operator()(const char *key) const noexcept {
                return std::hash<std::string_view>()(std::string_view(key, std::strlen(key)));
            }
        };

    protected:
        std::vector<char> m_filebuf;

        spp::sparse_hash_map<char *, Entry, hash> m_map;
    };
}

#endif // PHONEMEDICTIONARY_H
