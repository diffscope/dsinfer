#ifndef TST_ONNXDRIVER_VALUEUTILS_H
#define TST_ONNXDRIVER_VALUEUTILS_H

#include <filesystem>

#include <dsinfer/log.h>
#include <dsinfer/jsonvalue.h>

class ValueUtils {
public:
    explicit ValueUtils(dsinfer::Log::Category *logger_);
    ~ValueUtils();

    dsinfer::JsonValue jsonValueFromPath(const std::filesystem::path &path);
    std::string inferValueStringify(const dsinfer::JsonValue &value);
private:
    dsinfer::Log::Category *logger;
};

#endif // TST_ONNXDRIVER_VALUEUTILS_H