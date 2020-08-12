# thread

Extend STL

## Build

For Mac and Linux:

```bash
git clone https://github.com/xbigo/thread.git
mkdir build
cd build
cmake -S ../thread -D BUILD_TESTING=1
make
make check
make install
```

For MSVC:

```bat
git clone https://github.com/xbigo/thread.git
mkdir build
cd build
cmake -S ../thread -D BUILD_TESTING=1 -G "Visual Studio 16 2019"
cmake --build .
cmake --build . --target check
cmake --build . --target install
```

