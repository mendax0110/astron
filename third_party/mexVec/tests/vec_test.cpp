#include "../include/Vec.hpp"
#include <gtest/gtest.h>
#include <ranges>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <memory>

class VecTest : public ::testing::Test
{
protected:
    mex::Vec<int> vec_int;
    mex::Vec<std::string> vec_str;
};

TEST_F(VecTest, DefaultConstructor)
{
    EXPECT_TRUE(vec_int.empty());
    EXPECT_EQ(vec_int.size(), 0);
    EXPECT_GE(vec_int.capacity(), 8);
    EXPECT_TRUE(vec_int.is_small());
}

TEST_F(VecTest, CountConstructor)
{
    mex::Vec<int> v(10);
    EXPECT_EQ(v.size(), 10);
    EXPECT_GE(v.capacity(), 10);

    for (int i : v)
    {
        EXPECT_EQ(i, 0);
    }
}

TEST_F(VecTest, CountValueConstructor)
{
    mex::Vec<int> v(5, 42);
    EXPECT_EQ(v.size(), 5);

    for (int i : v)
    {
        EXPECT_EQ(i, 42);
    }
}

TEST_F(VecTest, RangeConstructor)
{
    std::vector<int> std_vec = {1, 2, 3, 4, 5};
    mex::Vec<int> v(std_vec.begin(), std_vec.end());

    EXPECT_EQ(v.size(), 5);
    for (size_t i = 0; i < v.size(); ++i)
    {
        EXPECT_EQ(v[i], std_vec[i]);
    }
}

TEST_F(VecTest, CopyConstructor)
{
    mex::Vec<int> v1 = {1, 2, 3, 4, 5};
    mex::Vec<int> v2(v1);

    EXPECT_EQ(v1.size(), v2.size());
    for (size_t i = 0; i < v1.size(); ++i)
    {
        EXPECT_EQ(v1[i], v2[i]);
    }

    // Modify v2, ensure v1 unchanged
    v2[0] = 999;
    EXPECT_EQ(v1[0], 1);
    EXPECT_EQ(v2[0], 999);
}

TEST_F(VecTest, MoveConstructor)
{
    mex::Vec<int> v1 = {1, 2, 3, 4, 5};
    auto* old_data = v1.data();
    size_t old_size = v1.size();

    mex::Vec<int> v2(std::move(v1));

    EXPECT_EQ(v2.size(), old_size);
    if (old_size > 8)
    {
        EXPECT_EQ(v2.data(), old_data);
    }
    EXPECT_TRUE(v1.empty());
}

TEST_F(VecTest, InitializerListConstructor)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    EXPECT_EQ(v.size(), 5);
    for (size_t i = 0; i < v.size(); ++i)
    {
        EXPECT_EQ(v[i], static_cast<int>(i + 1));
    }
}

TEST_F(VecTest, CopyAssignment)
{
    mex::Vec<int> v1 = {1, 2, 3};
    mex::Vec<int> v2;

    v2 = v1;

    EXPECT_EQ(v1.size(), v2.size());
    for (size_t i = 0; i < v1.size(); ++i)
    {
        EXPECT_EQ(v1[i], v2[i]);
    }

    v2[0] = 999;
    EXPECT_EQ(v1[0], 1);
}

TEST_F(VecTest, MoveAssignment)
{
    mex::Vec<int> v1 = {1, 2, 3, 4, 5};
    mex::Vec<int> v2;

    size_t old_size = v1.size();
    v2 = std::move(v1);

    EXPECT_EQ(v2.size(), old_size);
    EXPECT_TRUE(v1.empty());
}

TEST_F(VecTest, InitializerListAssignment)
{
    mex::Vec<int> v;
    v = {10, 20, 30};

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);
}

TEST_F(VecTest, AssignCount)
{
    mex::Vec<int> v;
    v.assign(5, 42);

    EXPECT_EQ(v.size(), 5);
    for (int i : v)
    {
        EXPECT_EQ(i, 42);
    }
}

TEST_F(VecTest, AssignRange)
{
    std::vector<int> std_vec = {1, 2, 3, 4, 5};
    mex::Vec<int> v;

    v.assign(std_vec.begin(), std_vec.end());

    EXPECT_EQ(v.size(), 5);
    for (size_t i = 0; i < v.size(); ++i)
    {
        EXPECT_EQ(v[i], std_vec[i]);
    }
}

