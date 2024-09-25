#include <iostream>
#include <filesystem>

#include <dsinfer/environment.h>
#include <dsinfer/inferenceregistry.h>

namespace fs = std::filesystem;

namespace DS = dsinfer;

namespace dsinfer {

    class MyInferenceDriver : public InferenceDriver {
    public:
        MyInferenceDriver() {
        }

        bool initialize(const JsonObject &args, const std::string *error) {
            return false;
        }

        int64_t sessionCreate(const JsonObject &args) const {
            return 0;
        }
        int64_t sessionDestroy(int64_t id) const {
            return 0;
        }
        int64_t sessionAttributeGet(int64_t id, int attr, JsonValue *out) const {
            return 0;
        }
        int64_t sessionAttributeSet(int64_t id, int attr, const JsonValue &in) const {
            return 0;
        }

        int64_t taskCreate(const JsonObject &args) const {
            return 0;
        }
        int64_t taskDestroy(int64_t id) const {
            return 0;
        }
        int64_t taskStart(int64_t id) const {
            return 0;
        }
        int64_t taskStop() const {
            return 0;
        }
        int64_t taskAttributeGet(int64_t id, int attr, JsonValue *out) const {
            return 0;
        }
        int64_t taskAttributeSet(int64_t id, int attr, const JsonValue &in) const {
            return 0;
        }
    };

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("dsinfer-cli <plugin path>\n");
        return 0;
    }

    std::string error;

    // Configure environment
    DS::Environment env;
    env.addPluginPath("org.OpenVPI.InferenceInterpreter", fs::current_path() / "plugins");

    fs::path libPath = fs::current_path() / "lib";
    env.addLibraryPath(libPath);

    auto driver = new DS::MyInferenceDriver();
    if (!driver->initialize(
            {
                {"ep", "DML"}
    },
            &error)) {
        printf("Error: %s\n", error.data());
        return -1;
    }

    // Set driver
    auto inf_reg = env.registry(DS::ContributeSpec::CT_Inference)->cast<DS::InferenceRegistry>();
    inf_reg->setDriver(driver);

    // Load library
    auto lib1 = env.openLibrary(libPath / "zhibin-0.5.1.0", &error);
    if (!lib1) {
        printf("Error: %s\n", error.data());
        return -1;
    }

    // Get inference
    auto inf_spec =
        lib1->contribute(DS::ContributeSpec::CT_Inference, "pitch")->cast<DS::InferenceSpec>();

    auto inf1 = inf_spec->create({}, &error);
    if (!inf1) {
        printf("Error: %s\n", error.data());
        return -1;
    }

    if (!inf1->initialize({}, &error)) {
        printf("Error: %s\n", error.data());
        return -1;
    }

    // Prepare input
    DS::JsonObject input;
    // ...

    // Start inf1
    if (!inf1->start(input, &error)) {
        printf("Error: %s\n", error.data());
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