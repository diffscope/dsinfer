# DiffSinger Inference

Low level library for DiffSinger onnx model inference.

## Requirements

+ [nlohmann_json](https://github.com/nlohmann/json)
+ [stduuid](https://github.com/mariusbancila/stduuid)
+ [sha256](https://github.com/B-Con/crypto-algorithms)
+ [qmsetup](https://github.com/stdware/qmsetup)
+ [syscmdline](https://github.com/SineStriker/syscmdline)
+ [stdcorelib](https://github.com/SineStriker/stdcorelib)
+ [zlib](https://github.com/madler/zlib)

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

### Build & Install

<!-- If you have installed the required libraries specified in `scripts/vcpkg-manifest/vcpkg.json`, you can skip setting VCPKG variables so long as you make sure CMake can find them. -->

The buildsystem is able to deploy the shared libraries to build directory and install directory automatically.

```sh
cmake -B build -G Ninja \
    -DCMAKE_INSTALL_PREFIX=<dir> \  # install directory
    -DCMAKE_PREFIX_PATH=<dir> \     # directory `Qt5Config.cmake` locates
    -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DQMSETUP_APPLOCAL_DEPS_PATHS_DEBUG=vcpkg/installed/<triplet>/debug/<runtime> \
    -DQMSETUP_APPLOCAL_DEPS_PATHS_RELEASE=vcpkg/installed/<triplet>/<runtime> \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build --target all

cmake --build build --target install

# triplet:
#   Windows:  `x64-windows` 
#   Mac:      `x64-osx` or `arm64-osx`
#   Linux:    `x64-linux` or `arm64-linux`

# runtime:
#   Windows:    `bin`
#   Mac/Linux:  `lib`
```