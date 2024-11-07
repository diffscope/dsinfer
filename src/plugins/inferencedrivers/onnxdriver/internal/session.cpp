#include "session.h"
#include "sessionsystem.h"
#include "sessionimage.h"
#include "executionprovider.h"
#include "env.h"
#include "onnxdriver_common.h"
#include "onnxdriver_logger.h"

#include <dsinfer/dsinferglobal.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <sstream>
#include <fstream>
#include <unordered_set>

#include <hash-library/sha256.h>

namespace fs = std::filesystem;

namespace dsinfer::onnxdriver {

    template <typename ValueMapType>
    inline ValueMapType SessionRunHelper(SessionImage *image, Ort::RunOptions &runOptions,
                                         const ValueMapType &inputValueMap, Error *error);

    class Session::Impl {
    public:
        SessionImage *image = nullptr;
        Ort::RunOptions runOptions;
    };

    Session::Session() : _impl(std::make_unique<Impl>()) {
    }

    Session::~Session() {
        close();
    }

    Session::Session(Session &&other) noexcept {
        std::swap(_impl, other._impl);
    }

    Session &Session::operator=(Session &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        std::swap(_impl, other._impl);
        return *this;
    }

    static std::vector<uint8_t> compute_sha256(const fs::path &path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return {};
        }

        static constexpr const size_t buffer_size = 4096; // Process 4KB each time
        char buffer[buffer_size];

