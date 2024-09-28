#ifndef INFERENCESPEC_H
#define INFERENCESPEC_H

#include <string>
#include <filesystem>

#include <dsinfer/contributespec.h>
#include <dsinfer/jsonvalue.h>
#include <dsinfer/inference.h>

namespace dsinfer {

    class InferenceRegistry;

    class DSINFER_EXPORT InferenceSpec : public ContributeSpec {
    public:
        ~InferenceSpec();

    public:
        std::filesystem::path path() const;

        std::string className() const;
        std::string name() const;
        int apiLevel() const;

        JsonObject schema() const;
        JsonObject configuration() const;

    public:
        class Inference *create(const JsonObject &options, Error *error) const;
        bool validate(const JsonValue &options, std::string *error) const;

    protected:
        class Impl;
        InferenceSpec();

        friend class InferenceRegistry;
    };

}


#endif // INFERENCESPEC_H
