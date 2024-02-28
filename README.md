# DiffSinger Inference

Low level library for DiffSinger onnx model inference.

## Requirements

+ [rapidjson](https://github.com/tencent/rapidjson)
+ [yaml-cpp](https://github.com/jbeder/yaml-cpp)
+ [onnxruntime](https://github.com/onnxruntime/onnxruntime)
+ [qmsetup](https://github.com/stdware/qmsetup)

## Functionalities

+ Vocoder Inference
+ Acoustic Inference
+ Duration Inference (Encoder/Decoder)
+ Pitch Inference
+ Variance Inference

## Setup Environment

### VCPKG Packages

#### Windows
```sh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat

vcpkg install --x-manifest-root=../scripts/vcpkg-manifest --x-install-root=./installed --triplet=x64-windows
```

#### Unix
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

### OnnxRuntime

```sh
cd libs
cmake [-Dep=gpu] -P ../scripts/setup-onnxruntime.cmake
```