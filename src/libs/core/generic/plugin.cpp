#include "plugin.h"

#include "sharedlibrary.h"

namespace dsinfer {

#ifdef _WIN32
#endif

    Plugin::~Plugin() = default;

    std::filesystem::path Plugin::path() const {
        return SharedLibrary::locateLibraryPath(this);
    }

}