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
    inline Ort::Value createTensorFromRawBytes(OrtAllocator *allocator,
                                               const std::vector<uint8_t> &data,
                                               const std::vector<int64_t> &shape,
                                               Error *error = nullptr) {
        auto expectedDataLength = std::reduce(shape.begin(), shape.end(),
                                              int64_t{1}, std::multiplies<>());
        auto expectedBytes = expectedDataLength * sizeof(T);
        if (data.size() != expectedBytes) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid input format: data size must match shape");
            }
            return Ort::Value(nullptr);
        }
        auto value = Ort::Value::CreateTensor<T>(allocator, shape.data(), shape.size());
        auto buffer = value.template GetTensorMutableData<uint8_t>();
        if (!buffer) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Failed to convert: ort tensor buffer is null");
            }
            return Ort::Value(nullptr);
        }
        std::memcpy(buffer, data.data(), expectedBytes);
        return value;
    }

    inline Ort::Value deserializeTensor(const JsonValue &input, Error *error = nullptr) {
        const auto &jVal_data = input["value"];  // bytes
        const auto &jVal_type = input["type"];  // string
        const auto &jVal_shape = input["shape"];  // array

        if (!jVal_data.isBinary() || !jVal_type.isString() || !jVal_shape.isArray()) {
            if (error) {
                *error = Error(Error::InvalidFormat,
                               "Invalid input format");
            }
            return Ort::Value(nullptr);
        }

        auto data = jVal_data.toBinary();
        auto type = jVal_type.toString();
        auto jArr_shape = jVal_shape.toArray();
        std::vector<int64_t> shape;
        shape.reserve(jArr_shape.size());

        int64_t expectedDataLength = 1;
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
            expectedDataLength *= shapeDimValue;
        }

        Ort::AllocatorWithDefaultOptions allocator;
        if (type == "float") {
            return createTensorFromRawBytes<float>(allocator, data, shape, error);
        } else if (type == "int64") {
            return createTensorFromRawBytes<int64_t>(allocator, data, shape, error);
        } else if (type == "bool") {
            return createTensorFromRawBytes<bool>(allocator, data, shape, error);
        }

        // unknown type
        if (error) {
            *error = Error(Error::InvalidFormat,
                           "Invalid input format: unknown data type");
        }
        return Ort::Value(nullptr);
    }

    inline JsonValue serializeTensor(const Ort::Value &tensor, Error *error = nullptr) {
        std::string dataType;
        size_t elemSize = 1;
        auto typeAndShapeInfo = tensor.GetTensorTypeAndShapeInfo();
        auto type = typeAndShapeInfo.GetElementType();

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

        // Serialize shape
        JsonArray shapeArray;
        auto tensorShape = typeAndShapeInfo.GetShape();
        shapeArray.resize(tensorShape.size());
        for (size_t i = 0; i < tensorShape.size(); ++i) {
            shapeArray[i] = JsonValue(tensorShape[i]);
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

    inline Ort::Value parseInputContent(const JsonObject &content, Error *error = nullptr) {
        if (auto it_content = content.find("data"); it_content != content.end()) {
            if (checkStringValue(content, "format", "bytes")) {
                return onnxdriver::deserializeTensor(it_content->second.toObject(), error);
            } else if (checkStringValue(content, "format", "array")) {
                // TODO: to be implemented
                if (error) {
                    *error = Error(Error::InvalidFormat,
                                   "Array format is not implemented yet");
                }
                return Ort::Value(nullptr);
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
