#include "onnxtest.h"

#include <cstdint>
#include <fstream>
#include <limits>

#include <stdcorelib/console.h>
#include <stdcorelib/pimpl.h>

#include <dsinfer/contributespec.h>
#include <dsinfer/error.h>
#include <dsinfer/inferenceregistry.h>
#include <dsinfer/inferencedriver.h>
#include <dsinfer/jsonvalue.h>

#include "context.h"
#include "valueutils.h"
#include "testinferdata.h"


#define ENSURE_CTX(ctx)                                                                            \
    do {                                                                                           \
        if (!ctx) {                                                                                \
            stdc::console::printf(stdc::console::nostyle,                                                \
                                  stdc::console::red | stdc::console::intensified,                 \
                                  stdc::console::nocolor, "Context is nullptr!\n");                  \
            return false;                                                                          \
        }                                                                                          \
    } while (false)

#define ENSURE_OK_ERROR_CONSISTENT(name, ok, error)                                                \
    do {                                                                                           \
        if (ok ^ error.ok()) {                                                                     \
            logger.fatal("Bugs found in code: " name " return value "                              \
                         "is not consistent with dsinfer::Error::ok()");                           \
            return false;                                                                          \
        }                                                                                          \
    } while (false)


namespace fs = std::filesystem;
namespace DS = dsinfer;
namespace VU = ValueUtils;


template <typename T>
bool insertObjectHelper(DS::Log::Category &logger, DS::InferenceContext *context, const char *key,
                        const std::vector<T> &inputData) {
    logger.info(R"(Inserting new object "%1" to OnnxContext %2)", key, context->id());
    bool ok = context->insertObject(key, VU::toContextObj<T>(inputData));
    if (!ok) {
        logger.critical(R"(Failed to insert object "%1" to OnnxContext %2)", key, context->id());
        return false;
    }

    // Getting objects from context
    auto obj = context->getObject(key);
    auto objContent = obj["content"];
    if (objContent.isUndefined()) {
        logger.critical(R"(Failed to get object "%1" from OnnxContext %2: "content" is missing)",
                        key, context->id());
        return false;
    }
    logger.debug(R"(Content of "%1": %2)", key, VU::inferValueStringify(objContent));
    return true;
}

class OnnxTest::Impl {
public:
    Context *ctx = nullptr;
    DS::InferenceDriver *driver;
};


OnnxTest::OnnxTest(Context *context) : _impl(std::make_unique<Impl>()) {
    __stdc_impl_t;
    impl.ctx = context;
}

OnnxTest::~OnnxTest() = default;

bool OnnxTest::initDriver() {
    return initDriver("cpu");
}

