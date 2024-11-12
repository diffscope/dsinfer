#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <dsinfer/environment.h>
#include <dsinfer/inferenceregistry.h>
#include <dsinfer/inferencespec.h>
#include <dsinfer/log.h>

#include "context.h"
#include "onnxtest.h"

namespace fs = std::filesystem;

namespace DS = dsinfer;

int main(int argc, char *argv[]) {
    Context ctx;
    DS::Log::setCallback(log_report_callback);
    DS::Log::setLevel(DS::Log::Debug);

    bool ok = true;
    OnnxTest test(&ctx);

    ok = test.initDriver("cpu");
    if (!ok) {
        ctx.logger.critical("initDriver - test failed");
        return EXIT_FAILURE;
    }

    ok = test.testTask();
    if (!ok) {
        ctx.logger.critical("testTask - test failed");
        return EXIT_FAILURE;
    }

    ctx.logger.info("All tests completed");
    return EXIT_SUCCESS;
}