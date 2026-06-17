#  Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
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

from time import time as _time
from time import sleep as _sleep
import numpy as _np
import threading as _threading
import curses as _curses
import cv2 as _cv2
import yaml as _yaml

print_stdout = True
print_curses = False
_proctime = {}

'''
Helpers functions to print average processing time measurements
'''

def report_proctime(tag, value):
    """
    Used for reporting the processing time values
    All the values with same tag are automatically averaged
    This information is used when printing the ncurses table

    Args:
        tag (string): unique tag to indicate specific processing entity
        value (float): Current measured processing time in microseconds
    """
    global _proctime
    global print_curses
    try:
        data = _proctime[tag]
    except KeyError:
        _proctime[tag] = (0.0, 0)
    finally:
        avg, n = _proctime[tag]
        avg = (avg * n + value) / (n + 1)
        n = n + 1
        _proctime[tag] = (avg, n)
        if (print_stdout):
            print("[UTILS] Time for '%s': %5.2f ms (avg %5.2f ms)" % (tag, value * 1000, avg * 1000))

def reporting_loop():
    """
    Called from a new thread which periodically prints all the processing
    times gathered by call to report_proctime()
    It uses ncurses to print a nice looking table showcasing current value,
    average value and total samples measured, etc
    """
    global _proctime
    stdscr = _curses.initscr()
    _curses.noecho()
    _curses.cbreak()
    stdscr.keypad(True)

    while (True):
        stdscr.clear()
        stdscr.addstr(1, 1, "+%s+" % ('-' * 60))
        stdscr.addstr(2, 1, "| {:<59s}|".format("Average Processing time"))
        stdscr.addstr(3, 1, "+%s+" % ('-' * 60))
        i = 4
        for tag in _proctime.keys():
            (avg, n) = _proctime[tag]
            avg = avg * 1000
            stdscr.addstr(i, 1, "| {:<29s}:".format(tag))
            stdscr.addstr(i, 34, "{:>8.2f} ms".format(avg), _curses.A_BOLD)
            stdscr.addstr(i, 42, " from {:^5d} samples |".format(n))
            i = i + 1
        stdscr.addstr(i, 1, "+%s+" % ('-' * 60))
        stdscr.refresh()
        _sleep(1)

    # Cleanup before existing
    _curses.nocbreak()
    stdscr.keypad(False)
    _curses.echo()
    _curses.endwin()
    sys.exit(1)

def enable_curses_reports():
    """
    By default, all the processing times are reported on stdout with a single
    print statement. Calling this will start a new thread which uses ncurses
    to display a table with processing times measured for all the tags and
    keeps the table updated periodically.
    This is useful for visualizing the performance of the demo.
    """
    global print_curses
    global print_stdout
    print_stdout = False
    print_curses = True
    thread_report = _threading.Thread(target = reporting_loop)
    thread_report.start()

def RGB2YUV(rgb):
    """
    Simple non accelerated based RGB to YUV conversion routine.

    Args:
        rgb (numpy array): input image which has R,G,B as 3 separate channels 

    Returns:
        numpy array: it has Y,U,V as 3 separate channnels
    """
    m = _np.array([[ 0.29900, -0.16874,  0.50000],
                 [0.58700, -0.33126, -0.41869],
                 [ 0.11400, 0.50000, -0.08131]])
    yuv = _np.dot(rgb,m)
    yuv[:,:, 1:] += 128.0
    rgb = _np.clip(yuv, 0.0, 255.0)
    return yuv

def YUV2RGB( yuv ):
    """
    Simple non accelerated based YUV to RGB conversion routine.

    Args:
        yuv (numpy array): input image which has Y,U,V as 3 separate channels

    Returns:
        numpy array: it has R,G,B as 3 separate channnels
    """
    m = _np.array([[ 1.0, 1.0, 1.0],
                 [-0.000007154783816076815, -0.3441331386566162, 2.0320025777816772],
                 [ 1.14019975662231445, -0.5811380310058594 , 0.00001542569043522235] ])
    yuv[:,:, 1:] -= 128.0
    rgb = _np.dot(yuv,m)
    rgb = _np.clip(rgb, 0.0, 255.0)

    return rgb

def resize(img, new_width, new_height):
    """
    Resize the image to given dimensions and returns the new image.
    Uses openCV with cubic interpolation for scaling.
    """
    start = _time()
    img = _cv2.resize(img, (new_width, new_height), interpolation=_cv2.INTER_CUBIC)
    end = _time()

    report_proctime('cv2_resize', (end - start))
    return img

def resize_smaller_dim(img, dim):
    """
    Resize the image by keeping the same aspect ratio.
    Smaller dimension is resized to given value and the other
    dimension is resized in the same ratio.
    e.g. if X is an image of size 1280x720, resize_smaller_dim(512)
    will return an image with size of 910x512

    Args:
        img (numpy array): Input image to be resized
        dim (int): The smaller dimension will be scaled to this value

    Returns:
        numpy array: Resized image
    """
    orig_width, orig_height, _ = img.shape
    new_height = orig_height * dim // min(img.shape[:2])
    new_width = orig_width * dim // min(img.shape[:2])

    return resize(img, new_width, new_height)

