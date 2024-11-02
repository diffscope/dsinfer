#ifndef ACOUSTIC_LOGGER_H
#define ACOUSTIC_LOGGER_H

#include <dsinfer/log.h>

namespace dsinfer {

    static inline Log::Category acoustic_log() {
        return Log::Category("acoustic");
    }

}

#endif // ACOUSTIC_LOGGER_H
