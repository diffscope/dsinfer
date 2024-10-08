#ifndef DSINFERCOREGLOBAL_H
#define DSINFERCOREGLOBAL_H

#include <memory>

#ifdef _MSC_VER
#  define DSINFER_DECL_EXPORT __declspec(dllexport)
#  define DSINFER_DECL_IMPORT __declspec(dllimport)
#else
#  define DSINFER_DECL_EXPORT __attribute__((visibility("default")))
#  define DSINFER_DECL_IMPORT __attribute__((visibility("default")))
#endif

#ifndef DSINFER_EXPORT
#  ifdef DSINFER_STATIC
#    define DSINFER_EXPORT
#  else
#    ifdef DSINFER_LIBRARY
#      define DSINFER_EXPORT DSINFER_DECL_EXPORT
#    else
#      define DSINFER_EXPORT DSINFER_DECL_IMPORT
#    endif
#  endif
#endif

#define DSINFER_DISABLE_COPY(Class)                                                                \
    Class(const Class &) = delete;                                                                 \
    Class &operator=(const Class &) = delete;

#define DSINFER_DISABLE_MOVE(Class)                                                                \
    Class(Class &&) = delete;                                                                      \
    Class &operator=(Class &&) = delete;

#define DSINFER_DISABLE_COPY_MOVE(Class)                                                           \
    DSINFER_DISABLE_COPY(Class)                                                                    \
    DSINFER_DISABLE_MOVE(Class)

namespace dsinfer_private {

    // Unique Data
    template <class T, class T1 = T>
    inline const T *get(const std::unique_ptr<T1> &d) {
        return static_cast<const T *>(d.get());
    }

    template <class T, class T1 = T>
    inline T *get(std::unique_ptr<T1> &d) {
        return static_cast<T *>(d.get());
    }

    // Shared Data
    template <class T, class T1 = T>
    inline const T *get(const std::shared_ptr<T1> &d) {
        return static_cast<const T *>(d.get());
    }

    template <class T, class T1 = T>
    inline T *get(std::shared_ptr<T1> &d) {
        if (d.use_count() > 1) {
            d = std::make_shared<T>(*static_cast<T *>(d.get())); // detach
        }
        return static_cast<T *>(d.get());
    }

}

#define __dsinfer_impl_get(T) ::dsinfer_private::get<std::remove_const_t<T::Impl>>(_impl)
#define __dsinfer_decl_get(T) static_cast<T *>(_decl)

#define __dsinfer_impl(T) auto &impl = *__dsinfer_impl_get(T)
#define __dsinfer_decl(T) auto &decl = *__dsinfer_decl_get(T)

#define __dsinfer_impl_t __dsinfer_impl(std::remove_pointer_t<decltype(this)>)
#define __dsinfer_decl_t __dsinfer_decl(Decl)


#define DSINFER_DECLARE_CAST_FUNCTIONS(CLASS_NAME)                                                 \
public:                                                                                            \
    template <class T>                                                                             \
    inline constexpr T *cast() {                                                                   \
        static_assert(std::is_base_of<CLASS_NAME, T>::value,                                       \
                      "T should inherit from " #CLASS_NAME);                                       \
        return static_cast<T *>(this);                                                             \
    }                                                                                              \
                                                                                                   \
    template <class T>                                                                             \
    inline constexpr const T *cast() const {                                                       \
        static_assert(std::is_base_of<CLASS_NAME, T>::value,                                       \
                      "T should inherit from " #CLASS_NAME);                                       \
        return static_cast<T *>(this);                                                             \
    }

#ifndef _TSTR
#  ifdef _WIN32
#    define _TSTR(T) L##T
#  else
#    define _TSTR(T) T
#  endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define DSINFER_PRINTF_FORMAT(fmtpos, attrpos)                                                   \
      __attribute__((__format__(__printf__, fmtpos, attrpos)))
#else
#  define DSINFER_PRINTF_FORMAT(fmtpos, attrpos)
#endif

#endif // DSINFERCOREGLOBAL_H
