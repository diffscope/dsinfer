#include "onnxtest.h"

#include <fstream>

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
            stdc::Console::printf(stdc::Console::Red | stdc::Console::Intensified,                 \
                                  stdc::Console::Default, "Context is nullptr!\n");                \
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


class OnnxTest::Impl {
public:
    Context *ctx = nullptr;
    std::shared_ptr<DS::InferenceDriver> driver;
};


OnnxTest::OnnxTest(Context *context) : _impl(std::make_unique<Impl>()) {
    __stdc_impl_t;
    impl.ctx = context;
}

OnnxTest::~OnnxTest() = default;

bool OnnxTest::initContext() {
    __stdc_impl_t;
    ENSURE_CTX(impl.ctx);
    auto &logger = impl.ctx->logger;

    auto inferenceReg = impl.ctx->env.registry(DS::ContributeSpec::Inference)->cast<DS::InferenceRegistry>();
    impl.driver.reset(inferenceReg->createDriver("onnx"));
    if (!impl.driver) {
        logger.critical(R"(Failed to load driver "onnx")");
        return false;
    }
    return true;
}

bool OnnxTest::initDriver() {
    return initDriver("cpu");
}

bool OnnxTest::initDriver(const char *ep) {
    __stdc_impl_t;
    ENSURE_CTX(impl.ctx);
    auto &logger = impl.ctx->logger;

    if (!impl.driver) {
        logger.critical("Onnx driver plugin is not loaded!");
    }

    DS::Error error;
    bool ok = true;

    ok = impl.driver->initialize(DS::JsonObject {
        {"ep", ep},
        {"deviceIndex", "0"}
    }, &error);
    ENSURE_OK_ERROR_CONSISTENT("OnnxDriver::initialize", ok, error);
    if (!ok) {
        logger.critical(error.what());
        return false;
    }
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

    ValueUtils vu(&logger);
    // ========== Utility Functions ==========
    // Create new session
    const auto newSession = [&](const fs::path &modelPath,
                                bool useCpuHint,
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
        }, &error);
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
    auto generateInputObj = [](const std::vector<float> &input_data) {
        auto buffer = reinterpret_cast<const uint8_t *>(input_data.data());
        auto bufferSize = input_data.size() * sizeof(float);
        return DS::JsonObject {
            {"type",    "object"                                                                },
            {"content",
             DS::JsonObject{
                 {"class", "Ort::Value"},
                 {"format", "bytes"},
                 {"data",
                  DS::JsonObject{{"type", "float"},
                                 {"shape", DS::JsonArray{(float) input_data.size()}},
                                 {"value", std::vector<uint8_t>(buffer, buffer + bufferSize)}}}}}
        };
    };

    logger.info("Inserting new object %1 to OnnxContext %2", "input1", context->id());
    ok = context->insertObject("input1", generateInputObj({1.0f, 2.0f, 3.0f, 4.0f}));
    if (!ok) {
        logger.critical("Failed to insert object %1 to OnnxContext %2", "input1", context->id());
        return false;
    }

    logger.info("Inserting new object %1 to OnnxContext %2", "input2", context->id());
    ok = context->insertObject("input2", generateInputObj({5.0f, 6.0f, 7.0f, 8.0f}));
    if (!ok) {
        logger.critical("Failed to insert object %1 to OnnxContext %2", "input2", context->id());
        return false;
    }

    // Getting objects from context
    auto obj_input1 = context->getObject("input1");
    logger.debug("Content of input1: %1", vu.inferValueStringify(obj_input1["content"]));

    // ========== Test OnnxTask ==========
    for (int dataId = 1; dataId < TestInferData::count() + 1; ++dataId) {
        logger.info("Running OnnxTask test case %1/%2", dataId, TestInferData::count());
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
        //std::filesystem::path taskInputJsonPath(
        //    _TSTR("test_data/input_data/input-two_float_vectors-1.json"));
        //auto inputFormat = vu.jsonValueFromPath(taskInputJsonPath);

        auto inputFormat = TestInferData::generate(
            dataId,
            (dataId % 2) ? session1->id() : session2->id(),
            context->id());
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
                logger.warning("Input json does not contain key \"input\"!");
                // Let OnnxTask::start handle it, so we don't return false here.
                // return false;
                break;
            }
            logger.debug("Getting input values");
            auto inputArray = inputFormatInput.toArray();
            for (size_t i = 0; i < inputArray.size(); ++i) {
                logger.debug("[%1/%2] %3", i + 1, inputArray.size(),
                             vu.inferValueStringify(inputArray[i]));
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
                         vu.inferValueStringify(resultArr[i]));
            if (resultArr[i]["format"].toString() == "reference") {
                auto key = resultArr[i]["data"]["value"].toString();
                logger.debug("Getting key \"%1\" from context %2", key, contextId);
                auto it = contextMap.find(contextId);
                if (it == contextMap.end()) {
                    logger.critical("Context %1 not found in contextMap!", contextId);
                }
                auto obj = it->second->getObject(key);
                logger.debug(vu.inferValueStringify(obj["content"]));
            }
        }
    }

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
