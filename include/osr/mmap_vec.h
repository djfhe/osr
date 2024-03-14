#pragma once

#include "cista/mmap.h"

#include "utl/verify.h"

#include "cista/strong.h"

namespace osr {

template <typename Key, typename T>
struct basic_mmap_vec {
  static_assert(std::is_trivially_copyable_v<T>);

  explicit basic_mmap_vec(cista::mmap mmap)
      : mmap_{std::move(mmap)}, size_{mmap_.size() / sizeof(T)} {}

  void push_back(T const& t) {
    ++size_;
    mmap_.resize(sizeof(T) * size_);
    (*this)[Key{size_ - 1U}] = t;
  }

  T& operator[](Key const i) {
    return *reinterpret_cast<T*>(mmap_.data() + sizeof(T) * cista::to_idx(i));
  }

  T const& operator[](Key const i) const {
    return *reinterpret_cast<T const*>(mmap_.data() +
                                       sizeof(T) * cista::to_idx(i));
  }

  std::size_t size() const { return size_; }

  void resize(std::size_t const size) {
    size_ = size;
    mmap_.resize(size);
  }

  cista::mmap mmap_;
  std::size_t size_{0U};
};

template <typename T>
using mmap_vec = basic_mmap_vec<std::size_t, T>;

template <typename Key, typename T>
using mmap_vec_map = basic_mmap_vec<Key, T>;

}  // namespace osr