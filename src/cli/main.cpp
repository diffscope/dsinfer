#include <iostream>

#include <dsinfer/environment.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("dsinfer-cli <path>\n");
        return 0;
    }

    dsinfer::Environment env;
    try {
        env.load(argv[1], dsinfer::EP_CPU, 0);
    } catch (const std::exception &e) {
        printf("Failed to initialize: %s\n", e.what());
        return -1;
    }
    printf("Version: %s\n", env.versionString().data());

    auto sessMgr = dsEnv->sessionManager();
    if (!sessMgr) {
        return -2;
    }
    auto modelHandle = sessMgr->create("model.onnx");
    auto model = sessMgr->get(modelHandle);

    if (!model) {
        return -3;
    }

    if (model->isLoaded()) {
        std::cout << "Model is loaded!" << '\n';
        std::cout << "[Input names]" << '\n';
        for (const auto &name : model->inputNames()) {
            std::cout << name << '\n';
        }

        std::cout << "[Output names]" << '\n';
        for (const auto &name : model->outputNames()) {
            std::cout << name << '\n';
        }

        std::cout << "Model use count = " << model->useCount() << '\n';

        {
            auto model2 = sessMgr->get(modelHandle);
            std::cout << "Model use count = " << model->useCount() << '\n';
        }

        std::cout << "Model use count = " << model->useCount() << '\n';
    } else {
        std::cout << "Model is not loaded!" << '\n';
    }

    return 0;
}