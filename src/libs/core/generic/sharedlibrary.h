#ifndef LOADSO_LIBRARY_H
#define LOADSO_LIBRARY_H

#include <memory>
#include <filesystem>

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    class DSINFER_EXPORT SharedLibrary {
    public:
        SharedLibrary();
        ~SharedLibrary();

        SharedLibrary(SharedLibrary &&other) noexcept;
        SharedLibrary &operator=(SharedLibrary &&other) noexcept;

    public:
        enum LoadHint {
            ResolveAllSymbolsHint = 0x01,
            ExportExternalSymbolsHint = 0x02,
            LoadArchiveMemberHint = 0x04, // Unused
            PreventUnloadHint = 0x08,
            DeepBindHint = 0x10
        };

        bool open(const std::filesystem::path &path, int hints = 0);
        bool close();

        bool isOpen() const;
        std::filesystem::path path() const;

        void *handle() const;
        void *resolve(const char *name) const;

        std::string lastError() const;

        static bool isLibrary(const std::filesystem::path &path);
        static std::filesystem::path setLibraryPath(const std::filesystem::path &path);
        static std::filesystem::path locateLibraryPath(const void *addr);

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
    };

}

#endif // LOADSO_LIBRARY_H
