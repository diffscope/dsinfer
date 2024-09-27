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

}

#endif // ALGORITHMS_H
