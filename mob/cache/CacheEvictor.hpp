#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <random>
#include <atomic>
#include <memory>
#include <iostream>
#include <iomanip>
#include <functional>
#include <cmath>
#include <bitset>
#include <limits>
#include <sstream>

namespace mob::cache
{

class PlatformIndependentCacheEvictor {
public:

    enum class EvictionStrategy
    {
        Sequential,
        Random,
        Stride,
        PrimeStride,
        Mixed,
        Pattern,
        Alternating
    };


    enum class CacheLevel
    {
        L1,
        L2,
        L3,
        All
    };

private:
    struct CacheParams
    {
        size_t l1SizeBytes = 32 * 1024;
        size_t l2SizeBytes = 256 * 1024;
        size_t l3SizeBytes = 8 * 1024 * 1024;
        size_t cacheLineBytes = 64;
        size_t associativity = 8;
        size_t pageSize = 4096;
        
        size_t getTargetSize(CacheLevel level) const
        {
            switch (level)
            {
                case CacheLevel::L1: return l1SizeBytes * 2;
                case CacheLevel::L2: return l2SizeBytes * 2;
                case CacheLevel::L3: return l3SizeBytes * 2;
                case CacheLevel::All: return l3SizeBytes * 4;
                default: return l3SizeBytes * 2;
            }
        }
    };


    class EvictionBuffer
    {
    private:
        std::vector<uint8_t> buffer_;
        size_t size_;
        size_t lineSize_;
        std::unique_ptr<uint8_t[]> alignedBuffer_;
        bool useCustomAlignment_;
        
    public:
        EvictionBuffer(size_t size, size_t lineSize = 64, bool forceAlignment = true) 
            : size_(size), lineSize_(lineSize), useCustomAlignment_(forceAlignment)
        {
            
            if (useCustomAlignment_ && lineSize_ > 0) {
                size_t alignedSize = ((size_ + lineSize_ - 1) / lineSize_) * lineSize_;
                alignedBuffer_ = std::make_unique<uint8_t[]>(alignedSize + lineSize_);
                
                uintptr_t addr = reinterpret_cast<uintptr_t>(alignedBuffer_.get());
                uintptr_t alignedAddr = (addr + lineSize_ - 1) & ~(lineSize_ - 1);
                buffer_.assign(
                    reinterpret_cast<uint8_t*>(alignedAddr),
                    reinterpret_cast<uint8_t*>(alignedAddr) + alignedSize
                );
                
                std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<uint8_t> dist(0, 255);
                for (auto& byte : buffer_) {
                    byte = dist(rng);
                }
            } else {
                buffer_.resize(size_);
                std::mt19937 rng(std::random_device{}());
                std::uniform_int_distribution<uint8_t> dist(0, 255);
                std::generate(buffer_.begin(), buffer_.end(), [&]() { return dist(rng); });
            }
        }

        size_t getSize() const { return buffer_.size(); }
        
        uint8_t& accessLine(size_t lineIndex, size_t offset = 0) {
            size_t index = lineIndex * lineSize_ + offset;
            if (index < buffer_.size()) {
                return buffer_[index];
            }
            throw std::out_of_range("Cache line index out of range");
        }
        
        const uint8_t& accessLine(size_t lineIndex, size_t offset = 0) const {
            size_t index = lineIndex * lineIndex + offset;
            if (index < buffer_.size()) {
                return buffer_[index];
            }
            throw std::out_of_range("Cache line index out of range");
        }
    };

private:
    CacheParams cacheParams_;
    std::vector<std::unique_ptr<EvictionBuffer>> buffers_;
    std::mt19937 rng_;
    size_t defaultBufferSize_;


public:
    PlatformIndependentCacheEvictor(
        size_t defaultBufferSize = 64 * 1024 * 1024
    ) : defaultBufferSize_(defaultBufferSize), rng_(std::random_device{}())
    {
        addBuffer(defaultBufferSize_);
    }
    
    void evictCache(
        CacheLevel level = CacheLevel::All,
        EvictionStrategy strategy = EvictionStrategy::Mixed
    ) { 
        size_t targetSize = cacheParams_.getTargetSize(level);
        
        size_t bytesAccessed = 0;
        
        switch (strategy) {
            case EvictionStrategy::Sequential:
                bytesAccessed = evictSequential(targetSize);
                break;
            case EvictionStrategy::Random:
                bytesAccessed = evictRandom(targetSize);
                break;
            case EvictionStrategy::Stride:
                bytesAccessed = evictStride(targetSize, 17); // 17字节步长
                break;
            case EvictionStrategy::PrimeStride:
                bytesAccessed = evictPrimeStride(targetSize);
                break;
            case EvictionStrategy::Mixed:
                bytesAccessed = evictMixed(targetSize);
                break;
            case EvictionStrategy::Pattern: bytesAccessed = evictPattern(targetSize);
                break;
            case EvictionStrategy::Alternating:
                bytesAccessed = evictAlternating(targetSize);
                break;
        }
    }
    
    
private:
    size_t evictSequential(size_t targetSize)
    {
        size_t bytesAccessed = 0;
        
        for (auto& buffer : buffers_)
        {
            size_t bufferSize = buffer->getSize();
            size_t lineCount = bufferSize / cacheParams_.cacheLineBytes;
            size_t linesToAccess = std::min(lineCount, targetSize / cacheParams_.cacheLineBytes);
            
            for (size_t i = 0; i < linesToAccess; ++i)
            {
                volatile uint8_t value = buffer->accessLine(i);
                (void)value;
                bytesAccessed += cacheParams_.cacheLineBytes;
            }
        }
        
        return bytesAccessed;
    }