TEST_F(VecTest, At)
{
    mex::Vec<int> v = {1, 2, 3};

    EXPECT_EQ(v.at(0), 1);
    EXPECT_EQ(v.at(1), 2);
    EXPECT_EQ(v.at(2), 3);

    EXPECT_THROW(v.at(3), std::out_of_range);
    EXPECT_THROW(v.at(100), std::out_of_range);
}

TEST_F(VecTest, OperatorBracket)
{
    mex::Vec<int> v = {10, 20, 30};

    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[1], 20);
    EXPECT_EQ(v[2], 30);

    v[1] = 99;
    EXPECT_EQ(v[1], 99);
}

TEST_F(VecTest, FrontBack)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    EXPECT_EQ(v.front(), 1);
    EXPECT_EQ(v.back(), 5);

    v.front() = 10;
    v.back() = 50;

    EXPECT_EQ(v.front(), 10);
    EXPECT_EQ(v.back(), 50);
}

TEST_F(VecTest, Data)
{
    mex::Vec<int> v = {1, 2, 3};

    int* ptr = v.data();
    EXPECT_EQ(ptr[0], 1);
    EXPECT_EQ(ptr[1], 2);
    EXPECT_EQ(ptr[2], 3);

    ptr[1] = 99;
    EXPECT_EQ(v[1], 99);
}

TEST_F(VecTest, Iterators)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    int sum = 0;
    for (int & it : v)
    {
        sum += it;
    }
    EXPECT_EQ(sum, 15);

    sum = 0;
    for (int x : v)
    {
        sum += x;
    }
    EXPECT_EQ(sum, 15);
}

TEST_F(VecTest, ReverseIterators)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    std::vector<int> reversed;
    for (int & it : std::ranges::reverse_view(v))
    {
        reversed.push_back(it);
    }

    EXPECT_EQ(reversed.size(), 5);
    EXPECT_EQ(reversed[0], 5);
    EXPECT_EQ(reversed[4], 1);
}

TEST_F(VecTest, ConstIterators)
{
    const mex::Vec<int> v = {1, 2, 3};

    int sum = 0;
    for (int it : v)
    {
        sum += it;
    }
    EXPECT_EQ(sum, 6);
}

TEST_F(VecTest, Empty)
{
    mex::Vec<int> v;
    EXPECT_TRUE(v.empty());

    v.push_back(1);
    EXPECT_FALSE(v.empty());

    v.pop_back();
    EXPECT_TRUE(v.empty());
}

TEST_F(VecTest, Size)
{
    mex::Vec<int> v;
    EXPECT_EQ(v.size(), 0);

    v.push_back(1);
    EXPECT_EQ(v.size(), 1);

    v.push_back(2);
    v.push_back(3);
    EXPECT_EQ(v.size(), 3);

    v.pop_back();
    EXPECT_EQ(v.size(), 2);
}

TEST_F(VecTest, Reserve)
{
    mex::Vec<int> v;

    v.reserve(100);
    EXPECT_GE(v.capacity(), 100);
    EXPECT_EQ(v.size(), 0);

    for (int i = 0; i < 100; ++i)
    {
        v.push_back(i);
    }
    EXPECT_EQ(v.size(), 100);
}

TEST_F(VecTest, ShrinkToFit)
{
    mex::Vec<int> v;
    v.reserve(100);

    for (int i = 0; i < 5; ++i)
    {
    v.push_back(i);
    }

    EXPECT_GE(v.capacity(), 100);
    v.shrink_to_fit();
    EXPECT_LE(v.capacity(), 10);
    EXPECT_TRUE(v.is_small());
}

TEST_F(VecTest, Clear)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    auto old_capacity = v.capacity();
    v.clear();

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
    EXPECT_EQ(v.capacity(), old_capacity);
}

TEST_F(VecTest, InsertSingle)
{
    mex::Vec<int> v = {1, 2, 4, 5};

    auto it = v.insert(v.begin() + 2, 3);

    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(*it, 3);
    EXPECT_EQ(v[2], 3);

    for (size_t i = 0; i < v.size(); ++i)
    {
        EXPECT_EQ(v[i], static_cast<int>(i + 1));
    }
}

