#ifndef SINGERSPEC_H
#define SINGERSPEC_H

#include <string>

#include <dsinfer/displaytext.h>
#include <dsinfer/contributespec.h>
#include <dsinfer/inference.h>

namespace dsinfer {

    class SingerRegistry;

    class SingerImport {
    public:
        ContributeIdentifier inference;
        JsonValue options;
        // std::vector<std::string> roles;
    };

    class DSINFER_EXPORT SingerSpec : public ContributeSpec {
    public:
        ~SingerSpec();

    public:
        std::filesystem::path path() const;

        std::string model() const;
        DisplayText name() const;

        std::filesystem::path avatar() const;
        std::filesystem::path background() const;
        std::filesystem::path demoAudio() const;

        const std::vector<SingerImport> &imports() const;

        JsonObject configuration() const;

    protected:
        class Impl;
        SingerSpec();

        friend class SingerRegistry;
    };

}

#endif // SINGERSPEC_H
