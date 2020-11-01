#pragma once

#include "spoor/runtime/buffer/circular_buffer.h"
#include "util/memory/owned_ptr.h"
#include "util/memory/ptr_owner.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

template <class T>
class BufferSlicePool : public util::memory::PtrOwner<CircularBuffer<T>> {
 public:
  enum class BorrowError;

  using Buffer = CircularBuffer<T>;
  using SizeType = typename Buffer::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Buffer>;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;

  enum class BorrowError {
    kNoSlicesAvailable,
    kCasAttemptsExhausted,
  };

  virtual constexpr ~BufferSlicePool() = default;
  [[nodiscard]] virtual constexpr auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult = 0;

  [[nodiscard]] virtual constexpr auto Size() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Capacity() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Empty() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto Full() const -> bool = 0;

 protected:
  constexpr BufferSlicePool() = default;
  constexpr BufferSlicePool(const BufferSlicePool&) = default;
  constexpr BufferSlicePool(BufferSlicePool&&) noexcept = default;
  constexpr auto operator=(const BufferSlicePool&)
      -> BufferSlicePool& = default;
  constexpr auto operator=(BufferSlicePool&&) noexcept
      -> BufferSlicePool& = default;
};

}  // namespace spoor::runtime::buffer
