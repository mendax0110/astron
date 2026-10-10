#!/bin/bash

set -e

echo "==================================================================="
echo "Code Coverage Generation for mex::Vec"
echo "==================================================================="

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PROJECT_DIR="${PROJECT_DIR:-$(pwd)}"
BUILD_DIR="${BUILD_DIR:-${PROJECT_DIR}/build_coverage}"

echo -e "${YELLOW}Project Directory:${NC} $PROJECT_DIR"
echo -e "${YELLOW}Build Directory:${NC} $BUILD_DIR"
echo ""

echo -e "${GREEN}Step 1: Setting up build directory${NC}"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo -e "${GREEN}Step 2: Configuring with coverage flags${NC}"
cmake "$PROJECT_DIR" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_FLAGS="-fprofile-instr-generate -fcoverage-mapping -O0 -g" \
    -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Step 3: Building tests${NC}"
make -j$(sysctl -n hw.ncpu) vec_test

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Step 4: Running tests to collect coverage data${NC}"

echo "  Running vec_test..."
cd tests
LLVM_PROFILE_FILE="vec_test.profraw" ./vec_test
TEST1_EXIT=$?

if [ $TEST1_EXIT -ne 0 ]; then
    echo -e "${RED}Tests failed!${NC}"
    echo "  vec_test exit code: $TEST1_EXIT"
fi

echo -e "${GREEN}Step 5: Merging coverage data${NC}"
xcrun llvm-profdata merge \
    vec_test.profraw \
    -o coverage.profdata

if [ $? -ne 0 ]; then
    echo -e "${RED}Failed to merge coverage data!${NC}"
    exit 1
fi

echo -e "${GREEN}Step 6: Generating coverage reports${NC}"

echo -e "${YELLOW}  Generating text report...${NC}"
xcrun llvm-cov report \
    vec_test \
    -instr-profile=coverage.profdata \
    -object vec_test \
    ../../include/Vec.hpp \
    > coverage_report.txt

echo -e "${YELLOW}  Generating HTML report...${NC}"
xcrun llvm-cov show \
    vec_test \
    -instr-profile=coverage.profdata \
    -object vec_test \
    ../../include/Vec.hpp \
    -format=html \
    -output-dir=coverage_html \
    -show-line-counts-or-regions \
    -show-instantiation-summary

echo -e "${GREEN}Step 7: Coverage Summary${NC}"
echo "========================================================================="
xcrun llvm-cov report \
    vec_test \
    -instr-profile=coverage.profdata \
    -object vec_test \
    ../../include/Vec.hpp

echo ""
echo "========================================================================="
echo -e "${GREEN}Coverage generation complete!${NC}"
echo ""
echo "Reports generated:"
echo "  - Text report:    $BUILD_DIR/tests/coverage_report.txt"
echo "  - HTML report:    $BUILD_DIR/tests/coverage_html/index.html"
echo ""
echo "To view HTML report:"
echo "  open $BUILD_DIR/tests/coverage_html/index.html"
echo ""
echo "Quick stats:"
grep -A 2 "TOTAL" "$BUILD_DIR/tests/coverage_report.txt" || echo "Could not extract stats"
echo "========================================================================="