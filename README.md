# top-like
A short cxx project inspired by fasterthanlime's axact

## Description

This project is essentially a C++ version of [fasterthanlime/axact](https://github.com/fasterthanlime/axact). I did this to help refamiliarize myself with C++, so _Caveat Utilitor_.
If for some reason you want to build this, then you can use the following commands 
```
conan install . --output-folder=build --build=missing
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=build -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake --build .
```
If you don't care to use [Conan](https://conan.io/) (perhaps you're in a future where C++ has a standard package manager) then the dependencies are just `boost` version 1.81.0, `CMake`, and C++17.

