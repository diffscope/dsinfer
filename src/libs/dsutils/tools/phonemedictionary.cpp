#include "phonemedictionary.h"

#include <fstream>
#include <algorithm>

namespace dsutils {

    bool PhonemeDictionary::load(const std::filesystem::path &path, std::string *error) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            if (error)
                *error = "failed to open file";
            return false;
        }

        file.seekg(0, std::ios::end);
        std::streamsize file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        m_filebuf.resize(file_size + 1); // +1 for terminator
        if (!file.read(m_filebuf.data(), file_size)) {
            if (error)
                *error = "failed to read file";
            m_filebuf.clear();
            return false;
        }
        m_filebuf[file_size] = '\n'; // add terminating line break
        m_map.clear();

        // Parse the buffer
        const auto buffer_begin = m_filebuf.data();
        const auto buffer_end = buffer_begin + m_filebuf.size();

        // Estimate line numbers if the file is too large
        static constexpr const size_t larget_file_size = 1 * 1024 * 1024;
        if (file_size > larget_file_size) {
            size_t line_cnt = std::count(buffer_begin, buffer_end, '\n') + 1;
            m_map.reserve(line_cnt);
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
                            goto out_failed;

                        default:
                            break;
                    }
                    ++p;
                }

                // Tab not found
                if (!value_start) {
                    goto out_failed;
                }

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
                std::string_view key(start, value_start - 1 - start);
                m_map[key] = Entry{int(value_start - buffer_begin), value_cnt};
                start = p + 1;
            }

            out_failed: {}
            }
        }
        return true;
    }

    void PhonemeDictionary::readEntry(Entry entry, std::string_view out[]) const {
        auto p = m_filebuf.data() + entry.offset;
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

}