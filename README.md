### Build Steps
The library uses c++ 20 features .So you need a compatible compiler

Use following commands to build
```
mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=path to compiler  ..
make
```
eg: 
```
cmake -DCMAKE_CXX_COMPILER=/opt/homebrew/Cellar/gcc/13.1.0/bin/g++-13 ..
````
