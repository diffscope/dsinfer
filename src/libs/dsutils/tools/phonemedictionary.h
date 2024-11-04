#ifndef PHONEMEDICTIONARY_H
#define PHONEMEDICTIONARY_H

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <dsutils/dsutilsglobal.h>

namespace dsutils {

    class DSUTILS_EXPORT PhonemeDictionary {
    public:
        PhonemeDictionary() {
        }

        struct Entry {
            Entry() : m_buf(nullptr), m_cnt(0) {
            }
            inline const char *buffer() {
                return m_buf;
            }
            inline int count() const {
                return m_cnt;
            }
            void read(std::string_view out[], int cnt) const;

        private:
            inline Entry(const char *buf, int cnt) : m_buf(buf), m_cnt(cnt) {
            }
            const char *m_buf;
            int m_cnt;
            friend class PhonemeDictionary;
        };

        bool load(const std::filesystem::path &path, std::string *error = nullptr);

        const auto &get() const {
            return m_map;
        }

    protected:
        std::vector<char> m_filebuf;
        std::unordered_map<std::string_view, Entry> m_map;
    };

}

#endif // PHONEMEDICTIONARY_H
