#ifndef DSINFER_ONNXDRIVER_ONNXDRIVER_LOGGER_H
#define DSINFER_ONNXDRIVER_ONNXDRIVER_LOGGER_H

#include <dsinfer/log.h>

namespace dsinfer {

    static inline Log::Category onnxdriver_log() {
        return Log::Category("onnxdriver");
    }

}

#endif // DSINFER_ONNXDRIVER_ONNXDRIVER_LOGGER_H
