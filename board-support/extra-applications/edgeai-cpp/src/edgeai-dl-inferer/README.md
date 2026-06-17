# edgeai-dl-inferer - A Linux based CPP library for abstracting Open Source Runtime (OSRT) API

This document walks through what is contained in The edgeai-dl-inferer library
package, how to build, install, and run the provided sample applications,
and the library API.

# Installation

## Dependencies
This library has a header file dependency on the TI OSRT library and the following should have been installed
prior to compiling this library. The associated libraries are needed for successfully linking all examples.

- onnxruntime
- tensorflow
- neo-ai-dlr

## Compilation on target

Clone this repository, build it, and install it.
```
git clone https://github.com/TexasInstruments/edgeai-dl-inferer
cd edgeai-dl-inferer
mkdir build && cd build
cmake ..
make
make install
```
The above installs the library and header files under /usr dirctory. The headers and library will be placed as follows

- **Headers**: /usr/**include/edgeai_dl_inferer**/
- **Library**: /usr/**lib**/

The desired install location can be specified as follows

```
cmake -DCMAKE_INSTALL_PREFIX=<path/to/install> ..
make -j2
make install
```

- **Headers**: path/to/install/**include/edgeai_dl_inferer**/
- **Library**: path/to/install/**lib**

## Cross-Compilation for the target

The library can be cross-compiled on an x86_64 machine for the target. Here are the steps for cross-compilation.
```
git clone https://github.com/TexasInstruments/edgeai-dl-inferer
cd edgeai-dl-inferer
# Update cmake/setup_cross_compile.sh to specify tool paths and settings
mkdir build
cd build
source ../cmake/setup_cross_compile.sh
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/cross_compile_aarch64.cmake ..
make -j2
make install DESTDIR=<path/to/targetfs>
```
The library and headers will be placed under the following directory structire. Here we use the default
'/usr' install prefix and we prepend the tarfetfs directory location

- **Headers**: path/to/targetfs/usr/**include/edgeai_dl_inferer**/
- **Library**: path/to/targetfs/usr/**lib**/

# Documentation

