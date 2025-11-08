#include "spdlog/spdlog.h"
#include "fmt/format.h"
#include "gtest/gtest.h"

#include <string>
#include <iostream>

TEST(ImportTest, SpdLogTest)
{
    spdlog::info("spdlog test");
}

TEST(ImportTest, FmtTest)
{
    std::string name = "fmt";
    auto testStr = fmt::format("{} test", name);
    std::cout << testStr << std::endl;
}
