#include "phonemedictionary.h"

#include <fstream>
#include <algorithm>

#include <stdcorelib/pimpl.h>
#include <stdcorelib/console.h>

#include <sparsepp/spp.h>

namespace dsutils {

    struct const_char_hash {
    public:
        size_t operator()(const char *key) const noexcept {
            return std::hash<std::string_view>()(std::string_view(key, std::strlen(key)));
        }
    };

    class PhonemeDictionary::Impl {
    public:
        struct Entry {
            int offset;
            int count;
        };
        std::vector<char> filebuf;
        spp::sparse_hash_map<char *, Entry, const_char_hash> map;

        void readEntry(Entry entry, std::string_view out[]) const {
            auto p = filebuf.data() + entry.offset;
            auto q = p;
            int i = 0;
            while (i < entry.count) {
                if (*q == '\0') {
                    out[i++] = p;
                    p = q + 1;
                }
                q++;
            }
        }
    };

    PhonemeDictionary::PhonemeDictionary() : _impl(std::make_shared<Impl>()) {
    }

    PhonemeDictionary::~PhonemeDictionary() = default;

    bool PhonemeDictionary::load(const std::filesystem::path &path, std::string *error) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            if (error)
                *error = "failed to open file";
            return false;
        }

        __stdc_impl_t;
        auto &filebuf = impl.filebuf;
        auto &map = impl.map;

        file.seekg(0, std::ios::end);
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        filebuf.resize(file_size + 1); // +1 for terminator
        if (!file.read(filebuf.data(), file_size)) {
            if (error)
                *error = "failed to read file";
            filebuf.clear();
            return false;
        }
        filebuf[file_size] = '\n'; // add terminating line break
        map.clear();

        // Parse the buffer
        const auto buffer_begin = filebuf.data();
        const auto buffer_end = buffer_begin + filebuf.size();

        // Estimate line numbers if the file is too large
        static constexpr const size_t larget_file_size = 1 * 1024 * 1024;
        if (file_size > larget_file_size) {
            size_t line_cnt = std::count(buffer_begin, buffer_end, '\n') + 1;
            map.reserve(line_cnt);
        }

        // Traverse lines
        {
            auto start = buffer_begin;
            while (start < buffer_end) {
                while (start < buffer_end && (*start == '\r' || *start == '\n')) {
                    *start = '\0';
                    start++;
                }

                char *value_start = nullptr;
                int value_cnt = 0;

                // Find tab
                auto p = start + 1;
                while (p < buffer_end) {
                    switch (*p) {
                        case '\t':
                            value_start = p + 1;
                            *p = '\0';
                            goto out_tab_find;

                        case '\r':
                        case '\n':
                            goto out_next_line;

                        default:
                            break;
                    }
                    ++p;
                }

                // Tab not found
                goto out_next_line;

            out_tab_find:
                // Find space or line break
                while (p < buffer_end) {
                    switch (*p) {
                        case ' ':
                            value_cnt++;
                            *p = '\0';
                            break;

                        case '\r':
                        case '\n':
                            value_cnt++;
                            *p = '\0';
                            goto out_success;

                        default:
                            break;
                    }
                    ++p;
                }

            out_success: {
                // std::string_sview key(start, value_start - 1 - start);
                map[start] = Impl::Entry{int(value_start - buffer_begin), value_cnt};
                start = p + 1;
            }
            out_next_line: {}
            }
        }
        return true;
    }

    stdc::vlarray<std::string_view> PhonemeDictionary::find(const char *key) const {
        __stdc_impl_t;
        auto &filebuf = impl.filebuf;
        auto &map = impl.map;

        auto it = map.find(const_cast<char *>(key));
        if (it == map.end()) {
            return {};
        }
        auto &entry = it->second;
        stdc::vlarray<std::string_view> out(entry.count);
        impl.readEntry(it->second, out.data());
        return out;
    }

    static std::string join(std::string_view v[], int size, const std::string_view &delimiter) {
        if (size == 0) {
            return {};
        }
        std::string res;
        for (int i = 0; i < size - 1; ++i) {
            res.append(v[i]);
            res.append(delimiter);
        }
        res.append(v[size - 1]);
        return res;
    }

    void PhonemeDictionary::print_front(int size) const {
        __stdc_impl_t;
        auto &map = impl.map;
        size = std::min<int>(size, map.size());

        auto it = map.begin();
        for (int i = 0; i < size; ++i, ++it) {
            const auto &entry = it->second;
            stdc::vlarray<std::string_view> values(entry.count);
            impl.readEntry(entry, values.data());
            stdc::u8println("Phoneme %1: %2 - %3", i, it->first,
                            join(values.data(), values.size(), " ").c_str());
        }
    }

}