#ifndef CONTRIBUTEREGISTRY_H
#define CONTRIBUTEREGISTRY_H

#include <filesystem>

#include <dsinfer/contributespec.h>

namespace dsinfer {

    class DSINFER_EXPORT ContributeRegistry {
    public:
        virtual ~ContributeRegistry();

    public:
        Environment *env() const;

    protected:
        virtual std::string specKey() const = 0;
        virtual ContributeSpec *parseSpec(const std::filesystem::path &basePath,
                                          const JsonObject &config, Error *error) const = 0;
        virtual void unload(ContributeSpec *spec) = 0;

    public:
        template <class T>
        inline constexpr T *cast();

        template <class T>
        inline constexpr const T *cast() const;

    protected:
        class Impl;
        std::unique_ptr<Impl> _impl;
        explicit ContributeRegistry(Impl &impl);

        friend class Environment;
    };

    template <class T>
    inline constexpr T *ContributeRegistry::cast() {
        static_assert(std::is_base_of<ContributeRegistry, T>::value,
                      "T should inherit from dsinfer::ContributeRegistry");
        return static_cast<T *>(this);
    }

    template <class T>
    inline constexpr const T *ContributeRegistry::cast() const {
        static_assert(std::is_base_of<ContributeRegistry, T>::value,
                      "T should inherit from dsinfer::ContributeRegistry");
        return static_cast<T *>(this);
    }

}

#endif // CONTRIBUTEREGISTRY_H
