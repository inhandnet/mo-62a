# MO-62A C/C++ Edge AI 开发指南

本板搭载完整的 C/C++ AI 推理 SDK，可直接在**设备上编译、调试**你自己的推理程序，
推理由 C7x DSP（TIDL）硬件加速。无需交叉编译环境。

## 1. SDK 提供了什么

| 内容 | 位置 |
|---|---|
| 头文件（edgeai-dl-inferer API + 推理后端 + app_utils） | `/usr/include/edgeai/` |
| 静态库（dl_inferer/pre/post + TFLite 全套依赖） | `/usr/lib/edgeai/` |
| CMake 包配置 `find_package(EdgeAI)` | `/usr/lib/cmake/EdgeAI/` |
| TI 运行时（onnxruntime-TIDL 1.15 / tivision_apps，.so） | `/opt/ti/edgeai/lib/` |
| 示例工程 | `/usr/share/edgeai-cpp-examples/` |
| 模型库（ONNX/TFLite，含 TIDL artifacts） | `/opt/model_zoo/` |

开发依赖（cmake、g++、opencv/gstreamer/yaml-cpp 的 -dev 包）已随镜像预装。

## 2. 写一个最小推理程序

一个 `CMakeLists.txt` 就够了——`find_package(EdgeAI)` 会带上所有 include 路径和链接库：

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_infer CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(EdgeAI REQUIRED)

add_executable(my_infer main.cpp)
target_link_libraries(my_infer PRIVATE EdgeAI::edgeai)
```

`main.cpp` 加载模型并推理的核心调用（完整可运行版见
`/usr/share/edgeai-cpp-examples/hello_inference/`）：

```cpp
#include <ti_dl_inferer.h>
using namespace ti::dl_inferer;

InfererConfig cfg;
cfg.getConfig("/opt/model_zoo/<model-dir>", true, 1);   // 读 param.yaml
DLInferer *inf = DLInferer::makeInferer(cfg);            // 选 TFLite/ONNX(TIDL)/DLR
inf->dumpInfo();                                         // 打印输入/输出张量

VecDlTensorPtr in, out;
inf->createBuffers(inf->getOutputInfo(), out, true);
inf->createBuffers(inf->getInputInfo(),  in,  true);
// ... 把预处理后的帧写入 in[0]->data ...
inf->run(in, out);                                       // 推理（TIDL 模型走 C7x）
// ... 从 out[i]->data / shape / numElem 读结果 ...
delete inf;
```

编译运行：

```bash
mkdir build && cd build && cmake .. && make
sudo LD_LIBRARY_PATH=/opt/ti/edgeai/lib ./my_infer
```

## 3. 两个示例

- **`hello_inference/`** — 最小例子：加载模型 + 用零输入跑一次推理 + 打印输出。
  纯 headless，不需要摄像头/显示器，是验证环境最快的方式。
  ```bash
  cd /usr/share/edgeai-cpp-examples/hello_inference
  mkdir build && cd build && cmake .. && make
  sudo LD_LIBRARY_PATH=/opt/ti/edgeai/lib ./hello_inference \
      -m /opt/model_zoo/ONR-OD-8200-yolox-nano-lite-mmdet-coco-416x416
  ```
- **`app_edgeai/`** — 完整摄像头 → 推理 → HDMI 流水线（Python `app_edgeai.py` 的 C++ 对应），
  接受相同的 YAML 配置（见 `configs/`）。把它当作自己应用的起点。

## 4. 运行须知（坑）

- **需要 root**：访问 C7x / VPAC / DRM 需要 `/dev/mem`、dma_heap、rpmsg 权限，普通用户会报
  `Create state function failed. Return value:-1`。用 `sudo`（带显示时用 `sudo -E` 保留环境变量）。
- **ONNX Runtime 必须是 TI 版**：`/opt/ti/edgeai/lib/libonnxruntime.so`（1.15，含 TIDL EP），
  不是 Debian 的 `libonnxruntime`（1.21，无 TIDL 符号）。SDK 已默认链接 TI 版。
- **kmssink 显示**先停桌面释放 DRM：`sudo systemctl stop lightdm`，结束后 `sudo systemctl start lightdm`。
- **环境变量**（HDMI 全流程需要）：
  ```bash
  export SOC=am62a
  export GST_PLUGIN_PATH=/opt/ti/edgeai/gstreamer/plugins
  export LD_LIBRARY_PATH=/opt/ti/edgeai/lib:/opt/ti/edgeai/gstreamer/lib
  ```
- **省事跑现成 demo**：`edgeai-demo run <model> --backend cpp`（自动处理上面所有事项）。

## 5. 与 Python demo 的关系

`edgeai-demo` 统一入口同时支持两种后端，YAML 配置完全通用：

```bash
edgeai-demo                                   # 交互：选 摄像头 / 模型 / backend
edgeai-demo run <model> --backend cpp         # C/C++ 后端
edgeai-demo run <model> --backend python      # Python 后端
```
