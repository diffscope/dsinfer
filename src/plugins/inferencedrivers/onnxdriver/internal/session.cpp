#include "session.h"

#include <cassert>
#include <cstdio>
#include <shared_mutex>
#include <sstream>
#include <fstream>
#include <unordered_set>
#include <algorithm>
#include <list>

#include <dsinfer/dsinferglobal.h>

#include <hash-library/sha256.h>

#include "onnxdriver_logger.h"
#include "env.h"
#include "sessionimage.h"
#include "scopedtimer.h"

namespace fs = std::filesystem;

namespace dsinfer::onnxdriver {

    struct SessionSystem {
        struct ImageData {
            SessionImage *image;
            int count;
        };

        struct ImageGroup {
            std::filesystem::path path;
            std::streamsize size = 0;
            std::vector<uint8_t> sha256;
            std::map<int, ImageData> images; // hint -> [ image, count ]
        };

        struct Sha256SizeKey {
            std::streamsize size;
            std::vector<uint8_t> sha256;

            bool operator<(const Sha256SizeKey &other) const {
                if (size == other.size) {
                    return std::lexicographical_compare(sha256.begin(), sha256.end(),
                                                        other.sha256.begin(), other.sha256.end());
                }
                return size < other.size;
            }
        };

        std::list<ImageGroup> image_list;

        using ListIterator = decltype(image_list)::iterator;

        std::map<std::filesystem::path::string_type, ListIterator> path_map;
        std::map<Sha256SizeKey, ListIterator> sha256_size_map;

        std::shared_mutex mtx;

        static SessionSystem &global() {
            static SessionSystem instance;
            return instance;
        }
    };

    class Session::Impl {
    public:
        Ort::RunOptions runOptions;

        SessionSystem::ImageGroup *group = nullptr;
        SessionImage *image = nullptr;
        int hints = 0;

        std::filesystem::path realPath;

