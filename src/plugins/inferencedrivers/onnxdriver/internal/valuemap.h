#ifndef DSINFR_ONNXDRIVER_VALUEMAP_H
#define DSINFR_ONNXDRIVER_VALUEMAP_H

#include <map>

#include <onnxruntime_cxx_api.h>

namespace dsinfer::onnxdriver {

    using ValueMap = std::map<std::string, Ort::Value>;

}

#endif // DSINFR_ONNXDRIVER_VALUEMAP_H