TEST_F(VecTest, InsertCount)
{
    mex::Vec<int> v = {1, 2, 5};

    v.insert(v.begin() + 2, 2, 3);

    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 3);
}

TEST_F(VecTest, InsertRange)
{
    mex::Vec<int> v = {1, 2, 6, 7};
    std::vector<int> to_insert = {3, 4, 5};

    v.insert(v.begin() + 2, to_insert.begin(), to_insert.end());

    EXPECT_EQ(v.size(), 7);
    for (size_t i = 0; i < v.size(); ++i)
    {
        EXPECT_EQ(v[i], static_cast<int>(i + 1));
    }
}

TEST_F(VecTest, Emplace)
{
    mex::Vec<std::string> v = {"Hello", "World"};

    v.emplace(v.begin() + 1, 5, 'x');  // "xxxxx"

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[1], "xxxxx");
}

TEST_F(VecTest, EraseSingle)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    auto it = v.erase(v.begin() + 2);

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(*it, 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 4);
    EXPECT_EQ(v[3], 5);
}

TEST_F(VecTest, EraseRange)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5, 6};

    auto it = v.erase(v.begin() + 2, v.begin() + 5);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(*it, 6);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 6);
}

TEST_F(VecTest, PushBack)
{
    mex::Vec<int> v;

    for (int i = 0; i < 100; ++i)
    {
        v.push_back(i);
    }

    EXPECT_EQ(v.size(), 100);
    for (int i = 0; i < 100; ++i)
    {
        EXPECT_EQ(v[i], i);
    }
}

TEST_F(VecTest, EmplaceBack)
{
    mex::Vec<std::string> v;

    auto& ref1 = v.emplace_back("Hello");
    auto& ref2 = v.emplace_back(5, 'x');

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(ref1, "Hello");
    EXPECT_EQ(ref2, "xxxxx");
    EXPECT_EQ(v[0], "Hello");
    EXPECT_EQ(v[1], "xxxxx");
}

TEST_F(VecTest, PopBack)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    v.pop_back();
    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v.back(), 4);

    v.pop_back();
    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v.back(), 3);
}

TEST_F(VecTest, Resize)
{
    mex::Vec<int> v = {1, 2, 3};

    v.resize(5);
    EXPECT_EQ(v.size(), 5);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 0);
    EXPECT_EQ(v[4], 0);

    // Resize smaller
    v.resize(2);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST_F(VecTest, ResizeWithValue)
{
    mex::Vec<int> v = {1, 2, 3};

    v.resize(6, 99);
    EXPECT_EQ(v.size(), 6);
    EXPECT_EQ(v[3], 99);
    EXPECT_EQ(v[4], 99);
    EXPECT_EQ(v[5], 99);
}

TEST_F(VecTest, Swap)
{
    mex::Vec<int> v1 = {1, 2, 3};
    mex::Vec<int> v2 = {4, 5, 6, 7};

    v1.swap(v2);

    EXPECT_EQ(v1.size(), 4);
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v1[0], 4);
    EXPECT_EQ(v2[0], 1);
}

TEST_F(VecTest, Equality)
{
    mex::Vec<int> v1 = {1, 2, 3};
    mex::Vec<int> v2 = {1, 2, 3};
    mex::Vec<int> v3 = {1, 2, 4};

    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
}

TEST_F(VecTest, ThreeWayComparison)
{
    mex::Vec<int> v1 = {1, 2, 3};
    mex::Vec<int> v2 = {1, 2, 3};
    mex::Vec<int> v3 = {1, 2, 4};

    EXPECT_TRUE((v1 <=> v2) == 0);
    EXPECT_TRUE((v1 <=> v3) < 0);
    EXPECT_TRUE((v3 <=> v1) > 0);
}

TEST_F(VecTest, SmallBufferOptimization)
{
    mex::Vec<int> v;
    EXPECT_TRUE(v.is_small());

    for (int i = 0; i < 8; ++i)
    {
        v.push_back(i);
    }
    EXPECT_TRUE(v.is_small());
    EXPECT_EQ(v.size(), 8);

    v.push_back(8);
    EXPECT_FALSE(v.is_small());
    EXPECT_EQ(v.size(), 9);
}

