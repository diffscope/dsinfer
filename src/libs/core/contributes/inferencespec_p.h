#ifndef INFERENCESPEC_P_H
#define INFERENCESPEC_P_H

#include <dsinfer/inferencespec.h>
#include <dsinfer/inferenceinterpreter.h>
#include <dsinfer/private/contributespec_p.h>

namespace dsinfer {

    class InferenceSpec::Impl : public ContributeSpec::Impl {
    public:
        Impl() : ContributeSpec::Impl(ContributeSpec::Inference) {
        }

        bool read(const std::filesystem::path &basePath, const JsonObject &obj,
                  Error *error) override;

        std::filesystem::path path;

        std::string id;
        std::string className;

        DisplayText name;
        int apiLevel = 0;

        JsonObject schema;
        JsonObject configuration;

        InferenceInterpreter *interp = nullptr;
    };

}

#endif // INFERENCESPEC_P_H