        SHA256 sha256_ctx;
        while (file.read(buffer, buffer_size) || file.gcount() > 0) {
            sha256_ctx.add(buffer, file.gcount());
        }
        std::vector<uint8_t> final;
        final.resize(32);
        sha256_ctx.getHash(final.data());
        return final;
    }

    template <typename CharType>
    static std::string bytesToHexString(const CharType *bytes, size_t bytesSize) {
        static_assert(std::is_same_v<CharType, char> || std::is_same_v<CharType, unsigned char>,
                      "CharType must be char or unsigned char.");
        static constexpr const char *hexDigits = "0123456789abcdef";
        std::string hexString(bytesSize * 2, '\0');
        for (size_t i = 0; i < bytesSize; ++i) {
            auto byte = static_cast<unsigned char>(bytes[i]);
            hexString[i * 2] = hexDigits[(byte >> 4) & 0x0f];
            hexString[i * 2 + 1] = hexDigits[byte & 0x0f];
        }
        return hexString;
    }

    bool Session::open(const fs::path &path, bool useCpuHint, Error *error) {
        __stdc_impl_t;
        // TODO: If the same session is already opened before, useCpuHint will have no effect
        //       due to SessionSystem will return the existing SessionImage instead of creating a
        //       new one. Should this be the desired behavior, or it needs to be fixed?

        if (!Env::instance() || !Env::instance()->isLoaded()) {
            onnxdriver_log().critical("Session - The environment is not initialized!");
            return false;
        }
        if (!SessionSystem::instance()) {
            onnxdriver_log().critical("Session - The session system is not initialized!");
            return false;
        }
        if (isOpen()) {
            onnxdriver_log().warning("Session - Session %1 is already open!", path.string());
            return false;
        }
        onnxdriver_log().debug("Session - Try open " + path.string());
        fs::path canonicalPath;
        try {
            canonicalPath = fs::canonical(path);
            onnxdriver_log().debug("Session - The canonical path is " + canonicalPath.string());
        } catch (const fs::filesystem_error &e) {
            if (error) {
                *error = Error(Error::FileNotFound, e.what());
            }
            return false;
        }

        if (!fs::is_regular_file(canonicalPath)) {
            if (error) {
                *error = Error(Error::FileNotFound, "Not a regular file");
            }
            return false;
        }

        auto sha256 = compute_sha256(path);
        auto sha256_str = bytesToHexString(sha256.data(), sha256.size());
        onnxdriver_log().debug("Session - SHA256 is %1", sha256_str);

        // TODO: Check sha256 to determine if a same file has been loaded

        std::string errorMessage;
        if (auto image = SessionSystem::instance()->getImage(canonicalPath); image == nullptr) {
            onnxdriver_log().debug(
                "Session - The session image does not exist. Creating a new one...");
            impl.image = SessionImage::create(path, useCpuHint, &errorMessage);
        } else {
            onnxdriver_log().debug(
                "Session - The session image already exists. Increasing the reference count...");
            impl.image = image;
            impl.image->ref();
        }

        if (!impl.image) {
            if (error) {
                *error = Error(Error::SessionError, errorMessage);
            }
            return false;
        }
        return true;
    }

    bool Session::close() {
        __stdc_impl_t;
        if (!impl.image)
            return false;

        onnxdriver_log().debug("Session [%1] - close", path().filename());

        if (impl.image->deref() == 0) {
            impl.image = nullptr;
        }
        return true;
    }

    fs::path Session::path() const {
        __stdc_impl_t;
        return impl.image ? impl.image->path : fs::path();
    }

    bool Session::isOpen() const {
        __stdc_impl_t;
        return impl.image != nullptr;
    }

    template <typename ValueMapType>
    inline ValueMapType SessionRunHelper(SessionImage *image, Ort::RunOptions &runOptions,
                                         const ValueMapType &inputValueMap, Error *error) {
        static_assert(std::is_same_v<ValueMapType, ValueMap> ||
                      std::is_same_v<ValueMapType, SharedValueMap>);

        auto filename = image ? image->path.filename() : "";
        onnxdriver_log().info("Session [%1] - Running inference", filename);
        auto timeStart = std::chrono::steady_clock::now();

        if (!image) {
            if (error) {
                *error = Error(Error::SessionError, "Session is not open");
            }
            return {};
        }

        if (inputValueMap.empty()) {
            if (error) {
                *error = Error(Error::SessionError, "Input map is empty");
            }
            return {};
        }

        const auto &requiredInputNames = image->inputNames;
        std::ostringstream msgStream;
        msgStream << '[' << filename << ']' << ' ';

        // Check for missing and extra input names. If found, return empty map and the error
        // message.
        {
            bool flagMissing = false;
            // Check for missing input names

            for (const auto &requiredInputName : requiredInputNames) {
                if (inputValueMap.find(requiredInputName) == inputValueMap.end()) {
                    if (flagMissing) {
                        // It isn't the first missing input name. Append a comma separator.
                        msgStream << ',' << ' ';
                    } else {
                        // It's the first missing input name. Append the message intro.
                        msgStream << "Missing input name(s): ";
                        flagMissing = true;
                    }
                    msgStream << '"' << requiredInputName << '"';
                }
            }

            // Check for extra input names
            bool flagExtra = false;
            std::unordered_set<std::string> requiredSet(requiredInputNames.begin(),
                                                        requiredInputNames.end());
            for (auto &it : std::as_const(inputValueMap)) {
                auto &actualInputName = it.first;
                if (requiredSet.find(actualInputName) == requiredSet.end()) {
                    if (flagExtra) {
                        msgStream << ',' << ' ';
                    } else {
                        if (flagMissing) {
                            msgStream << ';' << ' ';
                        }
                        msgStream << "Extra input names(s): ";
                        flagExtra = true;
                    }
                    msgStream << '"' << actualInputName << '"';
                }
            }

            if (flagMissing || flagExtra) {
                if (error) {
                    *error = Error(Error::SessionError, msgStream.str());
                }
                return {};
            }
        }

        try {
            auto memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

            Ort::IoBinding binding(image->session);

            if constexpr (std::is_same_v<ValueMapType, SharedValueMap>) {
                for (auto &[name, value] : inputValueMap) {
                    binding.BindInput(name.c_str(), *value);
                }
            } else {
                for (auto &[name, value] : inputValueMap) {
                    binding.BindInput(name.c_str(), value);
                }
            }

            const auto &outputNames = image->outputNames;
            for (const auto &name : outputNames) {
                binding.BindOutput(name.c_str(), memInfo);
            }

            runOptions.UnsetTerminate();
            image->session.Run(runOptions, binding);

            ValueMapType outValueMap;
            auto outputValues = binding.GetOutputValues();
            if constexpr (std::is_same_v<ValueMapType, SharedValueMap>) {
                for (size_t i = 0; i < outputValues.size(); ++i) {
                    outValueMap.emplace(outputNames[i],
                                        makeSharedValue(std::move(outputValues[i])));
                }
            } else {
                for (size_t i = 0; i < outputValues.size(); ++i) {
                    outValueMap.emplace(outputNames[i], std::move(outputValues[i]));
                }
            }
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now() - timeStart)
                               .count();
            auto elapsedSeconds = elapsed / 1000;
            auto elapsedMs = static_cast<int>(elapsed % 1000);
            char elapsedMsStr[4];
            snprintf(elapsedMsStr, sizeof(elapsedMsStr), "%03d", elapsedMs);
            onnxdriver_log().info("Session [%1] - Finished inference in %2.%3 seconds", filename,
                                  elapsedSeconds, elapsedMsStr);
            return outValueMap;
        } catch (const Ort::Exception &err) {
            if (error) {
                *error = Error(Error::SessionError, err.what());
            }
        }
        return {};
    }

    std::vector<std::string> Session::inputNames() const {
        __stdc_impl_t;
        if (!impl.image) {
            return {};
        }
        return impl.image->inputNames;
    }

    std::vector<std::string> Session::outputNames() const {
        __stdc_impl_t;
        if (!impl.image) {
            return {};
        }
        return impl.image->outputNames;
    }

    void Session::terminate() {
        __stdc_impl_t;
        impl.runOptions.SetTerminate();
    }

    ValueMap Session::run(const ValueMap &inputValueMap, Error *error) {
        __stdc_impl_t;
        return SessionRunHelper<ValueMap>(impl.image, impl.runOptions, inputValueMap, error);
    }

    SharedValueMap Session::run(const SharedValueMap &inputValueMap, Error *error) {
        __stdc_impl_t;
        return SessionRunHelper<SharedValueMap>(impl.image, impl.runOptions, inputValueMap, error);
    }

}