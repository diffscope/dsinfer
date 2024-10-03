#ifndef CONTRIBUTESPEC_H
#define CONTRIBUTESPEC_H

#include <string>
#include <filesystem>
#include <utility>

#include <dsinfer/jsonvalue.h>
#include <dsinfer/libraryspec.h>

namespace dsinfer {

    class DSINFER_EXPORT ContributeIdentifier {
    public:
        inline ContributeIdentifier(std::string parent, const VersionNumber &version,
                                    std::string id)
            : m_library(std::move(parent)), m_version(version), m_id(std::move(id)) {
        }

        inline ContributeIdentifier(std::string parent, std::string id)
            : m_library(std::move(parent)), m_id(std::move(id)) {
        }

        inline ContributeIdentifier(std::string id) : m_id(std::move(id)) {
        }

        inline ContributeIdentifier() = default;

        inline std::string library() const {
            return m_library;
        }

        inline VersionNumber version() const {
            return m_version;
        }

        inline std::string id() const {
            return m_id;
        }

        inline bool isEmpty() const {
            return m_id.empty();
        }

        std::string toString() const;
        static ContributeIdentifier fromString(const std::string &token);

        static bool isValidId(const std::string &id);

    protected:
        std::string m_library;
        VersionNumber m_version;
        std::string m_id;
    };

    class LibrarySpec;

    class ContributeRegistry;

    class DSINFER_EXPORT ContributeSpec {
    public:
        enum Type {
            Inference,
            Singer,
        };

        enum State {
            Invalid,
            Initialized,
            Ready,
            Finished,
            Deleted,
        };

        virtual ~ContributeSpec();

    public:
        int type() const;
        State state() const;
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
        friend class Environment;
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
