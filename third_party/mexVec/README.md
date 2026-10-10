# mexVec

High-performance dynamic array with Small-Buffer Optimization (SBO). Header-only.

## Features
- Small Buffer Optimization (inline storage for small vectors)
- Optimized path for trivially copyable types (memcpy/memmove)
- Configurable growth factor
- Iterator + ranges-friendly API

## Requirements
- C++20 compiler
- CMake

## Build + test
```sh
mkdir -p build && cd build
cmake ..
cmake --build . -- -j
ctest --output-on-failure
```

## Benchmarks
This repo builds a benchmark binary at `build/tests/vec_benchmark`.

```sh
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target vec_benchmark -- -j
./tests/vec_benchmark --benchmark_repetitions=3 --benchmark_min_time=0.2
```

## Usage
```cpp
#include "Vec.hpp" // from include/Vec.hpp

int main()
{
    mex::Vec<int> v;
    v.push_back(1);
}
```

## License
See repository for license information.
