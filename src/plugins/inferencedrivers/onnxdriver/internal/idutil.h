#ifndef DSINFR_ONNXDRIVER_IDUTIL
#define DSINFR_ONNXDRIVER_IDUTIL

#include <map>
#include <mutex>
#include <shared_mutex>

namespace dsinfer {

    class IdGenerator {
    public:
        inline int64_t generate() {
            return ++idCounter;
        }
    private:
        int64_t idCounter = 0;
    };


    template <typename T>
    class IdManager {
    public:
        inline int64_t add(T *obj) {
            std::unique_lock<std::shared_mutex> lock(mtx);
            auto id_ = idGenerator.generate();
            idMap[id_] = obj;
            return id_;
        }

        inline bool remove(int64_t id_) {
            std::unique_lock<std::shared_mutex> lock(mtx);
            if (auto it = idMap.find(id_); it != idMap.end()) {
                idMap.erase(it);
                return true;
            }
            return false;
        }

        inline T *get(int64_t id_) {
            std::shared_lock<std::shared_mutex> lock(mtx);
            if (auto it = idMap.find(id_); it != idMap.end()) {
                return it->second;
            }
            return nullptr;
        }

    private:
        std::shared_mutex mtx;
        IdGenerator idGenerator;
        std::map<int64_t, T *> idMap;
    };

}

#endif // DSINFR_ONNXDRIVER_IDUTIL