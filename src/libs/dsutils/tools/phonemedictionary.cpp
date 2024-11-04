#include "phonemedictionary.h"

#include <fstream>

namespace dsutils {

    void PhonemeDictionary::Entry::read(std::string_view out[], int cnt) const {
        auto p = m_buf;
        auto q = m_buf;
        int i = 0;
        int min = std::min(cnt, m_cnt);
        while (i < min) {
            if (*q == '\0') {
                out[i++] = p;
                p = q + 1;
            }
            q++;
        }
    }

    bool PhonemeDictionary::load(const std::filesystem::path &path, std::string *error) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            if (error)
                *error = "failed to open file";
            return false;
        }

        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        m_filebuf.resize(size + 1); // +1 for null terminator
        if (!file.read(m_filebuf.data(), size)) {
            if (error)
                *error = "failed to read file";
            return false;
        }
        m_filebuf[size] = '\n';

        // Parse the buffer
        m_map.clear();
        {
            auto buffer_begin = m_filebuf.data();
            auto buffer_end = buffer_begin + m_filebuf.size();
            auto start = buffer_begin;
            while (start < buffer_end) {
                while (start < buffer_end && (*start == '\r' || *start == '\n')) {
                    *start = '\0';
                    start++;
                }

                char *value_start = nullptr;
                int value_cnt = 0;

                // Find next line break
                auto p = start + 1;
                while (p < buffer_end) {
                    switch (*p) {
                        case '\t':
                        case ' ': {
                            if (!value_start) {
                                value_start = p + 1;
                            }
                            value_cnt++;
                            *p = '\0';
                            break;
                        }
                        case '\r':
                        case '\n': {
                            *p = '\0';
                            goto out_line_loop;
                        }
                        default:
                            break;
                    }
                    ++p;
                }
            out_line_loop:

                // Value not found, maybe EOF
                if (!value_start) {
                    continue;
                }

                std::string_view key(start, value_start - 1 - start);
                m_map[key] = Entry(value_start, value_cnt);
                start = p + 1;
            }
        }
        return true;
    }

}