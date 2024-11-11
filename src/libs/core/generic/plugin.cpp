#include "plugin.h"

#include <stdcorelib/library.h>

namespace dsinfer {

#ifdef _WIN32
#endif

    /*!
        Destroys a plugin instance.
    */
    Plugin::~Plugin() = default;

    /*!
        \fn const char *Plugin::iid() const;

        Returns the plugin interface id.
    */

    /*!
        \fn const char *Plugin::key() const;

        Returns the plugin key.
    */

    /*!
        Returns the plugin path.
    */
    std::filesystem::path Plugin::path() const {
        return stdc::Library::locateLibraryPath(this);
    }

}