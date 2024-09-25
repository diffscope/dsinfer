#ifndef CONTRIBUTESPEC_H
#define CONTRIBUTESPEC_H

#include <string>
#include <filesystem>

#include <dsinfer/jsonvalue.h>
#include <dsinfer/libraryspec.h>

namespace dsinfer {

    class LibrarySpec;

    class ContributeRegistry;

    class DSINFER_EXPORT ContributeSpec {
    public:
        enum ContributeType {
            CT_Inference,
            CT_Singer,
        };

        virtual ~ContributeSpec();

    public:
        int type() const;
        LibrarySpec *parent() const;
        inline Environment *env() const {
            return parent()->env();
        }

        std::string id() const;

    public:
        template <class T>
        inline constexpr T *cast();

        template <class T>
        inline constexpr const T *cast() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
        explicit ContributeSpec(Impl &impl);

        friend class ContributeRegistry;
    };

    template <class T>
    inline constexpr T *ContributeSpec::cast() {
        static_assert(std::is_base_of<ContributeSpec, T>::value,
                      "T should inherit from dsinfer::ContributeSpec");
        return static_cast<T *>(this);
    }

    template <class T>
    inline constexpr const T *ContributeSpec::cast() const {
        static_assert(std::is_base_of<ContributeSpec, T>::value,
                      "T should inherit from dsinfer::ContributeSpec");
        return static_cast<T *>(this);
    }

}

#endif // CONTRIBUTESPEC_H
