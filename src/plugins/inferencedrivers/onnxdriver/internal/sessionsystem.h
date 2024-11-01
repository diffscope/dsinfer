#ifndef DSINFR_ONNXDRIVER_SESSIONSYSTEM_H
#define DSINFR_ONNXDRIVER_SESSIONSYSTEM_H

#include <map>
#include <filesystem>
#include <shared_mutex>

namespace dsinfer::onnxdriver {

class SessionImage;

class SessionSystem {
public:
    SessionSystem();
    ~SessionSystem();

    SessionSystem(const SessionSystem &) = delete;
    SessionSystem &operator=(const SessionSystem &) = delete;

    static SessionSystem *instance();

    bool addImage(const std::filesystem::path &path, SessionImage *image, bool overwrite = false);
    bool removeImage(const std::filesystem::path &path);
    SessionImage *getImage(const std::filesystem::path &path);
private:
    mutable std::shared_mutex mtx;
    std::map<std::filesystem::path, SessionImage *> sessionImageMap;
};

}

#endif // DSINFR_ONNXDRIVER_SESSIONSYSTEM_H
