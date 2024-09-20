#ifndef DSINFERCORE_INFERENCEMANIFEST_H
#define DSINFERCORE_INFERENCEMANIFEST_H

#include <map>
#include <vector>
#include <filesystem>

#include <dsinferCore/librarymanifestbase.h>
#include <dsinferCore/json_cxxapi.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT InferenceInfo {
    public:
        InferenceInfo();
        ~InferenceInfo();

    public:
        std::filesystem::path path() const;

    public:
        std::string id() const;
        std::string className() const;
        int level() const;

        JsonObject arguments() const;
        JsonObject internalAttributes() const;

    public:
        InferenceInfo read(const std::filesystem::path &path);

    protected:
        class Impl;
        std::shared_ptr<Impl> _impl;
    };

    class DSINFER_CORE_EXPORT InferenceManifest : public LibraryManifestBase {
    public:
        InferenceManifest(const std::filesystem::path &path);

    public:
        std::vector<InferenceInfo> inferences() const;

    protected:
        class Impl;
    };

}

#endif // DSINFERCORE_INFERENCEMANIFEST_H