bool OnnxTest::initDriver(const char *ep) {
    __stdc_impl_t;
    ENSURE_CTX(impl.ctx);
    auto &logger = impl.ctx->logger;

    auto inferenceReg =
        impl.ctx->env.registry(DS::ContributeSpec::Inference)->cast<DS::InferenceRegistry>();

    DS::Error error;
    bool ok = inferenceReg->setup("onnx",
                                  DS::JsonObject({
                                      {"ep",          ep },
                                      {"deviceIndex", "0"},
    }),
                                  &error);
    ENSURE_OK_ERROR_CONSISTENT("OnnxDriver::initialize", ok, error);
    if (!ok) {
        logger.critical(error.what());
        return false;
    }
    impl.driver = inferenceReg->driver();
    return true;
}
bool OnnxTest::testTask() {
    __stdc_impl_t;
    ENSURE_CTX(impl.ctx);
    auto &logger = impl.ctx->logger;

    if (!impl.driver) {
        logger.critical("Onnx driver plugin is not loaded!");
    }

    DS::Error error;
    bool ok = true;

    // ========== Utility Functions ==========
    // Create new session
    const auto newSession = [&](const fs::path &modelPath, bool useCpuHint,
                                std::shared_ptr<DS::InferenceSession> *outSession) -> bool {
        logger.info("Creating new OnnxSession");
        if (!outSession) {
            logger.critical("outSession must not be nullptr!");
            return false;
        }
        std::shared_ptr<DS::InferenceSession> session(impl.driver->createSession());

        if (!session) {
            logger.critical("Failed to create OnnxSession");
            return false;
        }
        logger.info("OnnxSession %1 created", session->id());

        // Open onnx model
        logger.info("Opening OnnxSession %1 using model %2", session->id(), modelPath);

        bool ok = session->open(modelPath,
                                dsinfer::JsonObject{
                                    {"useCpuHint", useCpuHint}
        },
                                &error);
        ENSURE_OK_ERROR_CONSISTENT("OnnxSession::open", ok, error);
        if (!ok) {
            logger.critical(error.what());
            return false;
        }

        *outSession = session;
        return true;
    };

    // ========== Test OnnxSession ==========
    fs::path model1Path(_TSTR("test_data/onnx_models/vector_rss_sigmoid.onnx"));
    std::shared_ptr<DS::InferenceSession> session1;
    if (!newSession(model1Path, false, &session1)) {
        return false;
    }

    fs::path model2Path(_TSTR("test_data/onnx_models/vector_add.onnx"));
    std::shared_ptr<DS::InferenceSession> session2;
    if (!newSession(model2Path, false, &session2)) {
        return false;
    }

    // Test loading the same model (paths are different, but contents are identical)
    fs::path model3Path(_TSTR("test_data/onnx_models/vector_add-duplicate.onnx"));
    std::shared_ptr<DS::InferenceSession> session3;
    if (!newSession(model3Path, false, &session3)) {
        return false;
    }

    // Test loading the model whose filesize is the same as `vector_add.onnx`,
    // but the contents are different
    fs::path model4Path(_TSTR("test_data/onnx_models/vector_add-same_filesize.onnx"));
    std::shared_ptr<DS::InferenceSession> session4;
    if (!newSession(model4Path, false, &session4)) {
        return false;
    }

    // ========== Test OnnxContext ==========
    // TODO: InferenceContext should add a static interface to get context by ID,
    //       so the caller don't need a map to store contexts.
    std::map<int64_t, std::shared_ptr<DS::InferenceContext>> contextMap;
    // Create new context
    logger.info("Creating new OnnxContext");
    std::shared_ptr<DS::InferenceContext> context(impl.driver->createContext());
    if (!context) {
        logger.critical("Failed to create OnnxContext");
        return false;
    }
    contextMap[context->id()] = context;
    logger.info("OnnxContext %1 created", context->id());

    // Insert objects to context

    if (!insertObjectHelper<float>(logger, context.get(), "test_input1",
                                   {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f})) {
        return false;
    }
    if (!insertObjectHelper<float>(logger, context.get(), "test_input2",
                                   {3.0f, 6.0f, 7.0f, -8.0f, -9.0f, 12.0f})) {
        return false;
    }
    if (!insertObjectHelper<float>(logger, context.get(), "pitch",
                                   {66.2008, 66.1732, 66.1448, 66.1164, 66.072, 66.0396})) {
        return false;
    }
    if (!insertObjectHelper<int64_t>(logger, context.get(), "tokens", {15, 24, 16, 8, 32, 19})) {
        return false;
    }
    if (!insertObjectHelper<bool>(logger, context.get(), "retake",
                                  {false, false, true, true, true, true})) {
        return false;
    }
    printf("\n");

    // ========== Test OnnxTask ==========
    auto totalCases = TestInferData::count() + 1; // 1 extra case
    for (int dataId = 1; dataId <= totalCases; ++dataId) {
        logger.info("Running OnnxTask test case %1/%2", dataId, totalCases);
        // Create OnnxTask
        logger.info("Creating OnnxTask");
        std::shared_ptr<DS::InferenceTask> task(impl.driver->createTask());
        if (!task) {
            logger.critical("Failed to create OnnxTask");
            return false;
        }

        // Initialize OnnxTask
        ok = task->initialize({}, &error);
        ENSURE_OK_ERROR_CONSISTENT("OnnxTask::initialize", ok, error);
        if (!ok) {
            logger.critical(error.what());
            return false;
        }

        // Read test cases
        // std::filesystem::path taskInputJsonPath(
        //    _TSTR("test_data/input_data/input-two_float_vectors-1.json"));
        // auto inputFormat = VU::jsonValueFromPath(taskInputJsonPath);

        DS::JsonValue inputFormat;
        if (dataId <= TestInferData::count()) {
            inputFormat = TestInferData::generate(dataId, session1->id(), context->id());
        } else {
            inputFormat = DS::JsonObject{
                {"session", session1->id()},
                {"context", context->id()},
                {
                 "input", DS::JsonArray{DS::JsonObject{{"name", "input1"},
                                                 {"format", "reference"},
                                                 {
                                                     "data",
                                                     DS::JsonObject{{"value", "test_input1"}},
                                                 }},
                                  DS::JsonObject{
                                      {"name", "input2"},
                                      {"format", "reference"},
                                      {"data", DS::JsonObject{{"value", "test_input2"}}},
                                  }},
                 },
                {"output",
                 DS::JsonArray{DS::JsonObject{{"name", "output"}, {"format", "reference"}}}}
            };
        }
        if (inputFormat.isUndefined()) {
            return false;
        }
        int64_t contextId = 0;
        if (auto cid = inputFormat["context"]; cid.isUndefined()) {
            logger.warning("Context id is not specified in input json!");
            // Let OnnxTask::start handle it, so we don't return false here.
            // return false;
        } else {
            contextId = cid.toInt64();
        }

        do {
            auto inputFormatInput = inputFormat["input"];
            if (inputFormatInput.isUndefined()) {
                logger.warning(R"(Input json does not contain key "input"!)");
                // Let OnnxTask::start handle it, so we don't return false here.
                // return false;
                break;
            }
            logger.debug("Getting input values");
            auto inputArray = inputFormatInput.toArray();
            for (size_t i = 0; i < inputArray.size(); ++i) {
                logger.debug("[%1/%2] %3", i + 1, inputArray.size(),
                             VU::inferValueStringify(inputArray[i]));
            }
        } while (false);

        // Start OnnxTask
        ok = task->start(inputFormat, &error);
        ENSURE_OK_ERROR_CONSISTENT("OnnxTask::start", ok, error);
        if (!ok) {
            logger.critical(error.what());
            return false;
        }

        auto result = task->result();
        logger.debug("Result for OnnxTask %1: ", task->id());
        auto resultArr = result.toArray();
        for (size_t i = 0; i < resultArr.size(); ++i) {
            logger.debug("[%1/%2] %3", i + 1, resultArr.size(),
                         VU::inferValueStringify(resultArr[i]));
            if (resultArr[i]["format"].toString() == "reference") {
                auto key = resultArr[i]["data"]["value"].toString();
                logger.debug(R"(Getting key "%1" from context %2)", key, contextId);
                auto it = contextMap.find(contextId);
                if (it == contextMap.end()) {
                    logger.critical("Context %1 not found in contextMap!", contextId);
                }
                auto obj = it->second->getObject(key);
                logger.debug(VU::inferValueStringify(obj["content"]));
            }
        }

        printf("\n");
    }

    logger.info("Finished OnnxTask test cases");

    // Check OnnxContext keys
    DS::JsonValue cmdOutput;
    context->executeCommand(
        DS::JsonObject{
            {"command", "list"}
    },
        &cmdOutput);
    logger.debug("OnnxContext keys: %1", cmdOutput.toJson());

    // Close OnnxSession
    {
        ok = session1->close(&error);
        ENSURE_OK_ERROR_CONSISTENT("OnnxTask::close", ok, error);
        if (!ok) {
            logger.critical(error.what());
            return false;
        }
    }
    {
        ok = session2->close(&error);
        ENSURE_OK_ERROR_CONSISTENT("OnnxTask::close", ok, error);
        if (!ok) {
            logger.critical(error.what());
            return false;
        }
    }

    return true;
}
