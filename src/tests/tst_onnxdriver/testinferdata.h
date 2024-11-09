#ifndef TST_ONNXDRIVER_TESTINFERDATA_H
#define TST_ONNXDRIVER_TESTINFERDATA_H

#include <dsinfer/jsonvalue.h>

class TestInferData {
public:
    static dsinfer::JsonValue generate(int64_t data_id, int64_t session_id, int64_t context_id);
    static int64_t count();
};

#endif // TST_ONNXDRIVER_TESTINFERDATA_H