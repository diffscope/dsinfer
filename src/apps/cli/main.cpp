#include <iostream>
#include <filesystem>

#include <dsinfer/environment.h>
#include <dsinfer/inferenceregistry.h>
#include <dsinfer/format.h>

namespace fs = std::filesystem;

namespace DS = dsinfer;

namespace dsinfer {

    class MyInferenceDriver : public InferenceDriver {
    public:
        MyInferenceDriver() {
        }

    public:
        bool initialize(const JsonObject &args, Error *error) const override {
            return {};
        }
        int64_t sessionCreate(const std::filesystem::path &path, const JsonObject &args,
                              Error *error) const override {
            return {};
        }
        bool sessionDestroy(int64_t handle, Error *error) const override {
            return {};
        }
        bool sessionRunning(int64_t handle) const override {
            return {};
        }
        int64_t taskCreate() const override {
            return {};
        }
        void taskDestroy(int64_t handle) const override {
        }
        bool taskStart(int64_t handle, const JsonValue &input, Error *error) const override {
            return {};
        }
        bool taskStop(int64_t handle, Error *error) const override {
            return {};
        }
        int taskState(int64_t handle) const override {
            return {};
        }
        bool taskResult(int64_t handle, JsonValue *result) const override {
            return {};
        }
    };

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("dsinfer-cli <plugin path>\n");
        return 0;
    }

    DS::Error error;

    // Configure environment
    DS::Environment env;
    env.addPluginPath("org.OpenVPI.InferenceInterpreter", fs::current_path().parent_path() / "lib" /
                                                              "plugins" / "dsinfer" /
                                                              "inferenceinterpreters");

    fs::path libPath = fs::current_path() / "lib";
    env.addLibraryPath(libPath);

    auto driver = new DS::MyInferenceDriver();
    if (!driver->initialize(
            {
                {"ep", "DML"},
    },
            &error)) {
        printf("Error: %s\n", error.what());
        return -1;
    }

    // Set driver
    auto inf_reg = env.registry(DS::ContributeSpec::Inference)->cast<DS::InferenceRegistry>();
    inf_reg->setDriver(driver);

    // Load library
    auto lib1 = env.openLibrary(libPath / "zhibin-0.5.1.0", &error);
    if (!lib1) {
        printf("Error: %s\n", error.what());
        return -1;
    }

    // Get inference
    auto inf_spec =
        lib1->contribute(DS::ContributeSpec::Inference, "pitch")->cast<DS::InferenceSpec>();

    auto inf1 = inf_spec->create({}, &error);
    if (!inf1) {
        printf("Error: %s\n", error.what());
        return -1;
    }

    if (!inf1->initialize({}, &error)) {
        printf("Error: %s\n", error.what());
        return -1;
    }

    // Prepare input
    DS::JsonObject input;
    // ...

    // Start inf1
    if (!inf1->start(input, &error)) {
        printf("Error: %s\n", error.what());
        return -1;
    }

    while (inf1->state() == DS::Inference::Running) {
        _sleep(1000);
    }

    // Get result
    auto result = inf1->result();

    // Process result
    // ...

    return 0;
}