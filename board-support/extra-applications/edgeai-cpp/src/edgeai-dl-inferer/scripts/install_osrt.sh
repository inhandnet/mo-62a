#!/bin/bash

#  Copyright (C) 2021 Texas Instruments Incorporated - http://www.ti.com/
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#
#    Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#    Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the
#    distribution.
#
#    Neither the name of Texas Instruments Incorporated nor the names of
#    its contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

REL=10_01_00_01
SCRIPTDIR=`pwd`
TARGET_FS_PATH=/
PYTHON_DIST=/usr/local/lib/python3.12/dist-packages

INSTALL_OPENCV=1
while test $# -gt 0; do
  case "$1" in
    --no-opencv*)
      INSTALL_OPENCV=0
      shift
      ;;
    *)
      break
      ;;
  esac
done

if [ `arch` == "aarch64" ]; then
    if grep -qi ubuntu /etc/os-release; then
        echo "Installing dependedcies at $TARGET_FS_PATH"

        cd $TARGET_FS_PATH/$HOME

        if [ ! -d required_libs ];then
            mkdir required_libs
        fi

        if [ ! -d arago_j7_pywhl ];then
            mkdir arago_j7_pywhl
        fi

        cd $TARGET_FS_PATH/$HOME/arago_j7_pywhl

        wget -q --no-proxy https://software-dl.ti.com/jacinto7/esd/tidl-tools/$REL/OSRT_TOOLS/ARM_LINUX/ARAGO/dlr-1.13.0-py3-none-any.whl
        wget -q --no-proxy https://software-dl.ti.com/jacinto7/esd/tidl-tools/$REL/OSRT_TOOLS/ARM_LINUX/ARAGO/onnxruntime_tidl-1.15.0-cp312-cp312-linux_aarch64.whl
        wget -q --no-proxy https://software-dl.ti.com/jacinto7/esd/tidl-tools/$REL/OSRT_TOOLS/ARM_LINUX/ARAGO/tflite_runtime-2.12.0-cp312-cp312-linux_aarch64.whl

        # ln -s /usr/bin/pip3 /usr/bin/pip3.10
        pip3 install --upgrade --force-reinstall dlr-1.13.0-py3-none-any.whl -t $PYTHON_DIST --disable-pip-version-check
        pip3 install onnxruntime_tidl-1.15.0-cp312-cp312-linux_aarch64.whl -t $PYTHON_DIST --disable-pip-version-check
        pip3 install --upgrade --force-reinstall tflite_runtime-2.12.0-cp312-cp312-linux_aarch64.whl -t $PYTHON_DIST --disable-pip-version-check
        pip3 install --upgrade --force-reinstall --no-cache-dir numpy==1.26.4 -t $PYTHON_DIST --disable-pip-version-check

        cd $TARGET_FS_PATH/$HOME
        rm -r $TARGET_FS_PATH/$HOME/arago_j7_pywhl

        if [  ! -d $TARGET_FS_PATH/usr/include/tensorflow ];then
            wget -q --no-proxy https://software-dl.ti.com/jacinto7/esd/tidl-tools/$REL/OSRT_TOOLS/ARM_LINUX/ARAGO/tflite_2.12_aragoj7.tar.gz
            tar xf tflite_2.12_aragoj7.tar.gz
            rm tflite_2.12_aragoj7.tar.gz
            mv tflite_2.12_aragoj7/tensorflow  $TARGET_FS_PATH/usr/include
            mv tflite_2.12_aragoj7/tflite_2.12  $TARGET_FS_PATH/usr/lib/
            cp tflite_2.12_aragoj7/libtensorflow-lite.a $TARGET_FS_PATH/usr/lib/
            rm -r tflite_2.12_aragoj7
            cd $TARGET_FS_PATH/$HOME
        else
            echo "skipping tensorflow setup: found /usr/include/tensorflow"
            echo "To redo the setup delete: /usr/include/tensorflow and run this script again"
        fi

        if [  ! -d $TARGET_FS_PATH/usr/include/onnxruntime ];then
            wget -q --no-proxy https://software-dl.ti.com/jacinto7/esd/tidl-tools/$REL/OSRT_TOOLS/ARM_LINUX/ARAGO/onnx_1.15.0_aragoj7.tar.gz
            tar xf onnx_1.15.0_aragoj7.tar.gz
            rm onnx_1.15.0_aragoj7.tar.gz
            cp -r  onnx_1.15.0_aragoj7/libonnxruntime.so*   $TARGET_FS_PATH/usr/lib/
            cd   $TARGET_FS_PATH/usr/lib/
            ln -s libonnxruntime.so.1.15.0* libonnxruntime.so
            cd  $TARGET_FS_PATH/$HOME
            mv onnx_1.15.0_aragoj7/onnxruntime $TARGET_FS_PATH/usr/include/
            rm -r onnx_1.15.0_aragoj7
            cd  $TARGET_FS_PATH/$HOME
        else
            echo "skipping onnxruntime setup: found /usr/include/onnxruntime"
            echo "To redo the setup delete: /usr/include/onnxruntime and run this script again"
        fi

        if [  ! -f  $TARGET_FS_PATH/usr/include/itidl_rt.h ];then
            git clone -b master git://git.ti.com/processor-sdk-vision/arm-tidl.git
            cp arm-tidl/rt/inc/itidl_rt.h  $TARGET_FS_PATH/usr/include/
            cp arm-tidl/rt/inc/itvm_rt.h $TARGET_FS_PATH/usr/include/
            rm -r arm-tidl
            cd $TARGET_FS_PATH/$HOME/
        else
            echo "skipping itidl_rt.h setup: found /usr/include/itidl_rt.h"
            echo "To redo the setup delete: /usr/include/itidl_rt.h and run this script again"
        fi

        if [  ! -f  $TARGET_FS_PATH/usr/dlr/libdlr.so ];then
            mkdir   $TARGET_FS_PATH/usr/dlr/
            cd  $TARGET_FS_PATH/usr/dlr/
            ln -s -r  $TARGET_FS_PATH/$PYTHON_DIST/dlr/libdlr.so libdlr.so
            cd  $TARGET_FS_PATH/$HOME
        fi

        if [  ! -f  $TARGET_FS_PATH/usr/lib/libdlr.so ];then
            cd  $TARGET_FS_PATH/usr/lib/
            ln -s -r $TARGET_FS_PATH/usr/dlr/libdlr.so libdlr.so
            cd  $TARGET_FS_PATH/$HOME
        fi

        if [  ! -f  $TARGET_FS_PATH/usr/include/neo-ai-dlr ];then
            mkdir  $TARGET_FS_PATH/usr/include/neo-ai-dlr
            ln -s -r $TARGET_FS_PATH/$PYTHON_DIST/dlr/include /usr/include/neo-ai-dlr/include
            cd  $TARGET_FS_PATH/$HOME
        fi

        if [ 1 == $INSTALL_OPENCV ]; then
            if [  ! -d $TARGET_FS_PATH/usr/include/opencv-4.2.0 ];then
                wget -q --no-proxy https://software-dl.ti.com/jacinto7/esd/tidl-tools/$REL/OSRT_TOOLS/ARM_LINUX/ARAGO/opencv_4.2.0_aragoj7.tar.gz
                tar -xf opencv_4.2.0_aragoj7.tar.gz
                rm opencv_4.2.0_aragoj7.tar.gz
                cp -r opencv_4.2.0_aragoj7/opencv $TARGET_FS_PATH/usr/lib/
                mv opencv_4.2.0_aragoj7/opencv-4.2.0 $TARGET_FS_PATH/usr/include/
                cd $TARGET_FS_PATH/$HOME
                rm -r opencv_4.2.0_aragoj7
            else
                echo "skipping opencv-4.2.0 setup: found /usr/include/opencv-4.2.0"
                echo "To redo the setup delete: /usr/include/opencv-4.2.0 and run this script again"
            fi
        fi

        #Cleanup
        cd $TARGET_FS_PATH/$HOME/
        rm -rf required_libs
        rm -rf tidl_tools

        cd $SCRIPTDIR
    fi
fi