TEST_F(VecTest, SmallBufferMove)
{
    mex::Vec<int> v1;
    for (int i = 0; i < 5; ++i)
    {
        v1.push_back(i);
    }
    EXPECT_TRUE(v1.is_small());

    mex::Vec<int> v2(std::move(v1));
    EXPECT_TRUE(v2.is_small());
    EXPECT_EQ(v2.size(), 5);
}

TEST_F(VecTest, StdSort)
{
    mex::Vec<int> v = {5, 2, 8, 1, 9, 3, 7};

    std::sort(v.begin(), v.end());

    EXPECT_TRUE(std::is_sorted(v.begin(), v.end()));
}

TEST_F(VecTest, StdFind)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    auto it = std::find(v.begin(), v.end(), 3);
    EXPECT_NE(it, v.end());
    EXPECT_EQ(*it, 3);

    it = std::find(v.begin(), v.end(), 99);
    EXPECT_EQ(it, v.end());
}

TEST_F(VecTest, StdAccumulate)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};

    int sum = std::accumulate(v.begin(), v.end(), 0);
    EXPECT_EQ(sum, 15);
}

TEST_F(VecTest, EraseRemoveIdiom)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5, 3, 6, 3};

    auto removed = mex::erase(v, 3);

    EXPECT_EQ(removed, 3);
    EXPECT_EQ(v.size(), 5);

    for (int x : v)
    {
        EXPECT_NE(x, 3);
    }
}

TEST_F(VecTest, EraseIf)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5, 6, 7, 8};

    auto removed = mex::erase_if(v, [](int x) { return x % 2 == 0; });

    EXPECT_EQ(removed, 4);
    EXPECT_EQ(v.size(), 4);

    for (int x : v)
    {
        EXPECT_EQ(x % 2, 1);
    }
}

TEST_F(VecTest, StringVector)
{
    mex::Vec<std::string> v;

    v.emplace_back("Hello");
    v.emplace_back("World");
    v.push_back("!");

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], "Hello");
    EXPECT_EQ(v[1], "World");
    EXPECT_EQ(v[2], "!");
}

TEST_F(VecTest, UniquePtr)
{
    mex::Vec<std::unique_ptr<int>> v;

    v.push_back(std::make_unique<int>(42));
    v.emplace_back(std::make_unique<int>(99));

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(*v[0], 42);
    EXPECT_EQ(*v[1], 99);

    auto ptr = std::move(v[0]);
    EXPECT_EQ(*ptr, 42);
}

TEST_F(VecTest, LargeVector)
{
    mex::Vec<int> v;

    for (int i = 0; i < 10000; ++i)
    {
        v.push_back(i);
    }

    EXPECT_EQ(v.size(), 10000);
    for (int i = 0; i < 10000; ++i)
    {
        EXPECT_EQ(v[i], i);
    }
}

TEST_F(VecTest, RepeatedReallocation)
{
    mex::Vec<int> v;

    for (int i = 0; i < 1000; ++i)
    {
        v.push_back(i);
        v.shrink_to_fit();
    }

    EXPECT_EQ(v.size(), 1000);
}

TEST(VecEdgeCases, EmptyConstruction)
{
    mex::Vec<int> v(0);
    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
}

TEST(VecEdgeCases, SelfAssigment)
{
    mex::Vec<int> v = {1, 2, 3};
    v = v;

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(VecEdgeCases, SelfMoveAssigment)
{
    mex::Vec<int> v = {1, 2, 3};
    v = std::move(v);

    EXPECT_TRUE(v.empty() || v.size() >= 0);
}

TEST(VecEdgeCases, ReverseNoOp)
{
    mex::Vec<int> v = {1, 2, 3};
    auto cap = v.capacity();

    v.reserve(0);
    EXPECT_EQ(v.capacity(), cap);

    v.reserve(cap);
    EXPECT_EQ(v.capacity(), cap);
}

TEST(VecEdgeCases, InsertZeroCount)
{
    mex::Vec<int> v = {1, 2, 3};
    auto it = v.insert(v.begin() + 1, 0, 999);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(it, v.begin() + 1);
}

TEST(VecEdgeCases, InsertAtEnd)
{
    mex::Vec<int> v = {1, 2, 3};
    auto it = v.insert(v.end(), 4);

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[3], 4);
}

TEST(VecEdgeCases, EraseEmptyRange)
{
    mex::Vec<int> v = {1, 2, 3};
    auto it = v.erase(v.begin() + 1, v.begin() + 1);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(it, v.begin() + 1);
}

