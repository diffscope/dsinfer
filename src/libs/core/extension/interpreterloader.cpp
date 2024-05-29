#include "interpreterloader.h"

#ifndef _WIN32
#  include <unistd.h>
#endif

#include <map>
#include <vector>
#include <utility>

#include <loadso/library.h>

#include "dsinfer_pimpl.h"

namespace dsinfer {

#ifndef _WIN32
    static bool is_executable_by_user(const std::filesystem::path &path) {
        return ::access(path.c_str(), X_OK) == 0;
    }
#endif

    static const char *library_extension =
#if defined(_WIN32)
        ".dll"
#elif defined(__APPLE__)
        ".dylib"
#else
        ".so"
#endif
        ;

    static std::vector<std::filesystem::path>
        scan_shared_libraries(const std::filesystem::path &dir) {
        std::vector<std::filesystem::path> res;
        try {
            for (const auto &entry :
                 std::filesystem::directory_iterator(std::filesystem::canonical(dir))) {
                const auto &path = entry.path();
                if (entry.is_regular_file() && path.extension() == library_extension) {
#if defined(_WIN32) || defined(_WIN64)
                    res.push_back(path);
#else
                    if (is_executable_by_user(path)) {
                        res.push_back(path);
                    }
#endif
                }
            }
        } catch (const std::filesystem::filesystem_error &) {
            // ...
        }
        return res;
    }

    class InterpreterLoader::Impl {
    public:
        std::vector<LoadSO::Library *> libraries;
        std::map<std::string, IInterpreter *> interpreters;

        ~Impl() {
            for (const auto &item : std::as_const(libraries)) {
                delete item;
            }
        }
    };

    InterpreterLoader::InterpreterLoader() : _impl(std::make_unique<Impl>()) {
    }

    InterpreterLoader::~InterpreterLoader() = default;

    void InterpreterLoader::addLibraryPath(const std::filesystem::path &path) {
        __impl_t;

        auto shared_libraries = scan_shared_libraries(path);
        for (const auto &lib_path : std::as_const(shared_libraries)) {
            auto lib = std::make_unique<LoadSO::Library>();
            if (!lib->open(lib_path)) {
                continue;
            }

            using Entry = IInterpreter *(*) ();

            auto entry = reinterpret_cast<Entry>(lib->resolve("dsinfer_interpreter_instance"));
            if (!entry) {
                continue;
            }

            auto instance = entry();
            if (!instance) {
                continue;
            }

            if (!impl.interpreters.insert(std::make_pair(instance->key(), instance)).second) {
                continue;
            }

            printf("Loaded: %s\n", instance->key());

            impl.libraries.push_back(lib.release());
        }
    }

    IInterpreter *InterpreterLoader::find(const std::string &key) const {
        __impl_t;
        auto it = impl.interpreters.find(key);
        if (it == impl.interpreters.end())
            return nullptr;
        return it->second;
    }

}