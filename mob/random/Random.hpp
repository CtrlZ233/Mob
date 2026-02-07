#pragma once

#include <random>
#include <chrono>
#include <memory>
#include <string>
#include <stdexcept>

namespace mob::random
{

template<typename Engine = std::mt19937>
class RandomGenerator
{
public:
    explicit RandomGenerator(uint64_t seed) : seed_(seed)
    {
        engine_ = std::make_unique<Engine>(seed);
    }

    RandomGenerator()
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto seed = now.time_since_epoch().count();
        engine_ = std::make_unique<Engine>(seed);
    }

    uint64_t next()
    {
        return (*engine_)();
    }

    std::string getVisiableString(size_t minLen, size_t maxLen)
    {
        if (minLen > maxLen)
        {
            throw std::range_error("minLen must greater than maxLen");
        }
        static const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789";
        size_t length = minLen == maxLen ? minLen : minLen + (next() % (maxLen - minLen));
        return getString(length, charset);
    }

    std::string getString(size_t length, const std::string &charset)
    {
        if (charset.empty())
        {
            throw std::range_error("charset cannot be empty");
        }
        std::string result;
        result.reserve(length);
        for (size_t i = 0; i < length; ++i)
        {
            result.push_back(charset[next() % charset.size()]);
        }
        return result;
    }

private:

    std::unique_ptr<Engine> engine_;
    uint64_t seed_;
};

}