TEST(VecEdgeCases, ComparisionDifferenSizes)
{
    mex::Vec<int> v1 = {1, 2, 3};
    mex::Vec<int> v2 = {1, 2};

    EXPECT_FALSE(v1 == v2);
    EXPECT_TRUE((v1 <=> v2) > 0);
    EXPECT_TRUE((v2 <=> v1) < 0);
}

TEST(VecEdgeCases, ComparisionEmpty)
{
    mex::Vec<int> v1;
    mex::Vec<int> v2;

    EXPECT_TRUE(v1 == v2);
    EXPECT_TRUE((v1 <=> v2) == 0);
}

TEST(VecConst, ConstIterators)
{
    const mex::Vec<int> v = {1, 2, 3, 4, 5};

    int sum = 0;
    for (int it : v)
    {
        sum += it;
    }
    EXPECT_EQ(sum, 15);

    const int* ptr = v.data();
    EXPECT_EQ(ptr[0], 1);
    EXPECT_EQ(ptr[4], 5);
}

TEST(VecConst, ConstReverseIterators)
{
    const mex::Vec<int> v = {1, 2, 3};

    std::vector<int> reversed;
    for (int it : std::ranges::reverse_view(v))
    {
        reversed.push_back(it);
    }

    EXPECT_EQ(reversed[0], 3);
    EXPECT_EQ(reversed[2], 1);
}

TEST(VecGrowth, CapacityProgression)
{
    mex::Vec<int> v;
    EXPECT_EQ(v.capacity(), 8);

    for (int i = 0; i < 8; ++i)
    {
        v.push_back(i);
    }
    EXPECT_EQ(v.capacity(), 8);
    EXPECT_TRUE(v.is_small());

    v.push_back(8);
    EXPECT_FALSE(v.is_small());
    EXPECT_GE(v.capacity(), 9);
}

TEST(VecGrowth, MultipleReallocations)
{
    mex::Vec<int> v;
    std::vector<size_t> capacities;

    for (int i = 0; i < 100; ++i)
    {
        auto old_cap = v.capacity();
        v.push_back(i);
        if (v.capacity() != old_cap)
        {
            capacities.push_back(v.capacity());
        }
    }

    EXPECT_EQ(capacities.size(), 3);

    for (size_t i = 1; i < capacities.size(); ++i)
    {
        EXPECT_GT(capacities[i], capacities[i-1]);
    }
}

TEST(VecSwap, BothOnHeap)
{
    mex::Vec<int> v1(20, 1);
    mex::Vec<int> v2(30, 2);

    EXPECT_FALSE(v1.is_small());
    EXPECT_FALSE(v2.is_small());

    v1.swap(v2);

    EXPECT_EQ(v1.size(), 30);
    EXPECT_EQ(v2.size(), 20);
    EXPECT_EQ(v1[0], 2);
    EXPECT_EQ(v2[0], 1);
}

TEST(VecSwap, OneSmallOneHeap)
{
    mex::Vec<int> v1 = {1, 2, 3};
    mex::Vec<int> v2(20, 9);

    EXPECT_TRUE(v1.is_small());
    EXPECT_FALSE(v2.is_small());

    v1.swap(v2);

    EXPECT_FALSE(v1.is_small());
    EXPECT_TRUE(v2.is_small());
    EXPECT_EQ(v1.size(), 20);
    EXPECT_EQ(v2.size(), 3);
}

TEST(VecSwap, EmptyVectors)
{
    mex::Vec<int> v1;
    mex::Vec<int> v2;

    v1.swap(v2);

    EXPECT_TRUE(v1.empty());
    EXPECT_TRUE(v2.empty());
}

TEST(VecSwap, SelfSwap)
{
    mex::Vec<int> v = {1, 2, 3};
    v.swap(v);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
}

TEST(VecTemplateParams, SmallSize1)
{
    mex::Vec<int, 1> v;
    EXPECT_EQ(v.capacity(), 1);
    EXPECT_TRUE(v.is_small());

    v.push_back(1);
    EXPECT_TRUE(v.is_small());

    v.push_back(2);
    EXPECT_FALSE(v.is_small());
}

