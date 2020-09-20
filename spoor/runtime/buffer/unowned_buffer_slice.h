// TODO decide if we need to call each items' destructor on clear and
// destruction
// #include <iterator>
// std::for_each(vec.begin(), vec.end(), std::default_delete<Base>());
//  if (!Full() || insertion_iterator_ == buffer_.begin()) {
//    for (auto it = buffer_.rbegin(); it != buffer_.rend(); ++it) {
//      it->~T();
//    }
//  } else {
//    auto rinsertion_iterator =
//        std::next(std::make_reverse_iterator(insertion_iterator_));
//    for (auto it = rinsertion_iterator; it != buffer_.rend(); ++it) {
//      it->~T();
//    }
//    for (auto it = buffer_.rbegin(); it != rinsertion_iterator; ++it) {
//      it->~T();
//    }
//  }
#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_UNOWNED_BUFFER_SLICE_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_UNOWNED_BUFFER_SLICE_H_

#include <iostream>  // TODO
#include <span>
#include <vector>

#include "spoor/runtime/buffer/buffer_slice.h"

namespace spoor::runtime::buffer {

template <class T>
class UnownedBufferSlice final : public BufferSlice<T> {
 public:
  using SizeType = typename BufferSlice<T>::SizeType;

  UnownedBufferSlice() = delete;
  explicit constexpr UnownedBufferSlice(std::span<T> buffer);
  UnownedBufferSlice(const UnownedBufferSlice&) = delete;
  // TODO is the default sufficient?
  constexpr UnownedBufferSlice(UnownedBufferSlice&& other) noexcept = default;
  auto operator=(const UnownedBufferSlice&) -> UnownedBufferSlice& = delete;
  auto operator=(UnownedBufferSlice&&) noexcept -> UnownedBufferSlice& = delete;
  ~UnownedBufferSlice() = default;

  constexpr auto Push(const T& item) -> void override;
  constexpr auto Push(T&& item) -> void override;
  constexpr auto Clear() -> void override;
  [[nodiscard]] auto ContiguousMemoryChunks()
      -> std::vector<std::span<T>> override;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;
  [[nodiscard]] constexpr auto WillWrapOnNextPush() const -> bool override;

 private:
  std::span<T> buffer_;
  typename std::span<T>::iterator insertion_iterator_;
  SizeType size_;
};

template <class T>
constexpr UnownedBufferSlice<T>::UnownedBufferSlice(std::span<T> buffer)
    : buffer_{buffer}, insertion_iterator_{buffer_.begin()}, size_{0} {}

template <class T>
constexpr auto UnownedBufferSlice<T>::Push(const T& item) -> void {
  *insertion_iterator_ = item;
  ++insertion_iterator_;
  if (insertion_iterator_ == buffer_.end()) {
    insertion_iterator_ = buffer_.begin();
  }
  size_ = std::min(size_ + 1, Capacity());
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Push(T&& item) -> void {
  *insertion_iterator_ = std::move(item);
  ++insertion_iterator_;
  if (insertion_iterator_ == buffer_.end()) {
    insertion_iterator_ = buffer_.begin();
  }
  size_ = std::min(size_ + 1, Capacity());
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Clear() -> void {
  insertion_iterator_ = buffer_.begin();
  size_ = 0;
}

template <class T>
auto UnownedBufferSlice<T>::ContiguousMemoryChunks()
    -> std::vector<std::span<T>> {
  if (Empty()) return {};
  const auto begin = buffer_.begin();
  const auto end = buffer_.end();
  if (!Full() || insertion_iterator_ == begin) return {{&(*begin), Size()}};
  const std::span<T> first_chunk{
      &(*insertion_iterator_),
      static_cast<SizeType>(std::distance(insertion_iterator_, end))};
  const std::span<T> second_chunk{
      &(*begin),
      static_cast<SizeType>(std::distance(begin, insertion_iterator_))};
  return {first_chunk, second_chunk};
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Size() const -> SizeType {
  return size_;
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Capacity() const -> SizeType {
  return buffer_.size();
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Empty() const -> bool {
  return Size() == 0;
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Full() const -> bool {
  return Capacity() <= Size();
}

template <class T>
constexpr auto UnownedBufferSlice<T>::WillWrapOnNextPush() const -> bool {
  return Capacity() == 0 || next(insertion_iterator_) == buffer_.end();
}

}  // namespace spoor::runtime::buffer

#endif
