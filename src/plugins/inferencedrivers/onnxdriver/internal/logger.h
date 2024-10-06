#ifndef ONNXDRIVER_LOGGER_H
#define ONNXDRIVER_LOGGER_H

#include <dsinfer/log.h>

#define ONNXDRIVER_LOG_CATEGORY dsinfer::Log::Category("onnxdriver")

#define LOG_FATAL(format, ...) \
    ONNXDRIVER_LOG_CATEGORY.fatal(format, ##__VA_ARGS__)

#define LOG_CRITICAL(format, ...) \
    ONNXDRIVER_LOG_CATEGORY.critical(format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...) \
    ONNXDRIVER_LOG_CATEGORY.warning(format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
    ONNXDRIVER_LOG_CATEGORY.info(format, ##__VA_ARGS__)

#define LOG_DEBUG(format, ...) \
    ONNXDRIVER_LOG_CATEGORY.debug(format, ##__VA_ARGS__)

#define LOG_TRACE(format, ...) \
    ONNXDRIVER_LOG_CATEGORY.trace(format, ##__VA_ARGS__)

#define LOG_VERBOSE(format, ...) \
    ONNXDRIVER_LOG_CATEGORY.verbose(format, ##__VA_ARGS__)

#endif // ONNXDRIVER_LOGGER_H