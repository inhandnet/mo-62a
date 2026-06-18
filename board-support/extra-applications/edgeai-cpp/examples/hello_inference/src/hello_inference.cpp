/*
 * hello_inference — minimal Edge AI inference example for MO-62A.
 *
 * Loads a TIDL-compiled model from /opt/model_zoo (or any model directory with
 * a param.yaml + artifacts/), runs a single inference on a zero-filled input
 * buffer, and prints the output tensors. No camera or display is needed, so it
 * is the smallest possible end-to-end check that the C7x offload works.
 *
 * Build (on the board):
 *     mkdir build && cd build && cmake .. && make
 * Run:
 *     ./hello_inference -m /opt/model_zoo/<a-model-dir>
 *
 * The only SDK call surface used here is ti::dl_inferer::DLInferer. To build a
 * full camera->infer->HDMI pipeline, see the app_edgeai example next to this one.
 */
#include <cstdio>
#include <cstring>
#include <string>

#include <ti_dl_inferer.h>
#include <ti_dl_inferer_logger.h>

using namespace std;
using namespace ti::dl_inferer;
using namespace ti::dl_inferer::utils;

static void usage(const char *name)
{
    printf("Usage: %s -m <model-dir> [-l <0..3>]\n", name);
    printf("  -m  model directory (contains param.yaml + artifacts/)\n");
    printf("  -l  log level: 0 DEBUG, 1 INFO, 2 WARN (default), 3 ERROR\n");
}

int main(int argc, char *argv[])
{
    string   modelDir;
    LogLevel logLevel = WARN;

    for (int i = 1; i < argc; ++i)
    {
        string a = argv[i];
        if (a == "-m" && i + 1 < argc)       modelDir = argv[++i];
        else if (a == "-l" && i + 1 < argc)  logLevel = static_cast<LogLevel>(strtol(argv[++i], nullptr, 0));
        else { usage(argv[0]); return (a == "-h" || a == "--help") ? 0 : 1; }
    }
    if (modelDir.empty()) { usage(argv[0]); return 1; }
    logSetLevel(logLevel);

    // 1. Build the inferer config from the model's param.yaml.
    InfererConfig infConfig;
    if (infConfig.getConfig(modelDir, true, 1) < 0)
    {
        printf("ERROR: failed to read config from %s/param.yaml\n", modelDir.c_str());
        return 1;
    }

    // 2. Create the runtime inferer (TFLite / ONNX(TIDL) / DLR, picked from config).
    DLInferer *inferer = DLInferer::makeInferer(infConfig);
    if (inferer == nullptr)
    {
        printf("ERROR: makeInferer() failed for %s\n", modelDir.c_str());
        return 1;
    }

    printf("\n=== Model: %s ===\n", modelDir.c_str());
    inferer->dumpInfo();   // prints input/output tensor names, shapes, dtypes

    // 3. Allocate input/output buffers straight from the model interface info.
    VecDlTensorPtr inputs;
    VecDlTensorPtr outputs;
    int32_t status = inferer->createBuffers(inferer->getOutputInfo(), outputs, true);
    if (status == 0)
        status = inferer->createBuffers(inferer->getInputInfo(), inputs, true);
    if (status < 0)
    {
        printf("ERROR: createBuffers() failed.\n");
        delete inferer;
        return 1;
    }

    // 4. Feed a zero input (real apps fill this from a pre-processed frame).
    for (auto *t : inputs)
        if (t->data != nullptr) memset(t->data, 0, t->size);

    // 5. Run one inference. With a TIDL model this dispatches to the C7x DSP.
    printf("\nRunning one inference on a zero input...\n");
    status = inferer->run(inputs, outputs);
    if (status < 0)
    {
        printf("ERROR: inference failed.\n");
        delete inferer;
        return 1;
    }

    // 6. Report the outputs.
    printf("Inference OK. %zu output tensor(s):\n", outputs.size());
    for (size_t i = 0; i < outputs.size(); ++i)
    {
        const DlTensor *t = outputs[i];
        printf("  out[%zu] %-24s type=%-10s elems=%lld\n",
               i, t->name ? t->name : "(unnamed)",
               t->typeName ? t->typeName : "?",
               static_cast<long long>(t->numElem));
    }

    delete inferer;
    printf("\nDone. See the app_edgeai example for a full camera + HDMI pipeline.\n");
    return 0;
}
