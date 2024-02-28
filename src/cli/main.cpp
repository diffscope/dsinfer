#include <iostream>
#include <dsinfer/dsinfer_capi.h>

int main(int argc, char *argv[]) {
    std::cout << "Hello World!" << std::endl;
    dsinfer_init("onnxruntime.dll", EP_CPU);
    return 0;
}