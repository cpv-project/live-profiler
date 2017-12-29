# Environement Setup

To use this library on windows, you need to install mingw from [here](https://sourceforge.net/projects/msys2).<br/>
After you installed, please open "mingw64.exe" (don't use "msys2.exe") and execute the following commands: 

``` bash
pacman -Syu # update pacman itself
pacman -Syu # update other packages
pacman -S mingw-w64-x86_64-gcc cmake make
```

To use this library, just include the header files, for example:

``` bash
g++ -Wall -Wextra --std=c++14 -O3 -g -Ilive-profiler/include Main.cpp
```

Notice: Some collector only works on linux, consider use ifdef when you're writing a cross platform profiler.

# Run tests

The commands for running tests are same as linux:

``` bash
cd tests
sh run_tests.sh
```
