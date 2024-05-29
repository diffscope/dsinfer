#ifndef DSINFER_PIMPL_H
#define DSINFER_PIMPL_H

#include <memory>
#include <type_traits>

namespace __pimpl_private {

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

#define __pimpl_get_impl(T) ::__pimpl_private::get<std::remove_const_t<T::Impl>>(_impl)
#define __pimpl_get_decl(T) static_cast<T *>(_decl)

#define __impl(T) auto &impl = *__pimpl_get_impl(T)
#define __decl(T) auto &decl = *__pimpl_get_decl(T)

#define __impl_t __impl(std::remove_pointer_t<decltype(this)>)
#define __decl_t __decl(Decl)

#endif // DSINFER_PIMPL_H
