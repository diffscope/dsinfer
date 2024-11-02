# DiffSinger Inference

Low level library for DiffSinger onnx model inference.

## Requirements

+ [nlohmann_json](https://github.com/nlohmann/json)
+ [qmsetup](https://github.com/stdware/qmsetup)
+ [stduuid](https://github.com/mariusbancila/stduuid)
+ [sha256](https://github.com/B-Con/crypto-algorithms)

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