        template <typename ValueMapType>
        inline Error validateInputValueMap(const ValueMapType &inputValueMap) {
            static_assert(std::is_same_v<ValueMapType, ValueMap> ||
                          std::is_same_v<ValueMapType, SharedValueMap>);
            if (inputValueMap.empty()) {
                return {Error::SessionError, "Input map is empty"};
            }

            const auto &requiredInputNames = image->inputNames;
            std::ostringstream msgStream;
            msgStream << '[' << realPath.filename() << ']' << ' ';

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
                    return {Error::SessionError, msgStream.str()};
                }
            }
            return {}; // no error
        }

        template <typename ValueMapType>
        inline ValueMapType sessionRun(const ValueMapType &inputValueMap, Error *error) {
            static_assert(std::is_same_v<ValueMapType, ValueMap> ||
                          std::is_same_v<ValueMapType, SharedValueMap>);

            const auto &filename = realPath.filename();
            onnxdriver_log().info("Session [%1] - Running inference", filename);

            ScopedTimer timer([&](const ScopedTimer::duration_t &elapsed) {
                // When finished, print time elapsed
                auto elapsedStr = (std::ostringstream() << std::fixed << std::setprecision(3) << elapsed.count()).str();
                onnxdriver_log().info("Session [%1] - Finished inference in %2 seconds",
                                      filename, elapsedStr);
            });

            if (auto validateError = validateInputValueMap(inputValueMap); !validateError.ok()) {
                if (error) {
                    *error = std::move(validateError);
                }
                timer.deactivate();
                return {};
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
                return outValueMap;
            } catch (const Ort::Exception &err) {
                if (error) {
                    *error = Error(Error::SessionError, err.what());
                }
            }
            timer.deactivate();
            return {};
        }
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

    static bool getFileInfo(const fs::path &path, std::vector<uint8_t> &binaryResult,
                            std::string &stringResult, std::streamsize &sizeResult) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }

        // get size
        file.seekg(0, std::ios::end);
        sizeResult = file.tellg();
        file.seekg(0, std::ios::beg);

        static constexpr const size_t buffer_size = 4096; // Process 4KB each time
        char buffer[buffer_size];

        SHA256 sha256_ctx;
        while (file.read(buffer, buffer_size) || file.gcount() > 0) {
            sha256_ctx.add(buffer, file.gcount());
        }

        // get str
        stringResult = sha256_ctx.getHash();

        // get binary
        binaryResult.resize(32);
        sha256_ctx.getHash(binaryResult.data());
        return true;
    }

    bool Session::open(const fs::path &path, int hints, Error *error) {
        __stdc_impl_t;

        if (isOpen()) {
            onnxdriver_log().warning("Session - Session %1 is already open!", path.string());
            return false;
        }

        if (!Env::instance() || !Env::instance()->isLoaded()) {
            onnxdriver_log().critical("Session - The environment is not initialized!");
            return false;
        }

        // Open
        onnxdriver_log().debug("Session - Try open " + path.string());
        fs::path canonical_path;
        try {
            canonical_path = fs::canonical(path);
            onnxdriver_log().debug("Session - The canonical path is " + canonical_path.string());
        } catch (const fs::filesystem_error &e) {
            if (error) {
                *error = Error(Error::FileNotFound, e.what());
            }
            return false;
        }
        if (!fs::is_regular_file(canonical_path)) {
            if (error) {
                *error = Error(Error::FileNotFound, "not a regular file");
            }
            return false;
        }

        // Ready to load
        auto &session_system = SessionSystem::global();
        std::unique_lock<std::shared_mutex> lock(session_system.mtx);
        SessionImage *image = nullptr;
        std::vector<uint8_t> sha256;
        std::streamsize size;

        // Search path
        SessionSystem::ImageGroup *image_group = nullptr;
        if (auto it = session_system.path_map.find(canonical_path);
            it != session_system.path_map.end()) {
            image_group = &(*it->second);
            auto &image_map = image_group->images;
            if (auto it2 = image_map.find(hints); it2 != image_map.end()) {
                auto &data = it2->second;
                image = data.image;
                data.count++;
                goto out_exists;
            }
            sha256 = it->second->sha256;
            size = it->second->size;

            onnxdriver_log().debug("Session - No same hint in opened sessions");
            goto out_search_sha256;
        }

        // Calculate SHA256
        {
            std::string sha256_str;
            if (!getFileInfo(canonical_path, sha256, sha256_str, size)) {
                if (error) {
                    *error = Error(Error::FileNotFound, "failed to read file");
                }
                return false;
            }
            onnxdriver_log().debug("Session - SHA256 is %1", sha256_str);
        }

        // Search SHA256
        if (auto it = session_system.sha256_size_map.find({size, sha256});
            it != session_system.sha256_size_map.end()) {
            image_group = &(*it->second);
            auto &image_map = image_group->images;
            if (auto it2 = image_map.find(hints); it2 != image_map.end()) {
                auto &data = it2->second;
                image = data.image;
                data.count++;
                goto out_exists;
            }
        }

    out_search_sha256:

        onnxdriver_log().debug("Session - The session image does not exist. Creating a new one...");

        // Create new one
        image = new SessionImage();
        if (std::string error1; !image->open(canonical_path, hints, &error1)) {
            delete image;
            if (error) {
                *error = {
                    Error::FileNotFound,
                    "failed to read file: " + error1,
                };
            }
            return false;
        }

        // Insert
        if (!image_group) {
            onnxdriver_log().debug(
                "Session - The session image group doesn't exist. Creating a new group.");

            SessionSystem::ImageGroup group;
            group.path = canonical_path;
            group.size = size;
            group.sha256 = std::move(sha256);

            auto it = session_system.image_list.emplace(session_system.image_list.end(),
                                                        std::move(group));
            session_system.path_map[it->path] = it;
            session_system.sha256_size_map[{size, it->sha256}] = it;
            image_group = &(*it);
        }
        image_group->images[hints] = {image, 1};
        goto out_success;

    out_exists:
        onnxdriver_log().debug(
            "Session - The session image already exists. Increasing the reference count...");

    out_success:
        impl.group = image_group;
        impl.image = image;
        impl.hints = hints;
        impl.realPath = canonical_path;
        return true;
    }

    bool Session::close() {
        __stdc_impl_t;

        if (!impl.group)
            return false;

        const auto &path = impl.realPath;
        const auto &filename = path.filename();
        onnxdriver_log().debug("Session [%1] - close", filename);

        auto &session_system = SessionSystem::global();
        std::unique_lock<std::shared_mutex> lock(session_system.mtx);

        auto &group = *impl.group;
        auto &images = group.images;
        {
            auto it = images.find(impl.hints);
            assert(it != images.end());
            auto &data = it->second;
            if (--data.count != 0) {
                onnxdriver_log().debug("SessionImage [%1] - ref(), now ref count = %2", filename,
                                       data.count);
                goto out_success;
            }
            onnxdriver_log().debug("SessionImage [%1] - delete", filename);
            delete it->second.image;
            images.erase(it);
        }
        if (images.empty()) {
            onnxdriver_log().debug("Session - The session image group is empty. Destroying.");
            auto it = session_system.sha256_size_map.find({group.size, group.sha256});
            assert(it != session_system.sha256_size_map.end());

            auto list_it = it->second;

            session_system.sha256_size_map.erase(it);
            session_system.path_map.erase(group.path);
            session_system.image_list.erase(list_it);
        }

    out_success:
        impl.group = nullptr;
        impl.image = nullptr;
        impl.hints = 0;
        impl.realPath.clear();
        return true;
    }

    fs::path Session::path() const {
        __stdc_impl_t;
        return impl.realPath;
    }

    bool Session::isOpen() const {
        __stdc_impl_t;
        return impl.group != nullptr;
    }

    static std::vector<std::string> &shared_empty_names() {
        static std::vector<std::string> instance;
        return instance;
    }

    const std::vector<std::string> &Session::inputNames() const {
        __stdc_impl_t;
        if (!impl.image) {
            return shared_empty_names();
        }
        return impl.image->inputNames;
    }

    const std::vector<std::string> &Session::outputNames() const {
        __stdc_impl_t;
        if (!impl.image) {
            return shared_empty_names();
        }
        return impl.image->outputNames;
    }

    void Session::terminate() {
        __stdc_impl_t;
        impl.runOptions.SetTerminate();
    }

    ValueMap Session::run(const ValueMap &inputValueMap, Error *error) {
        __stdc_impl_t;
        if (!impl.group) {
            if (error) {
                *error = Error(Error::SessionError, "session is not open");
            }
            return {};
        }
        return impl.sessionRun<ValueMap>(inputValueMap, error);
    }

    SharedValueMap Session::run(const SharedValueMap &inputValueMap, Error *error) {
        __stdc_impl_t;
        if (!impl.group) {
            if (error) {
                *error = Error(Error::SessionError, "session is not open");
            }
            return {};
        }
        return impl.sessionRun<SharedValueMap>(inputValueMap, error);
    }

}