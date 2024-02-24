# DiffSinger Inference

Low level library for DiffSinger onnx model inference.

## Requirements

+ [rapidjson](https://github.com/tencent/rapidjson)
+ [onnxruntime](https://github.com/onnxruntime/onnxruntime)

## Functionalities

+ Vocoder Inference
+ Acoustic Inference
+ Duration Inference (Encoder/Decoder)
+ Pitch Inference
+ Variance Inference

## Setup Environment

### Windows
```sh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

vcpkg install --x-manifest-root=../scripts/vcpkg-manifest --x-install-root=./installed --triplet=x64-windows
```

### Unix
```sh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

./vcpkg install \
    --x-manifest-root=../scripts/vcpkg-manifest \
    --x-install-root=./installed \
    --triplet=<triplet>

# triplet:
#   Mac:   `x64-osx` or `arm64-osx`
#   Linux: `x64-linux` or `arm64-linux`
```