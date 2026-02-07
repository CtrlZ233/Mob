#include "mob/random/Random.hpp"
#include <gtest/gtest.h>
#include <set>

using namespace mob::random;

TEST(RandomTest, SeededGenerator)
{
    RandomGenerator<> gen1(12345);
    RandomGenerator<> gen2(12345);

    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(gen1.next(), gen2.next());
    }
}

TEST(RandomTest, NextGeneration)
{
    RandomGenerator<> gen(42);

    std::set<uint64_t> values;
    for (int i = 0; i < 100; ++i) {
        values.insert(gen.next());
    }

    EXPECT_GT(values.size(), 90);
}

TEST(RandomTest, VisibleStringLength)
{
    RandomGenerator<> gen(42);

    auto str1 = gen.getVisiableString(5, 5);
    EXPECT_EQ(str1.length(), 5);

    auto str2 = gen.getVisiableString(10, 20);
    EXPECT_GE(str2.length(), 10);
    EXPECT_LE(str2.length(), 20);

    auto str3 = gen.getVisiableString(0, 0);
    EXPECT_EQ(str3.length(), 0);
}

TEST(RandomTest, VisibleStringCharset)
{
    RandomGenerator<> gen(42);
    const std::string expectedCharset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789";

    auto str = gen.getVisiableString(100, 100);

    for (char c : str) {
        EXPECT_NE(expectedCharset.find(c), std::string::npos);
    }
}

TEST(RandomTest, CustomCharset)
{
    RandomGenerator<> gen(42);
    const std::string customCharset = "ABC123";

    auto str = gen.getString(50, customCharset);
    EXPECT_EQ(str.length(), 50);

    for (char c : str) {
        EXPECT_NE(customCharset.find(c), std::string::npos);
    }
}

TEST(RandomTest, EmptyCharsetError)
{
    RandomGenerator<> gen(42);
    const std::string emptyCharset = "";

    EXPECT_THROW(gen.getString(10, emptyCharset), std::range_error);
}

TEST(RandomTest, InvalidLengthError)
{
    RandomGenerator<> gen(42);

    EXPECT_THROW(gen.getVisiableString(20, 10), std::range_error);
}

TEST(RandomTest, ZeroLengthString)
{
    RandomGenerator<> gen(42);

    auto str = gen.getVisiableString(0, 0);
    EXPECT_EQ(str.length(), 0);
    EXPECT_TRUE(str.empty());
}

TEST(RandomTest, LargeString)
{
    RandomGenerator<> gen(42);

    auto str = gen.getVisiableString(10000, 10000);
    EXPECT_EQ(str.length(), 10000);
}

TEST(RandomTest, SeededReproducibility)
{
    RandomGenerator<> gen1(999);
    RandomGenerator<> gen2(999);

    auto str1 = gen1.getVisiableString(50, 50);
    auto str2 = gen2.getVisiableString(50, 50);

    EXPECT_EQ(str1, str2);
}

