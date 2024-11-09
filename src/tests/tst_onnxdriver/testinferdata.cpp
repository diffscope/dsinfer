#include "testinferdata.h"

#include <array>

namespace DS = dsinfer;

enum class DataFormat {
    Array,
    Bytes,
    Reference,
};


template <typename T>
static inline const char *type_to_string() {
    static_assert(std::is_same_v<T, float> ||
                      std::is_same_v<T, int64_t> ||
                      std::is_same_v<T, bool>,
                  "T must be one of the following types: float, int64_t, bool.");
    const char *elemType;
    if constexpr (std::is_same_v<T, float>) {
        elemType = "float";
    } else if constexpr (std::is_same_v<T, int64_t>) {
        elemType = "int64";
    } else if constexpr (std::is_same_v<T, bool>) {
        elemType = "bool";
    } else {
        elemType = "";
    }
    return elemType;
}

template <typename T>
static inline DS::JsonValue to_input_data_array(const char *name, const T *buffer, size_t size) {
    static_assert(std::is_same_v<T, float> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, bool>,
                  "T must be one of the following types: float, int64_t, bool.");
    auto elemType = type_to_string<T>();

    DS::JsonArray value;
    value.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        value.emplace_back(buffer[i]);
    }
    return DS::JsonObject {
        {"name", name},
        {"format", "array"},
        {"data", DS::JsonObject {
            {"type", elemType},
            {"shape", DS::JsonArray {int64_t(1), int64_t(size)}},
            {"value", value}
        }}
    };
}

template <typename T>
static inline DS::JsonValue to_input_data_bytes(const char *name, const T *buffer, size_t size) {
    static_assert(std::is_same_v<T, float> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, bool>,
                  "T must be one of the following types: float, int64_t, bool.");
    auto elemType = type_to_string<T>();

    const uint8_t *bytesBuffer = reinterpret_cast<const uint8_t *>(buffer);
    size_t bytesCount = size * sizeof(T);

    return DS::JsonObject {
        {"name", name},
        {"format", "bytes"},
        {"data", DS::JsonObject {
            {"type", elemType},
            {"shape", DS::JsonArray {int64_t(1), int64_t(size)}},
            {"value", DS::JsonValue(std::vector<uint8_t>(
                                   bytesBuffer, bytesBuffer + bytesCount))}
        }}
    };
}

static constexpr const char *dataformat_to_string(DataFormat format) {
    if (format == DataFormat::Array) {
        return "array";
    } else if (format == DataFormat::Bytes) {
        return "bytes";
    } else if (format == DataFormat::Reference) {
        return "reference";
    }
    return "";
}

struct NameInfo {
    const char *input1_name;
    const char *input2_name;
    const char *output_name;
};

struct FormatInfo {
    DataFormat input1_format;
    DataFormat input2_format;
    DataFormat output_format;
};

static inline DS::JsonValue generate_float_testdata(int64_t session_id,
                                                    int64_t context_id,
                                                    const FormatInfo &formatInfo,
                                                    const NameInfo &nameInfo) {
    // clang-format off
    std::array<float, 20> hardcoded_data_1 = {
         0.23796290f,  0.53711500f, -0.95318216f,  0.80716910f, -0.38328704f,
         0.59790190f, -0.44979632f,  0.52018136f, -0.33009866f,  0.61674390f,
        -0.29060215f, -0.15592323f,  0.59866330f, -0.87646330f, -0.26810077f,
        -0.84345610f,  0.44154274f, -0.94140560f, -0.83036830f,  0.12774510f
    };
    std::array<float, 20> hardcoded_data_2 = {
        -0.29216707f, -0.74938920f,  0.20425930f,  0.41660118f, -0.53738200f,
         0.19390522f,  0.75557244f,  0.35427395f, -0.48109946f, -0.34268898f,
        -0.95628880f,  0.05590263f,  0.94203300f, -0.52953863f, -0.58428663f,
         0.50943470f, -0.78055650f,  0.44433710f,  0.09467409f, -0.96645087f
    };
    // clang-format on
    DS::JsonArray inputJsonArr;
    inputJsonArr.reserve(2);
    if (formatInfo.input1_format == DataFormat::Array) {
        inputJsonArr.emplace_back(to_input_data_array(
            nameInfo.input1_name, hardcoded_data_1.data(), hardcoded_data_1.size()));
    } else if (formatInfo.input1_format == DataFormat::Bytes) {
        inputJsonArr.emplace_back(to_input_data_bytes(
            nameInfo.input1_name, hardcoded_data_1.data(), hardcoded_data_1.size()));
    }
    if (formatInfo.input2_format == DataFormat::Array) {
        inputJsonArr.emplace_back(to_input_data_array(
            nameInfo.input2_name, hardcoded_data_2.data(), hardcoded_data_2.size()));
    } else if (formatInfo.input2_format == DataFormat::Bytes) {
        inputJsonArr.emplace_back(to_input_data_bytes(
            nameInfo.input2_name, hardcoded_data_2.data(), hardcoded_data_2.size()));
    }
    auto outFormatStr = dataformat_to_string(formatInfo.output_format);
    return DS::JsonObject {
        {"session", session_id},
        {"context", context_id},
        {"input", inputJsonArr},
        {"output", DS::JsonArray {
            {DS::JsonObject {
                {"name", nameInfo.output_name},
                {"format", outFormatStr}
            }}
        }}
    };
}

static const int64_t g_dataCount = 12;

DS::JsonValue TestInferData::generate(int64_t data_id, int64_t session_id, int64_t context_id) {
    NameInfo nameInfo{"input1", "input2", "output"};

    FormatInfo formatConfigs[g_dataCount] = {
        {DataFormat::Array, DataFormat::Array, DataFormat::Array},     // case 1
        {DataFormat::Array, DataFormat::Array, DataFormat::Bytes},     // case 2
        {DataFormat::Array, DataFormat::Array, DataFormat::Reference},  // case 3
        {DataFormat::Bytes, DataFormat::Bytes, DataFormat::Array},      // case 4
        {DataFormat::Bytes, DataFormat::Bytes, DataFormat::Bytes},      // case 5
        {DataFormat::Bytes, DataFormat::Bytes, DataFormat::Reference},   // case 6
        {DataFormat::Array, DataFormat::Bytes, DataFormat::Array},      // case 7
        {DataFormat::Array, DataFormat::Bytes, DataFormat::Bytes},      // case 8
        {DataFormat::Array, DataFormat::Bytes, DataFormat::Reference},    // case 9
        {DataFormat::Bytes, DataFormat::Array, DataFormat::Array},      // case 10
        {DataFormat::Bytes, DataFormat::Array, DataFormat::Bytes},      // case 11
        {DataFormat::Bytes, DataFormat::Array, DataFormat::Reference}    // case 12
    };

    if (data_id < 1 || data_id > g_dataCount) {
        return {DS::JsonValue::Undefined};
    }

    const FormatInfo& formatInfo = formatConfigs[data_id - 1];

    return generate_float_testdata(session_id, context_id, formatInfo, nameInfo);
}

int64_t TestInferData::count() {
    return g_dataCount;
}
