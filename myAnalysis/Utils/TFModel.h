#ifndef common_TFModel_h
#define common_TFModel_h

#include <string>
#include <vector>

#include "/home/ntonon/Documents/Programmes/tensorflow/tensorflow/core/public/session.h"
#include "/home/ntonon/Documents/Programmes/tensorflow/tensorflow/core/framework/tensor.h"
#include "/home/ntonon/Documents/Programmes/tensorflow/tensorflow/cc/saved_model/loader.h"
#include "/home/ntonon/Documents/Programmes/tensorflow/tensorflow/cc/saved_model/tag_constants.h"
#include "/home/ntonon/Documents/Programmes/tensorflow/tensorflow/cc/saved_model/constants.h"
#include "/home/ntonon/Documents/Programmes/tensorflow/tensorflow/core/lib/io/path.h"
#include "/home/ntonon/Documents/Programmes/tensorflow/tensorflow/core/util/tensor_bundle/naming.h"

#include "CMSSW_TensorFlow.h"

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
    void *graphDef = nullptr;
    void *session = nullptr;

    const unsigned n_inputs;
    const unsigned n_outputs;
    const std::string input_name;
    const std::string output_name;
};

#endif // ifndef common_Analyzer_h
