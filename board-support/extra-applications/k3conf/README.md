K3CONF
======
A Powerful Diagnostic Tool for Texas Instruments K3 based Processors


INTRODUCTION:
-------------

K3CONF is a Linux user-space standalone application designed to provide a
quick'n easy way to dynamically diagnose Texas Instruments' K3 architecture
based processors. K3CONF is intended to provide similar experience to that of
OMAPCONF that runs on legacy TI platforms.

**WARNING**: This is work in progress! Don't expect things to be complete in any
dimension. Use at your own risk. And keep the reset button in reach.


SUPPORT:
--------

K3CONF currently supports AM654, J721E, J7200, AM64x, AM62x, J721S2, J784S4,
J722S, AM62Ax, AM62Px, and AM62Lx devices. AM62Lx platform support is
experimental. Legacy OMAP and DRA7 platforms are not supported.

This tool makes usage of /dev/mem. If your kernel doesn't have CONFIG_DEVMEM,
or enables CONFIG_DEVMEM_STRICT_IO, it will not work well.

Support for the AM62Lx makes use of the SCMI Raw Interface. This tool makes
usage of /sys/kernel/debug/scmi/0. If your kernel doesn't have
CONFIG_ARM_SCMI_NEED_DEBUGFS, CONFIG_ARM_SCMI_RAW_MODE_SUPPORT, and
CONFIG_ARM_SCMI_RAW_MODE_SUPPORT_COEX, it will not have full functionality.

Build Instructions:
-------------------

* Install build dependencies (Debian based example):

        # sudo apt install build-essential cmake

* If cross-compiling, install and set your cross-compiler:

        # sudo apt install gcc-aarch64-linux-gnu
        # export CC=aarch64-linux-gnu-gcc

* To build the output binary file run the following commands:

        # mkdir build
        # cd build
        # cmake ..
        # make

* Copy the output binary `k3conf` to your Filesystem. That's it!!


Build Instructions for static linked binary:
--------------------------------------------

* The default k3conf binary generated is with shared library, alternatively to
  build a static linked binary replace the `cmake` command in build
  instructions with below command.

        # cmake .. -Dstatic_exe=ON


Help:
-----

* Type `./k3conf --help` to get complete list of available commands and options.  
Note that in case of incorrect command/option, help will also be displayed.
