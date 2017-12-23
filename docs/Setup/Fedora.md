# Environement Setup

Setup environment on fedora is very easy, execute this command and all done:

``` bash
su -c "dnf install gcc-c++ cmake binutils-devel"
```

To use this library, just include the header files, for example:

``` bash
g++ -Wall -Wextra --std=c++14 -O3 -g -Ilive-profiler/include Main.cpp -lbfd
```

# Run tests

Run tests is easy too:

``` bash
cd tests
sh run_tests.sh
```
