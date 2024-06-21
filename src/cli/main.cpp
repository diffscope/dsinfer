#include <iostream>
#include <filesystem>

#include <dsinferCore/interpreterloader.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("dsinfer-cli <plugin path>\n");
        return 0;
    }

    dsinfer::InterpreterLoader ld;
    ld.addLibraryPath(argv[1]);

    return 0;
}