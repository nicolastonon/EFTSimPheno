#ifndef common_TFModel_h
#define common_TFModel_h

#include <string>
#include <vector>

#include "tensorflow/core/public/session.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/cc/saved_model/loader.h"
#include "tensorflow/cc/saved_model/tag_constants.h"
#include "tensorflow/cc/saved_model/constants.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/util/tensor_bundle/naming.h"

//NEW
// #include "tensorflow/cc/ops/standard_ops.h"
// #include "tensorflow/core/framework/graph.pb.h"
// #include "tensorflow/core/framework/tensor.h"
// #include "tensorflow/core/graph/default_device.h"
// #include "tensorflow/core/graph/graph_def_builder.h"
// #include "tensorflow/core/lib/core/threadpool.h"
// #include "tensorflow/core/lib/strings/str_util.h"
// #include "tensorflow/core/lib/strings/stringprintf.h"
// #include "tensorflow/core/platform/init_main.h"
// #include "tensorflow/core/platform/logging.h"
// #include "tensorflow/core/platform/types.h"
// #include "tensorflow/core/public/session.h"
// #include "tensorflow/core/framework/tensor.h"
// #include "tensorflow/cc/saved_model/loader.h"
// #include "tensorflow/cc/saved_model/tag_constants.h"
// #include "tensorflow/cc/saved_model/constants.h"
// #include "tensorflow/core/lib/io/path.h"
// #include "tensorflow/core/util/tensor_bundle/naming.h"

#include "CMSSW_TensorFlow.h"
//#include "PhysicsTools/TensorFlow/interface/TensorFlow.h" //CMSSW

// #include "common/include/Logging.h"

class TFModel {
public:
    // Analysis routine
    TFModel(const std::string &model_name, const unsigned _n_inputs,
            const std::string &_input_name, const unsigned _n_outputs,
            const std::string &_output_name);
    ~TFModel();
    std::vector<float> evaluate(float inputs[]);
    // std::vector<float> evaluate(const double inputs[]);

private:
    //NICOLAS -- CHANGED
    // void *graphDef = nullptr;
    // void *session = nullptr;

    tensorflow::Session* session = nullptr;
    tensorflow::GraphDef* graphDef = nullptr;

    const unsigned n_inputs;
    const unsigned n_outputs;
    const std::string input_name;
    const std::string output_name;
};

#endif // ifndef common_Analyzer_h
