#ifndef ONNXDRIVER_TENSORPARSER_H
#define ONNXDRIVER_TENSORPARSER_H

#include <numeric>

#include <onnxruntime_cxx_api.h>

#include "dsinfer/error.h"
#include "dsinfer/jsonvalue.h"

namespace dsinfer {
    namespace onnxdriver {

        template <typename T>
        inline Ort::Value createTensorFromRawBytes(OrtAllocator *allocator,
                                            const std::vector<uint8_t> &data,
                                            const std::vector<int64_t> &shape,
                                            dsinfer::Error *error = nullptr) {
            auto expectedDataLength = std::reduce(shape.begin(), shape.end(),
                                                  int64_t{1}, std::multiplies<>());
            auto expectedBytes = expectedDataLength * sizeof(T);
            if (data.size() != expectedBytes) {
                if (error) {
                    *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                            "Invalid input format: data size must match shape");
                }
                return Ort::Value(nullptr);
            }
            auto value = Ort::Value::CreateTensor<T>(allocator, shape.data(), shape.size());
            auto buffer = value.template GetTensorMutableData<uint8_t>();
            if (!buffer) {
                if (error) {
                    *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                            "Failed to convert: ort tensor buffer is null");
                }
                return Ort::Value(nullptr);
            }
            std::memcpy(buffer, data.data(), expectedBytes);
            return value;
        }

        inline Ort::Value deserializeTensor(const dsinfer::JsonValue &input, dsinfer::Error *error = nullptr) {
            const auto &jVal_data = input["data"];  // binary
            const auto &jVal_type = input["type"];  // string
            const auto &jVal_shape = input["shape"];  // array

            if (!jVal_data.isBinary() || !jVal_type.isString() || !jVal_shape.isArray()) {
                if (error) {
                    *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
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
                if (!item.isInteger() && !item.isDouble()) {
                    if (error) {
                        *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                                "Invalid input format: shape array elements must be numbers");
                    }
                    return Ort::Value(nullptr);
                }
                auto shapeDimValue = item.toInt();  // TODO: toInt64
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
                *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                        "Invalid input format: unknown data type");
            }
            return Ort::Value(nullptr);
        }

        inline dsinfer::JsonValue serializeTensor(const Ort::Value &tensor, dsinfer::Error *error = nullptr) {
            dsinfer::JsonValue jVal;
            auto typeAndShapeInfo = tensor.GetTensorTypeAndShapeInfo();
            auto type = typeAndShapeInfo.GetElementType();

            // Serialize type
            switch (type) {
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT: {
                    jVal["type"] = "float";
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64: {
                    jVal["type"] = "int64";
                    break;
                }
                case ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL: {
                    jVal["type"] = "bool";
                    break;
                }
                default:
                    if (error) {
                        *error = dsinfer::Error(dsinfer::Error::InvalidFormat,
                                                "Failed to convert to JsonValue: unknown tensor type");
                    }
                    return false; // Unknown tensor type
            }

            // Serialize shape
            dsinfer::JsonArray shapeArray;
            auto tensorShape = typeAndShapeInfo.GetShape();
            shapeArray.resize(tensorShape.size());
            for (size_t i = 0; i < tensorShape.size(); ++i) {
                shapeArray[i] = dsinfer::JsonValue(static_cast<int>(tensorShape[i])); // TODO: int64_t type in JsonValue
            }
            jVal["shape"] = shapeArray;

            // Serialize data (as binary)
            auto bufferSize = typeAndShapeInfo.GetElementCount();

            auto buffer = tensor.template GetTensorData<uint8_t>();
            if (!buffer) {
                if (error) {
                    *error = dsinfer::Error(dsinfer::Error::Type::InvalidFormat,
                                            "Failed to convert to JsonValue: ort tensor buffer is null");
                }
            }

            jVal["data"] = dsinfer::JsonValue(buffer, bufferSize);

            return jVal;
        }
    }
}

#endif // ONNXDRIVER_TENSORPARSER_H