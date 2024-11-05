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
        PhonemeDictionary() {
        }

        struct Entry {
            Entry() : m_off(0), m_cnt(0) {
            }
            inline int offset() {
                return m_off;
            }
            inline int count() const {
                return m_cnt;
            }

        private:
            inline Entry(int off, int cnt) : m_off(off), m_cnt(cnt) {
            }
            int m_off;
            int m_cnt;
            friend class PhonemeDictionary;
        };

        bool load(const std::filesystem::path &path, std::string *error = nullptr);

        const auto &get() const {
            return m_map;
        }

        void readEntry(Entry entry, std::string_view out[], int cnt) const;

    protected:
        std::vector<char> m_filebuf;
        std::unordered_map<std::string_view, Entry> m_map;
    };
}

#endif // PHONEMEDICTIONARY_H