TEST(VecTemplateParams, SmallSize16)
{
    mex::Vec<int, 16> v;
    EXPECT_EQ(v.capacity(), 16);

    for (int i = 0; i < 16; ++i)
    {
        v.push_back(i);
    }
    EXPECT_TRUE(v.is_small());

    v.push_back(16);
    EXPECT_FALSE(v.is_small());
}

TEST(VecTemplateParams, GrowthFactor11)
{
    mex::Vec<int, 8, 11> v;

    for (int i = 0; i < 9; ++i)
    {
        v.push_back(i);
    }

    EXPECT_GE(v.capacity(), 9);
}

TEST(VecTemplateParams, GrowthFactor20)
{
    mex::Vec<int, 8, 20> v;

    for (int i = 0; i < 9; ++i)
    {
        v.push_back(i);
    }

    EXPECT_GE(v.capacity(), 16);
}

struct NonTrivial
{
    std::unique_ptr<int> ptr;
    static int constructions;
    static int destructions;

    NonTrivial() : ptr(std::make_unique<int>(42))
    {
        ++constructions;
    }

    explicit NonTrivial(int val) : ptr(std::make_unique<int>(val))
    {
        ++constructions;
    }

    NonTrivial(const NonTrivial& other)
            : ptr(std::make_unique<int>(*other.ptr))
    {
        ++constructions;
    }

    NonTrivial(NonTrivial&& other) noexcept = default;

    ~NonTrivial()
    {
        ++destructions;
    }

    NonTrivial& operator=(const NonTrivial&) = delete;
    NonTrivial& operator=(NonTrivial&&) noexcept = default;

    static void reset()
    {
        constructions = 0;
        destructions = 0;
    }
};

int NonTrivial::constructions = 0;
int NonTrivial::destructions = 0;

TEST(VecNonTrivial, ConstructDestruct)
{
    NonTrivial::reset();

    {
        mex::Vec<NonTrivial> v;
        v.emplace_back(1);
        v.emplace_back(2);
        v.emplace_back(3);

        EXPECT_EQ(NonTrivial::constructions, 3);
    }

    EXPECT_EQ(NonTrivial::destructions, 3);
}

TEST(VecNonTrivial, Reallocation)
{
    NonTrivial::reset();

    {
        mex::Vec<NonTrivial, 2> v;
        v.emplace_back(1);
        v.emplace_back(2);

        int before_constructions = NonTrivial::constructions;

        v.emplace_back(3);

        EXPECT_EQ(NonTrivial::constructions, before_constructions + 1);
    }
}

TEST(VecIterators, InvalidationAfterReserve)
{
    mex::Vec<int> v = {1, 2, 3, 4, 5};
    auto old_data = v.data();

    v.reserve(100);

    if (v.capacity() != 8)
    {
        EXPECT_EQ(v.size(), 5);
        EXPECT_EQ(v[0], 1);
    }
}

TEST(VecIterators, ValidAfterPushBackWithCapacity)
{
    mex::Vec<int> v;
    v.reserve(10);

    auto it = v.begin();
    v.push_back(1);

    EXPECT_EQ(*it, 1);
}

TEST(VecCapacity, MaxSize)
{
    mex::Vec<int> v;
    auto max = v.max_size();

    EXPECT_GT(max, 1000000ULL);
}

TEST(VecAssign, AssignFromInitializerList)
{
    mex::Vec<int> v = {1, 2, 3};
    v.assign({10, 20, 30, 40});

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 10);
    EXPECT_EQ(v[3], 40);
}

TEST(VecAssign, AssignEmptyRange)
{
    mex::Vec<int> v = {1, 2, 3};
    std::vector<int> empty;

    v.assign(empty.begin(), empty.end());

    EXPECT_TRUE(v.empty());
}

TEST(VecInsert, InsertAtBeginning)
{
    mex::Vec<int> v = {2, 3, 4};
    v.insert(v.begin(), 1);

    EXPECT_EQ(v.size(), 4);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
}

TEST(VecInsert, InsertInitializerList)
{
    mex::Vec<int> v = {1, 5};
    v.insert(v.begin() + 1, {2, 3, 4});

    EXPECT_EQ(v.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(v[i], i + 1);
    }
}

TEST(VecEmplace, EmplaceAtBeginning)
{
    mex::Vec<std::string> v = {"world"};
    v.emplace(v.begin(), "hello");

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0], "hello");
    EXPECT_EQ(v[1], "world");
}

