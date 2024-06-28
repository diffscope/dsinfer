#ifndef INFERENCEINFO_H
#define INFERENCEINFO_H

#include <vector>
#include <memory>

#include <dsinferCore/dsinfercoreglobal.h>
#include <dsinferCore/common.h>

namespace dsinfer {

    class DSINFER_CORE_EXPORT InferenceInfo {
    public:
        InferenceInfo();
        ~InferenceInfo();

    public:
        bool fromCborData(const cbor_data &data);
        cbor_data toCborData() const;

    public:
        std::string className() const;
        void setClassName(const std::string &className);

        int level() const;
        void setLevel(int level);

        cbor_data attributes() const;
        void setAttributes(const cbor_data &attributes);

    protected:
        class Impl;
        std::shared_ptr<Impl> _impl;
    };

}

#endif // INFERENCEINFO_H
