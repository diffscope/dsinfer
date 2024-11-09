#ifndef DSINFER_ONNXDRIVER_TENSORPARSER_H
#define DSINFER_ONNXDRIVER_TENSORPARSER_H

#include <cstring>
#include <numeric>

#include <onnxruntime_cxx_api.h>

#include <dsinfer/error.h>
#include <dsinfer/jsonvalue.h>

namespace dsinfer {
    inline bool checkStringValue(const JsonObject &obj, const std::string &key, const std::string &value) {
        if (auto it = obj.find(key); it != obj.end()) {
            if (!it->second.isString()) {
                return false;
            }
            return it->second.toString() == value;
        }
        return false;
    }

    inline bool checkStringValues(const JsonObject &obj, const std::string &key, const std::initializer_list<std::string> &values) {
        if (auto it = obj.find(key); it != obj.end()) {
            if (!it->second.isString()) {
                return false;
            }
            const auto valString = it->second.toString();
            for (const auto &value : values) {
                if (valString == value) {
                    return true;
                }
            }
        }
        return false;
    }
}

namespace dsinfer::onnxdriver {

    template <typename T>
    inline Ort::Value createTensorFromBytes(OrtAllocator *allocator,
                                            const uint8_t *data,
                                            size_t dataSize,
                                            const int64_t *shape,
                                            size_t shapeSize,
                                            Error *error = nullptr) {
        auto expectedDataLength = std::reduce(shape, shape + shapeSize,
                                              int64_t{1}, std::multiplies<>());
        auto expectedBytes = expectedDataLength * sizeof(T);
        if (dataSize != expectedBytes) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid input format: data size must match shape");
            }
            return Ort::Value(nullptr);
        }
        auto value = Ort::Value::CreateTensor<T>(allocator, shape, shapeSize);
        auto buffer = value.template GetTensorMutableData<uint8_t>();
        if (!buffer) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Failed to convert: ort tensor buffer is null");
            }
            return Ort::Value(nullptr);
        }
        std::memcpy(buffer, data, expectedBytes);
        return value;
    }

    template <typename T>
    inline Ort::Value createTensorFromJsonArray(const JsonArray &jsonArray,
                                                const int64_t *shape,
                                                size_t shapeSize,
                                                Error *error = nullptr) {
        static_assert(std::is_same_v<T, float> || std::is_same_v<T, int64_t> || std::is_same_v<T, bool>);

        auto expectedDataLength = std::reduce(shape, shape + shapeSize,
                                              int64_t{1}, std::multiplies<>());
        if (jsonArray.size() != expectedDataLength) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid input format: data size must match shape");
            }
            return Ort::Value(nullptr);
        }
        Ort::AllocatorWithDefaultOptions allocator;
        auto value = Ort::Value::CreateTensor<T>(allocator, shape, shapeSize);
        auto buffer = value.template GetTensorMutableData<T>();
        if (!buffer) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Failed to convert: ort tensor buffer is null");
            }
            return Ort::Value(nullptr);
        }

        for (size_t i = 0; i < jsonArray.size(); ++i) {
            // TODO: validate data type for jsonArray elements
            if constexpr (std::is_same_v<T, float>) {
                buffer[i] = static_cast<float>(jsonArray[i].toDouble());
            } else if constexpr (std::is_same_v<T, int64_t>) {
                buffer[i] = jsonArray[i].toInt64();
            } else if constexpr (std::is_same_v<T, bool>) {
                buffer[i] = jsonArray[i].toBool();
            }
        }
        return value;
    }

    inline Ort::Value deserializeTensorFromBytes(const uint8_t *dataBuffer, size_t dataSize,
                                                 const int64_t *shapeBuffer, size_t shapeSize,
                                                 const std::string &dataType, Error *error) {
        Ort::AllocatorWithDefaultOptions allocator;
        if (dataType == "float" || dataType == "float32") {
            return createTensorFromBytes<float>(allocator, dataBuffer, dataSize,
                                                shapeBuffer, shapeSize, error);
        } else if (dataType == "int64") {
            return createTensorFromBytes<int64_t>(allocator, dataBuffer, dataSize,
                                                  shapeBuffer, shapeSize, error);
        } else if (dataType == "bool") {
            return createTensorFromBytes<bool>(allocator, dataBuffer, dataSize,
                                               shapeBuffer, shapeSize, error);
        }

        // unknown type
        if (error) {
            *error = Error(Error::InvalidFormat,
                           "Invalid input format: unknown data type");
        }
        return Ort::Value(nullptr);
    }

    inline Ort::Value deserializeTensor(const JsonValue &input, Error *error = nullptr) {
        const auto &jVal_data = input["value"];  // bytes
        const auto &jVal_type = input["type"];  // string
        const auto &jVal_shape = input["shape"];  // array

        if (!jVal_data.isBinary() && !jVal_data.isString() && !jVal_data.isArray()) {
            if (error) {
                *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                        "Invalid input format: value must be binary, string or array");
            }
            return Ort::Value(nullptr);
        }

        if (!jVal_type.isString()) {
            if (error) {
                *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                        "Invalid input format: type must be string");
            }
            return Ort::Value(nullptr);
        }
        if (!jVal_shape.isArray()) {
            if (error) {
                *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                        "Invalid input format: shape must be array");
            }
            return Ort::Value(nullptr);
        }

        // process type
        auto type = jVal_type.toString();

        // process shape
        auto jArr_shape = jVal_shape.toArray();
        std::vector<int64_t> shape;
        shape.reserve(jArr_shape.size());
        for (const auto &item: jArr_shape) {
            if (!item.isInt() && !item.isDouble()) {
                if (error) {
                    *error = Error(Error::InvalidFormat,
                                   "Invalid input format: shape array elements must be numbers");
                }
                return Ort::Value(nullptr);
            }
            auto shapeDimValue = item.toInt64();
            shape.push_back(shapeDimValue);
        }

        // process value
        if (jVal_data.isBinary()) {
            auto data = jVal_data.toBinary();
            return deserializeTensorFromBytes(data.data(), data.size(), shape.data(), shape.size(), type, error);
        } else if (jVal_data.isString()) {
            auto data = jVal_data.toString();
            return deserializeTensorFromBytes(reinterpret_cast<uint8_t *>(data.data()), data.size(), shape.data(), shape.size(), type, error);
        } else if (jVal_data.isArray()) {
            auto dataArray = jVal_data.toArray();
            if (type == "float" || type == "float32") {
                return createTensorFromJsonArray<float>(dataArray, shape.data(), shape.size(), error);
            } else if (type == "int64") {
                return createTensorFromJsonArray<int64_t>(dataArray, shape.data(), shape.size(), error);
            } else if (type == "bool") {
                return createTensorFromJsonArray<bool>(dataArray, shape.data(), shape.size(), error);
            } else {
                // TODO: support more data types
                if (error) {
                    *error = Error(Error::InvalidFormat, "data type \"" + type + "\" is not supported!");
                }
                return Ort::Value(nullptr);
            }
        }
        return Ort::Value(nullptr);
    }

    inline JsonValue serializeTensorAsBytes(const Ort::Value &tensor, Error *error = nullptr) {
        std::string dataType;
        size_t elemSize = 1;
        auto typeAndShapeInfo = tensor.GetTensorTypeAndShapeInfo();
        auto type = typeAndShapeInfo.GetElementType();

        // Serialize shape
        JsonArray shapeArray;
        auto tensorShape = typeAndShapeInfo.GetShape();
        shapeArray.resize(tensorShape.size());
        for (size_t i = 0; i < tensorShape.size(); ++i) {
            shapeArray[i] = JsonValue(tensorShape[i]);
        }

        // Serialize type
        switch (type) {
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT: {
                dataType = "float";
                elemSize = sizeof(float);
                break;
            }
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64: {
                dataType = "int64";
                elemSize = sizeof(int64_t);
                break;
            }
            case ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL: {
                dataType = "bool";
                elemSize = sizeof(bool);
                break;
            }
            default:
                if (error) {
                    *error = Error(Error::InvalidFormat,
                                   "Failed to convert to JsonValue: unknown tensor type");
                }
                return false; // Unknown tensor type
        }

        // Serialize data (as binary)
        auto bufferSize = typeAndShapeInfo.GetElementCount() * elemSize;

        auto buffer = tensor.template GetTensorData<uint8_t>();
        if (!buffer) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Failed to convert to JsonValue: ort tensor buffer is null");
            }
            return {};
        }

        return JsonObject {
            {"value", JsonValue(std::vector<uint8_t>(buffer, buffer + bufferSize))},
            {"shape", shapeArray},
            {"type", dataType},
        };
    }

    inline JsonValue serializeTensorAsArray(const Ort::Value &tensor, Error *error = nullptr) {
        std::string dataType;
        auto typeAndShapeInfo = tensor.GetTensorTypeAndShapeInfo();
        auto type = typeAndShapeInfo.GetElementType();

        // Serialize shape
        JsonArray shapeArray;
        auto tensorShape = typeAndShapeInfo.GetShape();
        shapeArray.resize(tensorShape.size());
        for (size_t i = 0; i < tensorShape.size(); ++i) {
            shapeArray[i] = JsonValue(tensorShape[i]);
        }

        // Serialize type and data
        JsonArray dataArray;
        auto elemCount = typeAndShapeInfo.GetElementCount();
        dataArray.reserve(elemCount);

        switch (type) {
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT: {
            dataType = "float";
            auto buffer = tensor.template GetTensorData<float>();
            if (!buffer) {
                if (error) {
                    *error = Error(Error::InvalidFormat,
                                   "Failed to convert to JsonValue: ort tensor buffer is null");
                }
                return {};
            }
            for (size_t i = 0; i < elemCount; ++i) {
                dataArray.emplace_back(buffer[i]);
            }
            break;
        }
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64: {
            dataType = "int64";
            auto buffer = tensor.template GetTensorData<int64_t>();
            if (!buffer) {
                if (error) {
                    *error = Error(Error::InvalidFormat,
                                   "Failed to convert to JsonValue: ort tensor buffer is null");
                }
                return {};
            }
            for (size_t i = 0; i < elemCount; ++i) {
                dataArray.emplace_back(buffer[i]);
            }
            break;
        }
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL: {
            dataType = "bool";
            auto buffer = tensor.template GetTensorData<bool>();
            if (!buffer) {
                if (error) {
                    *error = Error(Error::InvalidFormat,
                                   "Failed to convert to JsonValue: ort tensor buffer is null");
                }
                return {};
            }
            for (size_t i = 0; i < elemCount; ++i) {
                dataArray.emplace_back(buffer[i]);
            }
            break;
        }
        default:
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Failed to convert to JsonValue: unknown tensor type");
            }
            return false; // Unknown tensor type
        }

        return JsonObject {
            {"value", dataArray},
            {"shape", shapeArray},
            {"type", dataType},
        };
    }

    inline Ort::Value parseInputContent(const JsonObject &content, Error *error = nullptr) {
        if (auto it_content = content.find("data"); it_content != content.end()) {
            if (checkStringValues(content, "format", {"bytes", "array"})) {
                return onnxdriver::deserializeTensor(it_content->second.toObject(), error);
            }
        } else {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Failed to parse content");
            }
        }
        return Ort::Value(nullptr);
    }
}

#endif // DSINFER_ONNXDRIVER_TENSORPARSER_H
