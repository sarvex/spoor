#include "spoor/runtime/buffer/buffer_slice.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <numeric>
#include <type_traits>
#include <utility>

#include "spoor/runtime/buffer/contiguous_memory.h"
#include "util/numeric.h"

namespace {

using BufferSlice = spoor::runtime::buffer::BufferSlice<int64>;
using ValueType = BufferSlice::ValueType;
using SizeType = BufferSlice::SizeType;
using ContiguousMemory = spoor::runtime::buffer::ContiguousMemory<ValueType>;

TEST(BufferSlice, UnownedConstructor) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    std::vector<int64> data(capacity);
    {
      BufferSlice slice{data.data(), capacity};
      for (SizeType i{0}; i < capacity; ++i) {
        slice.Push(i);
      }
    }
    std::vector<int64> expected(capacity);
    std::iota(expected.begin(), expected.end(), 0);
    ASSERT_EQ(data, expected);
  }
}

TEST(BufferSlice, UnownedConstructorNullptr) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    BufferSlice slice{nullptr, capacity};
    ASSERT_EQ(slice.Capacity(), 0);
    ASSERT_TRUE(slice.Empty());
    ASSERT_TRUE(slice.Full());
    for (SizeType i{0}; i < 2 * capacity; ++i) {
      slice.Push(i);
      ASSERT_EQ(slice.Capacity(), 0);
      ASSERT_TRUE(slice.Empty());
      ASSERT_TRUE(slice.Full());
    }
  }
}

TEST(BufferSlice, Clear) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    BufferSlice slice{capacity};
    for (SizeType i{0}; i < capacity; ++i) {
      slice.Push(i);
    }
    slice.Clear();
    ASSERT_EQ(slice.Size(), 0);
  }
}

TEST(BufferSlice, Size) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    BufferSlice slice{capacity};
    for (SizeType i{0}; i < 2 * capacity; ++i) {
      slice.Push(i);
      ASSERT_EQ(slice.Size(), std::min(i + 1, capacity));
    }
  }
}

TEST(BufferSlice, Capacity) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    BufferSlice slice{capacity};
    for (SizeType i{0}; i < 2 * capacity; ++i) {
      slice.Push(i);
      ASSERT_EQ(slice.Capacity(), capacity);
    }
  }
}

TEST(BufferSlice, Empty) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    BufferSlice slice{capacity};
    ASSERT_TRUE(slice.Empty());
    for (SizeType i{0}; i < capacity; ++i) {
      slice.Push(i);
      ASSERT_FALSE(slice.Empty());
    }
  }
}

TEST(BufferSlice, WillWrapOnNextPush) {  // NOLINT
  for (const SizeType capacity : {0, 1, 2, 10}) {
    BufferSlice slice{capacity};
    if (capacity == 0) ASSERT_TRUE(slice.WillWrapOnNextPush());
    for (SizeType i{0}; i < 5 * capacity; ++i) {
      if ((i + 1) % capacity == 0) {
        ASSERT_TRUE(slice.WillWrapOnNextPush());
      } else {
        ASSERT_FALSE(slice.WillWrapOnNextPush());
      }
      slice.Push(i);
      if ((i + 1) % capacity != 0 && capacity < i + 1) {
        ASSERT_EQ(slice.ContiguousMemoryChunks().size(), 2);
      } else {
        ASSERT_EQ(slice.ContiguousMemoryChunks().size(), 1);
      }
    }
  }
}

TEST(BufferSlice, ContiguousMemoryChunksEmpty) {  // NOLINT
  BufferSlice slice{0};
  const std::vector<ContiguousMemory> empty{};
  ASSERT_EQ(slice.ContiguousMemoryChunks(), empty);
}

TEST(BufferSlice, ContiguousMemoryChunksOneChunk) {  // NOLINT
  const SizeType capacity{5};
  BufferSlice slice{capacity};

  const std::vector<ContiguousMemory> empty{};
  ASSERT_EQ(slice.ContiguousMemoryChunks(), empty);

  std::vector<ValueType> expected{};
  for (SizeType i{0}; i < capacity; ++i) {
    slice.Push(i);
    expected.push_back(i);
    const auto chunks = slice.ContiguousMemoryChunks();
    ASSERT_EQ(chunks.size(), 1);
    auto chunk = *chunks.begin();
    ASSERT_EQ(chunk.size, (i + 1) * sizeof(ValueType));
    ASSERT_EQ(std::memcmp(chunk.begin, expected.data(), chunk.size), 0);
  }
  for (SizeType i{capacity}; i < 5 * capacity; ++i) {
    slice.Push(i);
    if ((i + 1) % capacity == 0) {
      const auto chunks = slice.ContiguousMemoryChunks();
      ASSERT_EQ(chunks.size(), 1);
      const auto chunk = *chunks.begin();
      ASSERT_EQ(chunk.size, capacity * sizeof(ValueType));
      std::vector<ValueType> expected(capacity);
      std::iota(expected.begin(), expected.end(), i - capacity + 1);
      ASSERT_EQ(std::memcmp(chunk.begin, expected.data(), chunk.size), 0);
    }
  }
}

TEST(BufferSlice, ContiguousMemoryChunksTwoChunks) {  // NOLINT
  const SizeType capacity{5};
  BufferSlice slice{capacity};

  std::vector<ValueType> expected{};
  for (SizeType i{0}; i < capacity; ++i) {
    slice.Push(i);
  }
  for (SizeType i{capacity}; i < 5 * capacity; ++i) {
    slice.Push(i);
    if ((i + 1) % capacity != 0) {
      const auto chunks = slice.ContiguousMemoryChunks();
      ASSERT_EQ(chunks.size(), 2);

      const auto first_chunk = *chunks.begin();
      const auto expected_first_chunk_size = (capacity - (i + 1) % capacity);
      ASSERT_EQ(first_chunk.size,
                expected_first_chunk_size * sizeof(ValueType));
      std::vector<ValueType> expected_first_chunk(expected_first_chunk_size);
      std::iota(expected_first_chunk.begin(), expected_first_chunk.end(),
                i - capacity + 1);
      ASSERT_EQ(std::memcmp(first_chunk.begin, expected_first_chunk.data(),
                            first_chunk.size),
                0);

      const auto second_chunk = *std::prev(chunks.end());
      const auto expected_second_chunk_size = ((i + 1) % capacity);
      ASSERT_EQ(second_chunk.size,
                expected_second_chunk_size * sizeof(ValueType));
      std::vector<ValueType> expected_second_chunk(expected_second_chunk_size);
      std::iota(expected_second_chunk.begin(), expected_second_chunk.end(),
                i - capacity + 1 + expected_first_chunk_size);
      ASSERT_EQ(std::memcmp(second_chunk.begin, expected_second_chunk.data(),
                            second_chunk.size),
                0);
    }
  }
}

}  // namespace
