#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <dsinfer/dsinferglobal.h>

namespace dsinfer {

    template <typename ForwardIterator>
    void deleteAll(ForwardIterator begin, ForwardIterator end) {
        while (begin != end) {
            delete *begin;
            ++begin;
        }
    }

    template <typename Container>
    inline void deleteAll(const Container &c) {
        deleteAll(c.begin(), c.end());
    }

    inline constexpr size_t hash(size_t key, size_t seed = 0) noexcept {
        return size_t(key & (~0U)) ^ seed;
    }

}

#endif // ALGORITHMS_H
