#ifndef DSINFER_ORT_LIBRARY_H
#define DSINFER_ORT_LIBRARY_H

#include <string>
#include <dsinfer/dsinfer_cxxapi.h>

namespace dsinfer {

    Status loadOrtLibrary(const std::string &path);

    bool isOrtLoaded();

    std::string getOrtVersionString();

} // namespace dsinfer

#endif // DSINFER_ORT_LIBRARY_H
