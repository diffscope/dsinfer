#include <iostream>
#include <filesystem>

#include <dsinferCore/interpreterloader.h>
#include <dsinferCore/environment.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("dsinfer-cli <path>\n");
        return 0;
    }

    dsinfer::Environment env;
    try {
        env.load(argv[1], dsinfer::EP_CPU);
    } catch (const std::exception &e) {
        printf("Failed to initialize: %s\n", e.what());
        return -1;
    }
    printf("Version: %s\n", env.versionString().data());

    dsinfer::InterpreterLoader ld;
    ld.addLibraryPath(std::filesystem::current_path());

    return 0;
}