def resize_bigger_dim(img, dim):
    """
    Resize the image by keeping the same aspect ratio.
    Bigger dimension is resized to given value and the other
    dimension is resized in the same ratio.
    e.g. if X is an image of size 1280x720, resize_smaller_dim(512)
    will return an image with size of 512x288

    Args:
        img (numpy array): Input image to be resized
        dim (int): The bigger dimension will be scaled to this value

    Returns:
        numpy array: Resized image
    """
    orig_width, orig_height, _ = img.shape
    new_height = orig_height * dim // max(img.shape[:2])
    new_width = orig_width * dim // max(img.shape[:2])

    return resize(img, new_width, new_height)

def pad_border(img, dim=0):
    """
    Change the image into a square image with zero padding on the border.
    Final image will be dim x dim.

    Args:
        img (numpy array): Input image to be padded
        dim (side of the square): If dim == 0 then bigger dimension is picked

    Returns:
        numpy array: Zero padded image
    """
    start = _time()
    if (dim == 0):
        dim = max(img.shape[:2])
    vpad = dim - img.shape[0]
    hpad = dim - img.shape[1]
    top = vpad // 2
    bottom = vpad - top
    left = hpad // 2
    right = hpad - left

    img = _cv2.copyMakeBorder(img, top, bottom, left, right, _cv2.BORDER_CONSTANT, 0)
    end = _time()
    report_proctime('cv2_pad_border', (end - start))
    return img, (top, bottom, left, right)

def centre_crop(img, width, height):
    """
    Crop the image to given size.
    Uses numpy array slicing to avhieve the cropping
    """
    start = _time()
    orig_height, orig_width, _ = img.shape
    startx = orig_width // 2 - (width // 2)
    starty = orig_height // 2 - (height // 2)

    img = img[starty : starty + height, startx : startx + width]
    end = _time()
    report_proctime('centre_crop', (end - start))
    return img

def change_format(img, fin, fout):
    """
    Change the packing format of the image to specified one.
    Uses numpy transpose to change formats and add new dimensions.
    Supported formats:
        HWC => Height, Width, Channel
        NCHW => Number of frames, Channels, Height, Width
        NHWC => Number of frames, Height, Width, Channels

    Args:
        img (numpy array): Input image
        fin (string): input format
        fout (string): output format

    Returns:
        [numpy array]: Final image with given output format
            None if the formats are not supported
    """
    if (fin == 'HWC' and fout == 'NCHW'):
        transpose = (2,0,1)
        newdim = 0
    elif (fin == 'HWC' and fout == 'NHWC'):
        transpose = 0
        newdim = 0
    elif (fin == 'NCHW' and fout == 'NHWC'):
        transpose = (0, 2, 3, 1)
    else:
        print("[ERROR] Unsupported format conversion from %s to %s" % (fin, fout))
        return None

    start = _time()
    if (transpose):
        img = _np.transpose(img, transpose)
    if (newdim != None):
        img = _np.expand_dims(img, axis=newdim)
    img = img.astype(_np.float32)

    end = _time()
    report_proctime('packing_format_change', (end - start))
    return img

def channel_swap_bgr_to_rgb(hwc_img):
    """
    Swap the color channel ordering in the image.
    to convert from Blue, Green, Red order to Red, Green, Blue
    """
    start = _time()
    hwc_img = hwc_img[:,:,::-1]
    end  = _time()
    report_proctime('color_channel_swap', (end - start))
    return hwc_img

def subtract_mean_and_scale(img, mean, scale, chan_axis):
    """
    Perform mean subtraction for all channels.
    chan_axis decides the dimension to use for channels.
    e.g. NCHW has chan_axis = 1, HWC has chan_axis = 2

    Args:
        img (numpy array): Input image
        mean (list): List of mean values for all channels
        scale (list): List of scale values for all channels
        chan_axis (int): The numpy axis to be used for channels

    Returns:
        numpy array: Output image after all the mean scale conversion
    """
    start = _time()
    for mean, scale, ch in zip(mean, scale, range(img.shape[chan_axis])):
        if (chan_axis == 0):
            img[ch,...] = ((img[ch,...] - mean) * scale)
        elif (chan_axis == 1):
            img[:,ch,...] = ((img[:,ch,...] - mean) * scale)
        elif (chan_axis == 2):
            img[:,:,ch,...] = ((img[:,:,ch,...] - mean) * scale)
        elif (chan_axis == 3):
            img[:,:,:,ch] = ((img[:,:,:,ch] - mean) * scale)
        else:
            print("[ERROR] Unsupported channel axis")
    end  = _time()
    report_proctime('subtract_mean_and_scale', (end - start))
    return img

def load_labels(label_path):
    """
    Dictionary of ID to class name mapping
    """
    labels = {}
    with open(label_path, "r") as dataset_yaml:
        dataset = _yaml.safe_load(dataset_yaml)
    for category in dataset['categories']:
        labels[category['id']] = category['name']
    return labels

def print_test_banner(model_config, imgs_path):
    """
    Prints test case desciption
    """
    print(f"model_path = {model_config.model_path}")
    print(f"test_images = {imgs_path}")
    print(f"run_time = {model_config.run_time}")

    if model_config.task_type == 'classification':
        print(f"topN = {model_config.topN}")
    if model_config.task_type == 'detection':
        print(f"viz_threshold = {model_config.viz_threshold}")
    if model_config.task_type == 'segmentation':
        print(f"alpha = {model_config.alpha}")
