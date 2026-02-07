#include "mob/cache/CacheEvictor.hpp"
#include <gtest/gtest.h>

using namespace mob::cache;

TEST(CacheTest, CacheEvictorTest)
{
    PlatformIndependentCacheEvictor evictor(64 * 1024 * 1024);
    evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Sequential
    );

    evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L2,
        PlatformIndependentCacheEvictor::EvictionStrategy::Random
    );

    evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::All,
        PlatformIndependentCacheEvictor::EvictionStrategy::Mixed
    );
}

TEST(CacheTest, AllEvictionStrategies)
{
    PlatformIndependentCacheEvictor evictor(32 * 1024 * 1024);

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Sequential
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Random
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Stride
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::PrimeStride
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Mixed
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Pattern
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Alternating
    ));
}

TEST(CacheTest, AllCacheLevels)
{
    PlatformIndependentCacheEvictor evictor(32 * 1024 * 1024);

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Sequential
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L2,
        PlatformIndependentCacheEvictor::EvictionStrategy::Sequential
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L3,
        PlatformIndependentCacheEvictor::EvictionStrategy::Sequential
    ));

    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::All,
        PlatformIndependentCacheEvictor::EvictionStrategy::Sequential
    ));
}

TEST(CacheTest, DefaultParameters)
{
    EXPECT_NO_THROW(PlatformIndependentCacheEvictor evictor);

    PlatformIndependentCacheEvictor evictor;
    EXPECT_NO_THROW(evictor.evictCache());
}

TEST(CacheTest, CustomBufferSize)
{
    EXPECT_NO_THROW(PlatformIndependentCacheEvictor evictor(128 * 1024 * 1024));

    PlatformIndependentCacheEvictor evictor(16 * 1024 * 1024);
    EXPECT_NO_THROW(evictor.evictCache(
        PlatformIndependentCacheEvictor::CacheLevel::L1,
        PlatformIndependentCacheEvictor::EvictionStrategy::Random
    ));
}

