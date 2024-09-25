#ifndef SINGERSPEC_H
#define SINGERSPEC_H

#include <string>

#include <dsinfer/contributespec.h>
#include <dsinfer/inference.h>

namespace dsinfer {

    class SingerRegistry;

    class SingerImport {
    public:
        std::string libraryId;
        std::string inferenceId;
        JsonObject options;
    };

    class DSINFER_EXPORT SingerSpec : public ContributeSpec {
    public:
        ~SingerSpec();

    public:
        std::filesystem::path path() const;

        std::string name() const;
        std::filesystem::path avatar() const;
        std::filesystem::path background() const;
        std::filesystem::path demoAudio() const;

        const std::vector<SingerImport> &imports() const;

    public:
        std::vector<Inference *> createInferences(std::string *error) const;

    protected:
        class Impl;

        explicit SingerSpec(LibrarySpec *parent);

        friend class SingerRegistry;
    };

}

#endif // SINGERSPEC_H
