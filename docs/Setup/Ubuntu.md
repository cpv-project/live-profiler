# Environement Setup

Setup environment on ubuntu is very easy, execute this command and all done:

``` bash
sudo apt-get install g++ cmake binutils-dev
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
