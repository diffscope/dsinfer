#ifndef LIBRARYSPEC_P_H
#define LIBRARYSPEC_P_H

#include <map>

#include <dsinfer/jsonvalue.h>
#include <dsinfer/libraryspec.h>

namespace dsinfer {

    class LibrarySpec::Impl {
    public:
        explicit Impl(LibrarySpec *decl, Environment *env) : _decl(decl), env(env) {
        }
        ~Impl();

        using Decl = LibrarySpec;
        LibrarySpec *_decl;

    public:
        bool parse(const std::filesystem::path &dir,
                   const std::map<std::string, ContributeRegistry *> &regs,
                   std::vector<ContributeSpec *> *outContributes, Error *error);

        static bool readDesc(const std::filesystem::path &dir, JsonObject *out, Error *error);

        static std::filesystem::path
            searchDependency(const std::vector<std::filesystem::path> &paths,
                             const LibraryDependency &dep);

        Environment *env;

        std::filesystem::path path;
        std::string id;

        VersionNumber version;
        VersionNumber compatVersion;

        std::string description;
        std::string vendor;
        std::string copyright;
        std::string url;

        std::map<int, std::map<std::string, ContributeSpec *>> contributes;

        std::vector<LibraryDependency> dependencies;

        // state
        Error err;
        bool loaded = false;
    };

}

#endif // LIBRARYSPEC_P_H
