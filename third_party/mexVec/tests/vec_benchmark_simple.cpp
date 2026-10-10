#include "../include/Vec.hpp"
#include <benchmark/benchmark.h>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <numeric>

static void BM_MexVec_PushBack(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        mex::Vec<int> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
            benchmark::DoNotOptimize(v.data());
        }
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MexVec_PushBack)->Range(8, 8<<10);

static void BM_StdVector_PushBack(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        std::vector<int> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
            benchmark::DoNotOptimize(v.data());
        }
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StdVector_PushBack)->Range(8, 8<<10);

static void BM_MexVec_PushBack_Reserved(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        mex::Vec<int> v;
        v.reserve(state.range(0));
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MexVec_PushBack_Reserved)->Range(64, 8<<10);

static void BM_StdVector_PushBack_Reserved(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        std::vector<int> v;
        v.reserve(state.range(0));
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StdVector_PushBack_Reserved)->Range(64, 8<<10);

static void BM_MexVec_RandomAccess(benchmark::State& state)
{
    mex::Vec<int> v(state.range(0));
    std::iota(v.begin(), v.end(), 0);

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist(0, v.size() - 1);

    for (auto _ : state)
    {
        size_t idx = dist(rng);
        benchmark::DoNotOptimize(v[idx]);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_MexVec_RandomAccess)->Range(64, 16<<10);

static void BM_StdVector_RandomAccess(benchmark::State& state)
{
    std::vector<int> v(state.range(0));
    std::iota(v.begin(), v.end(), 0);

    std::mt19937 rng(42);
    std::uniform_int_distribution<size_t> dist(0, v.size() - 1);

    for ([[maybe_unused]] auto _ : state)
    {
        size_t idx = dist(rng);
        benchmark::DoNotOptimize(v[idx]);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_StdVector_RandomAccess)->Range(64, 16<<10);

static void BM_MexVec_SequentialAccess(benchmark::State& state)
{
    mex::Vec<int> v(state.range(0));
    std::iota(v.begin(), v.end(), 0);

    for ([[maybe_unused]] auto _ : state)
    {
        long long sum = 0;
        for (int i : v)
        {
            sum += i;
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MexVec_SequentialAccess)->Range(64, 16<<10);

static void BM_StdVector_SequentialAccess(benchmark::State& state)
{
    std::vector<int> v(state.range(0));
    std::iota(v.begin(), v.end(), 0);

    for ([[maybe_unused]] auto _ : state)
    {
        long long sum = 0;
        for (int i : v)
        {
            sum += i;
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StdVector_SequentialAccess)->Range(64, 16<<10);

static void BM_MexVec_InsertMiddle(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        mex::Vec<int> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
        }
        state.ResumeTiming();

        v.insert(v.begin() + v.size() / 2, 999);
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_MexVec_InsertMiddle)->Range(8, 1<<10);

static void BM_StdVector_InsertMiddle(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<int> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
        }
        state.ResumeTiming();

        v.insert(v.begin() + v.size() / 2, 999);
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_StdVector_InsertMiddle)->Range(8, 1<<10);

static void BM_MexVec_EraseMiddle(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        mex::Vec<int> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
        }
        state.ResumeTiming();

        v.erase(v.begin() + v.size() / 2);
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_MexVec_EraseMiddle)->Range(8, 1<<10);

static void BM_StdVector_EraseMiddle(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<int> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(i);
        }
        state.ResumeTiming();

        v.erase(v.begin() + v.size() / 2);
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_StdVector_EraseMiddle)->Range(8, 1<<10);

static void BM_MexVec_Copy(benchmark::State& state)
{
    mex::Vec<int> source(state.range(0));
    std::iota(source.begin(), source.end(), 0);

    for ([[maybe_unused]] auto _ : state)
    {
        mex::Vec<int> copy(source);
        benchmark::DoNotOptimize(copy.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MexVec_Copy)->Range(8, 8<<10);

static void BM_StdVector_Copy(benchmark::State& state)
{
    std::vector<int> source(state.range(0));
    std::iota(source.begin(), source.end(), 0);

    for ([[maybe_unused]] auto _ : state)
    {
        std::vector<int> copy(source);
        benchmark::DoNotOptimize(copy.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_StdVector_Copy)->Range(8, 8<<10);

static void BM_MexVec_SmallObjects(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        mex::Vec<int> v;
        for (int i = 0; i < 8; ++i)  // Within SBO
        {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * 8);
}
BENCHMARK(BM_MexVec_SmallObjects);

static void BM_StdVector_SmallObjects(benchmark::State& state)
{
    for ([[maybe_unused]] auto _ : state)
    {
        std::vector<int> v;
        for (int i = 0; i < 8; ++i)
        {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(state.iterations() * 8);
}
BENCHMARK(BM_StdVector_SmallObjects);

static void BM_MexVec_StringPushBack(benchmark::State& state)
{
    std::vector<std::string> strings;
    for (int i = 0; i < 100; ++i)
    {
        strings.push_back("String_" + std::to_string(i));
    }

    for ([[maybe_unused]] auto _ : state)
    {
        mex::Vec<std::string> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(strings[i % strings.size()]);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_MexVec_StringPushBack)->Range(8, 512);

static void BM_StdVector_StringPushBack(benchmark::State& state)
{
    std::vector<std::string> strings;
    for (int i = 0; i < 100; ++i)
    {
        strings.push_back("String_" + std::to_string(i));
    }

    for ([[maybe_unused]] auto _ : state)
    {
        std::vector<std::string> v;
        for (int i = 0; i < state.range(0); ++i)
        {
            v.push_back(strings[i % strings.size()]);
        }
        benchmark::DoNotOptimize(v.data());
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_StdVector_StringPushBack)->Range(8, 512);

static void BM_MexVec_Sort(benchmark::State& state)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 10000);

    for (auto _ : state)
    {
        state.PauseTiming();
        mex::Vec<int> v(state.range(0));
        for (auto& x : v)
        {
            x = dist(rng);
        }
        state.ResumeTiming();

        std::sort(v.begin(), v.end());
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_MexVec_Sort)->Range(64, 4<<10);

static void BM_StdVector_Sort(benchmark::State& state)
{
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, 10000);

    for ([[maybe_unused]] auto _ : state)
    {
        state.PauseTiming();
        std::vector<int> v(state.range(0));
        for (auto& x : v)
        {
            x = dist(rng);
        }
        state.ResumeTiming();

        std::sort(v.begin(), v.end());
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_StdVector_Sort)->Range(64, 4<<10);

BENCHMARK_MAIN();