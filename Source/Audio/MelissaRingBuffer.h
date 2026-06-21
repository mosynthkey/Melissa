//
//  MelissaRingBuffer.h
//  Melissa
//
//  Copyright(c) 2020 Masaki Ono
//

#pragma once

#include <cassert>
#include <cstddef>

template<typename T, size_t Capacity>
class MelissaRingBuffer
{
public:
    MelissaRingBuffer() : readIndex_(0), writeIndex_(0), count_(0)
    {
    }

    void push(T value)
    {
        assert(count_ < Capacity);
        data_[writeIndex_] = value;
        writeIndex_ = (writeIndex_ + 1) % Capacity;
        ++count_;
    }

    T pop()
    {
        assert(count_ > 0);
        T value = data_[readIndex_];
        readIndex_ = (readIndex_ + 1) % Capacity;
        --count_;
        return value;
    }

    T& operator[](size_t index)
    {
        assert(index < count_);
        return data_[(readIndex_ + index) % Capacity];
    }

    const T& operator[](size_t index) const
    {
        assert(index < count_);
        return data_[(readIndex_ + index) % Capacity];
    }

    size_t size() const
    {
        return count_;
    }

    bool empty() const
    {
        return count_ == 0;
    }

    bool full() const
    {
        return count_ >= Capacity;
    }

    void clear()
    {
        readIndex_ = 0;
        writeIndex_ = 0;
        count_ = 0;
    }

    void erase(size_t count)
    {
        assert(count <= count_);
        readIndex_ = (readIndex_ + count) % Capacity;
        count_ -= count;
    }

    static constexpr size_t capacity()
    {
        return Capacity;
    }

private:
    T data_[Capacity];
    size_t readIndex_;
    size_t writeIndex_;
    size_t count_;
};
