#include <iostream>
#include <filesystem>

#include <dsinferCore/interpreterloader.h>
#include <dsinferCore/environment.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("dsinfer-cli <onnxruntime path> <plugin path>\n");
        return 0;
    }

    dsinfer::Environment env;
    if (std::string err; !env.load(argv[1], dsinfer::EP_CPU, &err)) {
        printf("Failed to initialize: %s\n", err.data());
        return -1;
    }

    printf("Version: %s\n", env.versionString().data());

    dsinfer::InterpreterLoader ld;
    ld.addLibraryPath(argv[2]);

    return 0;
}