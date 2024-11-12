#include "utils.h"

namespace fs = std::filesystem;

namespace cli {

    fs::path home_dir() {
        return
#ifdef WIN32
            _wgetenv(L"USERPROFILE")
#else
            getenv("HOME")
#endif
                ;
    }

}