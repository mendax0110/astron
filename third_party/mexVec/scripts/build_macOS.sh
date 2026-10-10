rm -rf build && mkdir build && cd build
CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)
make test
make run_benchmarks