TEST(VecResize, ResizeToZero)
{
    mex::Vec<int> v = {1, 2, 3};
    v.resize(0);

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.size(), 0);
}

TEST(VecResize, ResizeToSameSize)
{
    mex::Vec<int> v = {1, 2, 3};
    v.resize(3);

    EXPECT_EQ(v.size(), 3);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[2], 3);
}

TEST(VecResize, ResizeLargeToSmall)
{
    mex::Vec<int> v(100, 42);
    v.resize(5);

    EXPECT_EQ(v.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(v[i], 42);
    }
}

TEST(VecClear, ClearEmpty)
{
    mex::Vec<int> v;
    v.clear();

    EXPECT_TRUE(v.empty());
}

TEST(VecClear, ClearHeapVector)
{
    mex::Vec<int> v(100, 1);
    auto cap = v.capacity();

    v.clear();

    EXPECT_TRUE(v.empty());
    EXPECT_EQ(v.capacity(), cap);
}

TEST(VecSBO, MoveFromHeapToSmall)
{
    mex::Vec<int> v1(20, 1);
    EXPECT_FALSE(v1.is_small());

    v1.resize(3);
    v1.shrink_to_fit();

    EXPECT_TRUE(v1.is_small());
    EXPECT_EQ(v1.size(), 3);
}

TEST(VecLarge, VeryLargeVector)
{
    mex::Vec<int> v;
    v.reserve(100000);

    for (int i = 0; i < 100000; ++i)
    {
        v.push_back(i);
    }

    EXPECT_EQ(v.size(), 100000);
    EXPECT_EQ(v[0], 0);
    EXPECT_EQ(v[99999], 99999);
}

TEST(VecSBO, CopySmallVector)
{
    mex::Vec<int> v1 = {1, 2, 3};
    EXPECT_TRUE(v1.is_small());

    mex::Vec<int> v2 = v1;

    EXPECT_TRUE(v2.is_small());
    EXPECT_EQ(v2.size(), 3);
    EXPECT_EQ(v2[0], 1);
}

TEST(VecSBO, MoveSmallToSmall)
{
    mex::Vec<int> v1 = {1, 2, 3};
    EXPECT_TRUE(v1.is_small());

    mex::Vec<int> v2 = std::move(v1);

    EXPECT_TRUE(v2.is_small());
    EXPECT_EQ(v2.size(), 3);
    EXPECT_TRUE(v1.empty());
}

TEST(VecComplexTypes, VectorTypes)
{
    mex::Vec<std::vector<int>> v;

    v.emplace_back(std::vector<int>{1, 2, 3});
    v.emplace_back(std::vector<int>{4, 5});

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].size(), 3);
    EXPECT_EQ(v[1].size(), 2);
    EXPECT_EQ(v[0][0], 1);
    EXPECT_EQ(v[1][1], 5);
}

TEST(VecComplexTypes, VectorOfVecs)
{
    mex::Vec<mex::Vec<int>> v;

    v.emplace_back(mex::Vec<int>{1, 2, 3});
    v.emplace_back(mex::Vec<int>{4, 5});

    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].size(), 3);
    EXPECT_EQ(v[1].size(), 2);
    EXPECT_EQ(v[0][0], 1);
    EXPECT_EQ(v[1][1], 5);
}

TEST(VecComplexTypes, VectorOfStrings)
{
    class ComplexClass
    {
    public:
        std::string name;
        int value;

        ComplexClass(std::string n, int v) : name(std::move(n)), value(v) {}

        bool operator==(const ComplexClass& other) const
        {
            return name == other.name && value == other.value;
        }

        bool operator<(const ComplexClass& other) const
        {
            return std::tie(name, value) < std::tie(other.name, other.value);
        }

        auto operator<=>(const ComplexClass& other) const = default;
    };

    mex::Vec<ComplexClass> v;
    v.emplace_back("Alice", 30);
    v.emplace_back("Bob", 25);
    EXPECT_EQ(v.size(), 2);
    EXPECT_EQ(v[0].name, "Alice");
    EXPECT_EQ(v[0].value, 30);

    EXPECT_EQ(v[1].name, "Bob");
    EXPECT_EQ(v[1].value, 25);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}