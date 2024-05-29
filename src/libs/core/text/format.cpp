#include "format.h"

#ifdef _WIN32
#include <loadso/system.h>
#endif

namespace dsinfer {

#ifdef _WIN32
    std::string wide2utf8(const std::wstring &s) {
        return LoadSO::System::MultiFromPathString(s);
    }
#endif

    std::string formatText(const std::string &format, const std::vector<std::string> &args) {
        std::string result = format;
        for (size_t i = 0; i < args.size(); i++) {
            std::string placeholder = "%" + std::to_string(i + 1);
            size_t pos = result.find(placeholder);
            while (pos != std::string::npos) {
                result.replace(pos, placeholder.length(), args[i]);
                pos = result.find(placeholder, pos + args[i].size());
            }
        }
        return result;
    }

}