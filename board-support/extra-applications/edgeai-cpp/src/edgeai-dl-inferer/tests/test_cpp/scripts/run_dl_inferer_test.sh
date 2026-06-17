#! /bin/bash

# How to RUN
#   ../bin/Release/app_dl_inferer_test -d *model_directory* -i *image_path* -v *viz-threshold (Default=0.6)* -t *top-n class (Default=5)* -a *alpha (Default=0.4)* -e Turns off TIDL

# Classification
../bin/Release/app_dl_inferer_test -d /opt/model_zoo/TFL-CL-0000-mobileNetV1-mlperf/ -i $EDGEAI_DATA_PATH/images/0001.jpg -t 5
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/ONR-CL-6360-regNetx-200mf/      -i $EDGEAI_DATA_PATH/images/0002.jpg -t 5
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/TVM-CL-3090-mobileNetV2-tv/     -i $EDGEAI_DATA_PATH/images/0003.jpg -t 5
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/TVM-CL-3520-mobileNetV1/        -i $EDGEAI_DATA_PATH/images/0004.jpg -t 5

# Detection
../bin/Release/app_dl_inferer_test -d /opt/model_zoo/TFL-OD-2020-ssdLite-mobDet-DSP-coco-320x320/                   -i $EDGEAI_DATA_PATH/images/0002.jpg -v 0.6
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/ONR-OD-8420-yolox-s-lite-mmdet-widerface-640x640/              -i $EDGEAI_DATA_PATH/images/0002.jpg -v 0.6
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/ONR-OD-8220-yolox-s-lite-mmdet-coco-640x640/                   -i $EDGEAI_DATA_PATH/images/0002.jpg -v 0.6
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/ONR-OD-8050-ssd-lite-regNetX-800mf-fpn-bgr-mmdet-coco-512x512/ -i $EDGEAI_DATA_PATH/images/0002.jpg -v 0.6
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/TVM-OD-5120-ssdLite-mobDet-DSP-coco-320x320/                   -i $EDGEAI_DATA_PATH/images/0005.jpg -v 0.6


# Semantic Segmentation
../bin/Release/app_dl_inferer_test -d /opt/model_zoo/TFL-SS-2580-deeplabv3_mobv2-ade20k32-mlperf-512x512/   -i $EDGEAI_DATA_PATH/images/0016.jpg -a 0.4
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/ONR-SS-8610-deeplabv3lite-mobv2-ade20k32-512x512/      -i $EDGEAI_DATA_PATH/images/0001.jpg -a 0.4
#../bin/Release/app_dl_inferer_test -d /opt/model_zoo/TVM-SS-5710-deeplabv3lite-mobv2-cocoseg21-512x512/     -i $EDGEAI_DATA_PATH/images/0002.jpg -a 0.4

# Human pose estimation
../bin/Release/app_dl_inferer_test -d /opt/model_zoo/ONR-KD-7060-human-pose-yolox-s-640x640/ -i $EDGEAI_DATA_PATH/images/0002.jpg -v 0.6

mkdir -p ../app_dl_inferer_test_outputs
mv *_output*.jpg ../app_dl_inferer_test_outputs/
sync