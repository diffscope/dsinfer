#ifndef TST_ONNXDRIVER_ONNXTEST_H
#define TST_ONNXDRIVER_ONNXTEST_H

#include <memory>

struct Context;

class OnnxTest {
public:
    explicit OnnxTest(Context *context);
    ~OnnxTest();
    bool initContext();
    bool initDriver();
    bool initDriver(const char *ep);
    bool testTask();
protected:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

#endif // TST_ONNXDRIVER_ONNXTEST_H