    void addBuffer(size_t size, bool forceAlignment = true)
    {
        auto buffer = std::make_unique<EvictionBuffer>(size, cacheParams_.cacheLineBytes, forceAlignment);
        buffers_.push_back(std::move(buffer));
    }
    
    
    size_t evictRandom(size_t targetSize)
    {
        size_t bytesAccessed = 0;
        
        for (auto& buffer : buffers_)
        {
            size_t bufferSize = buffer->getSize();
            size_t lineCount = bufferSize / cacheParams_.cacheLineBytes;
            size_t linesToAccess = std::min(lineCount, targetSize / cacheParams_.cacheLineBytes);
            
            std::vector<size_t> indices(lineCount);
            std::iota(indices.begin(), indices.end(), 0);
            std::shuffle(indices.begin(), indices.end(), rng_);
            
            for (size_t i = 0; i < linesToAccess; ++i)
            {
                volatile uint8_t value = buffer->accessLine(indices[i]);
                (void)value;
                bytesAccessed += cacheParams_.cacheLineBytes;
            }
        }
        
        return bytesAccessed;
    }
    
    size_t evictStride(size_t targetSize, size_t stride)
    {
        size_t bytesAccessed = 0;
        
        for (auto& buffer : buffers_) {
            size_t bufferSize = buffer->getSize();
            size_t lineCount = bufferSize / cacheParams_.cacheLineBytes;
            size_t linesToAccess = std::min(lineCount, targetSize / cacheParams_.cacheLineBytes);
            
            for (size_t i = 0; i < linesToAccess; i += stride) {
                volatile uint8_t value = buffer->accessLine(i);
                (void)value;
                bytesAccessed += cacheParams_.cacheLineBytes;
            }
        }
        
        return bytesAccessed;
    }
    
    size_t evictPrimeStride(size_t targetSize)
    {
        const size_t prime = 97;
        return evictStride(targetSize, prime);
    }
    
    size_t evictMixed(size_t targetSize)
    {
        size_t bytesAccessed = 0;
        bytesAccessed += evictSequential(targetSize / 3);
        bytesAccessed += evictRandom(targetSize / 3);
        bytesAccessed += evictPrimeStride(targetSize / 3);
        return bytesAccessed;
    }
    
    size_t evictPattern(size_t targetSize)
    {
        size_t bytesAccessed = 0;
        
        const size_t pattern[] = {0, 3, 1, 4, 2, 5};
        const size_t patternSize = sizeof(pattern) / sizeof(pattern[0]);
        
        for (auto& buffer : buffers_)
        {
            size_t bufferSize = buffer->getSize();
            size_t lineCount = bufferSize / cacheParams_.cacheLineBytes;
            size_t linesToAccess = std::min(lineCount, targetSize / cacheParams_.cacheLineBytes);
            
            for (size_t i = 0; i < linesToAccess; ++i) {
                size_t patternIdx = i % patternSize;
                size_t offset = pattern[patternIdx];
                volatile uint8_t value = buffer->accessLine(i, offset);
                (void)value;
                bytesAccessed += cacheParams_.cacheLineBytes;
            }
        }
        
        return bytesAccessed;
    }
    
    size_t evictAlternating(size_t targetSize)
    {
        size_t bytesAccessed = 0;
        
        for (auto& buffer : buffers_)
        {
            size_t bufferSize = buffer->getSize();
            size_t lineCount = bufferSize / cacheParams_.cacheLineBytes;
            size_t linesToAccess = std::min(lineCount, targetSize / cacheParams_.cacheLineBytes);
            
            for (size_t i = 0; i < linesToAccess; ++i)
            {
                if (i % 2 == 0) {
                    volatile uint8_t value = buffer->accessLine(i);
                    (void)value;
                } else {
                    buffer->accessLine(i) = static_cast<uint8_t>(i);
                }
                bytesAccessed += cacheParams_.cacheLineBytes;
            }
        }
        
        return bytesAccessed;
    }
};

}