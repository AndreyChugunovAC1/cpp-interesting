#pragma once

#include "size-state.h"

#include <algorithm>
#include <memory>
#include <utility>

template <typename T, std::size_t SMALL_SIZE>
  requires (SMALL_SIZE > 0)
class socow_vector {
public:
  using value_type = T;
  using iterator = T*;
  using const_iterator = const T*;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

private:
  struct buffer {
    std::size_t capacity;
    std::size_t share_count;
    value_type data[0];

    [[nodiscard]] static buffer* allocate(std::size_t capacity) {
      buffer* buf = static_cast<buffer*>(operator new(sizeof(buffer) + capacity * sizeof(value_type)));

      buf->capacity = capacity;
      buf->share_count = 1;
      return buf;
    }

    static void deallocate(buffer* buf) noexcept {
      operator delete(static_cast<void*>(buf));
    }

    buffer(const buffer& other) = delete;
    buffer(buffer&& other) = delete;
    buffer& operator=(const buffer& other) = delete;
    buffer& operator=(buffer&& other) = delete;
    ~buffer() = default;
  };

  auxiliary::size_state info;

  using static_buffer_t = T[SMALL_SIZE];
  using dynamic_buffer_t = buffer*;

  union {
    static_buffer_t sbuf;
    dynamic_buffer_t dbuf;
  };

  struct buffer_safe_pointer {
    std::size_t size = 0;
    buffer* buf = nullptr;

    buffer_safe_pointer() {}

    buffer_safe_pointer(buffer* buf)
      : buf(buf) {}

    buffer_safe_pointer(const buffer_safe_pointer&) = delete;
    buffer_safe_pointer& operator=(const buffer_safe_pointer&) = delete;

    buffer* release() {
      size = 0;
      return std::exchange(buf, nullptr);
    }

    buffer* operator->() const {
      return buf;
    }

    ~buffer_safe_pointer() {
      if (buf != nullptr) {
        std::destroy_n(buf->data, size);
        buffer::deallocate(buf);
      }
    }
  };

public:
  socow_vector()
    : info() {}

  socow_vector(const socow_vector& other)
    : info(other.info) {
    if (other.info.is_static()) {
      std::uninitialized_copy_n(other.sbuf, other.size(), sbuf);
    } else {
      dbuf = other.dbuf;
      dbuf->share_count++;
    }
  }

  socow_vector(socow_vector&& other) noexcept
    : info(other.info) {
    if (other.info.is_static()) {
      std::uninitialized_move_n(other.sbuf, other.size(), sbuf);
    } else {
      dbuf = other.dbuf;
      dbuf->share_count++;
    }
    other.clear();
  }

  socow_vector& operator=(const socow_vector& other) & {
    if (this != &other) {
      socow_vector tmp(other);

      clear();
      swap(tmp);
    }
    return *this;
  }

  socow_vector& operator=(socow_vector&& other) & noexcept {
    if (this != &other) {
      clear();
      swap(other);
    }
    return *this;
  }

private:
  static void
  swap_raw_arrays(pointer small_arr, pointer big_arr, std::size_t swap_size, std::size_t full_size) noexcept {
    std::swap_ranges(small_arr, small_arr + swap_size, big_arr);
    std::uninitialized_move_n(big_arr + swap_size, full_size - swap_size, small_arr + swap_size);
    std::destroy_n(big_arr + swap_size, full_size - swap_size);
  }

public:
  void swap(socow_vector& other) noexcept {
    if (this == &other) {
      return;
    }

    if (info.is_static()) {
      if (other.info.is_static()) {
        if (size() <= other.size()) { // case 1
          swap_raw_arrays(sbuf, other.sbuf, size(), other.size());
          std::swap(info, other.info);
        } else { // also case 1
          other.swap(*this);
        }
      } else { // case 2
        dynamic_buffer_t tmp = other.dbuf;

        swap_raw_arrays(other.sbuf, sbuf, 0, size());
        dbuf = tmp;
        std::swap(info, other.info);
      }
    } else {
      if (other.info.is_static()) { // also case 2
        other.swap(*this);
      } else {
        std::swap(dbuf, other.dbuf);
        std::swap(info, other.info);
      }
    }
  }

  friend void swap(socow_vector& left, socow_vector& right) noexcept {
    left.swap(right);
  }

  std::size_t size() const noexcept {
    return info.size();
  }

  std::size_t capacity() const noexcept {
    return info.is_static() ? SMALL_SIZE : dbuf->capacity;
  }

  bool empty() const noexcept {
    return size() == 0;
  }

private:
  bool is_shared() const noexcept {
    return info.is_dynamic() && dbuf->share_count > 1;
  }

public:
  /*** Iterators ***/
  iterator begin() {
    if (info.is_static()) {
      return sbuf;
    }
    if (is_shared()) {
      forced_update_to_capacity(capacity());
    }
    return dbuf->data;
  }

  const_iterator begin() const noexcept {
    if (info.is_static()) {
      return sbuf;
    }
    return dbuf->data;
  }

  pointer data() {
    return begin();
  }

  const_pointer data() const noexcept {
    return begin();
  }

  iterator end() {
    return begin() + size();
  }

  const_iterator end() const noexcept {
    return begin() + size();
  }

  reference operator[](std::size_t index) {
    return *(begin() + index);
  }

  const_reference operator[](std::size_t index) const noexcept {
    return *(begin() + index);
  }

  reference front() {
    return *begin();
  }

  const_reference front() const noexcept {
    return *begin();
  }

  reference back() {
    return *(end() - 1);
  }

  const_reference back() const noexcept {
    return *(end() - 1);
  }

  /*** Iterators ***/

public:
  void reserve(std::size_t new_capacity) /* strong */ {
    if ((is_shared() && new_capacity >= size()) || new_capacity > capacity()) {
      forced_update_to_capacity(new_capacity);
    }
  }

  /*** Element adding methods ***/
  void push_back(T&& elem) /* strong */ {
    insert(std::as_const(*this).end(), std::move(elem));
  }

  void push_back(const T& elem) /* strong */ {
    insert(std::as_const(*this).end(), elem);
  }

private:
  iterator unchanging_begin() {
    if (info.is_static()) {
      return sbuf;
    }
    return dbuf->data;
  }

  template <typename V>
  void insert_swapping(const_iterator pos, V&& elem) {
    iterator cur = end();

    std::construct_at(cur, std::forward<V>(elem));
    while (cur-- != pos) {
      std::iter_swap(cur, cur + 1);
    }
  }

  template <typename V>
  void insert_moving(std::size_t ind, V&& elem) {
    buffer_safe_pointer tmp(buffer::allocate(capacity() == size() ? 2 * capacity() : capacity()));

    std::construct_at(tmp->data + ind, std::forward<V>(elem));
    std::uninitialized_move_n(unchanging_begin(), ind, tmp->data);
    std::uninitialized_move_n(unchanging_begin() + ind, size() - ind, tmp->data + ind + 1);
    std::destroy_n(unchanging_begin(), size());
    if (info.is_dynamic()) {
      buffer::deallocate(dbuf);
    } else {
      info.make_dyamic();
    }
    dbuf = tmp.release();
  }

public:
  iterator insert(const_iterator pos, value_type&& elem) /* strong */ {
    std::size_t ind = pos - std::as_const(*this).begin();
    bool expand = size() == capacity();

    if (is_shared()) {
      buffer_safe_pointer tmp(buffer::allocate(expand ? 2 * capacity() : capacity()));

      std::construct_at(tmp->data + ind, std::move(elem));
      try {
        std::uninitialized_copy_n(dbuf->data, ind, tmp->data);
        tmp.size += ind;
        std::uninitialized_copy_n(dbuf->data + ind, size() - ind, tmp->data + ind + 1);
      } catch (...) {
        elem = std::move(tmp->data[ind]);
        std::destroy_at(tmp->data + ind);
        throw;
      }
      detach_buffer(dbuf);
      dbuf = tmp.release();
    } else if (expand) {
      insert_moving(ind, std::move(elem));
    } else {
      insert_swapping(pos, std::move(elem));
    }
    ++info;
    return begin() + ind;
  }

  iterator insert(const_iterator pos, const value_type& elem) /* strong */ {
    std::size_t ind = pos - std::as_const(*this).begin();
    bool expand = size() == capacity();

    if (is_shared()) {
      buffer_safe_pointer tmp(buffer::allocate(expand ? 2 * capacity() : capacity()));

      std::construct_at(tmp->data + ind, elem);
      std::uninitialized_copy_n(dbuf->data, ind, tmp->data);
      tmp.size += ind + 1;
      std::uninitialized_copy_n(dbuf->data + ind, size() - ind, tmp->data + ind + 1);
      detach_buffer(dbuf);
      dbuf = tmp.release();
    } else if (expand) {
      insert_moving(ind, elem);
    } else {
      insert_swapping(pos, elem);
    }
    ++info;
    return begin() + ind;
  }

  /*** End of element adding methods ***/

  /*** Element delete methods ***/
  void pop_back() /* strong */ {
    erase(std::as_const(*this).end() - 1);
  }

private:
  // pre: size() is correct, no nullptrs
  void detach_buffer(dynamic_buffer_t buf) noexcept {
    if (--buf->share_count == 0) {
      std::destroy_n(buf->data, size());
      buffer::deallocate(buf);
    }
  }

private:
  void migrate_to_dynamic(std::size_t new_capacity) {
    buffer* tmp = buffer::allocate(new_capacity);

    std::uninitialized_move_n(sbuf, size(), tmp->data);
    std::destroy_n(sbuf, size());
    dbuf = tmp;
    info.make_dyamic();
  }

  void migrate_to_static() {
    dynamic_buffer_t tmp = dbuf;

    if (tmp->share_count > 1) {
      try {
        std::uninitialized_copy_n(tmp->data, size(), sbuf); // may throw
      } catch (...) {
        dbuf = tmp;
        throw;
      }
    } else {
      std::uninitialized_move_n(tmp->data, size(), sbuf);
    }
    detach_buffer(tmp);
    info.make_static();
  }

  void expand_dynamic_buffer(std::size_t new_capacity) {
    buffer_safe_pointer tmp(buffer::allocate(new_capacity));

    if (dbuf->share_count > 1) {
      std::uninitialized_copy_n(dbuf->data, size(), tmp->data); // may throw
    } else {
      std::uninitialized_move_n(dbuf->data, size(), tmp->data);
    }
    detach_buffer(dbuf);
    dbuf = tmp.release();
  }

  void forced_update_to_capacity(std::size_t new_capacity) /* strong */ {
    if (info.is_static()) {
      if (new_capacity > SMALL_SIZE) {
        migrate_to_dynamic(new_capacity);
      }
    } else if (new_capacity > SMALL_SIZE) {
      expand_dynamic_buffer(new_capacity);
    } else {
      migrate_to_static();
    }
  }

public:
  iterator erase(const_iterator first, const_iterator last) /* strong */ {
    std::size_t indf = first - std::as_const(*this).begin();
    std::size_t indl = last - std::as_const(*this).begin();

    if (indf >= indl) {
      return begin() + indl; // may unshare
    }

    std::size_t new_size = size() - (indl - indf);

    if (is_shared()) {
      buffer_safe_pointer tmp(buffer::allocate(capacity()));

      std::uninitialized_copy_n(dbuf->data, indf, tmp->data);
      tmp.size += indf;
      std::uninitialized_copy_n(dbuf->data + indl, size() - indl, tmp->data + indf);
      detach_buffer(dbuf);
      dbuf = tmp.release();
    } else {
      iterator first_nc = begin() + indf, last_nc = begin() + indl;

      while (last_nc != end()) {
        std::iter_swap(first_nc++, last_nc++);
      }
      std::destroy(first_nc, last_nc);
    }
    info.set_size(new_size);
    return begin() + indf;
  }

  iterator erase(const_iterator pos) /* strong */ {
    return erase(pos, pos + 1);
  }

  /*** End of element delete methods ***/

  void shrink_to_fit() {
    if (size() < capacity()) {
      forced_update_to_capacity(size());
    }
  }

  /*** Cleanup methods ***/
  void clear() noexcept {
    if (is_shared()) {
      dbuf->share_count--;
      info.reset();
    } else {
      std::destroy_n(begin(), size());
      info.set_size(0);
    }
  }

private:
  void destruct() noexcept {
    if (is_shared()) {
      dbuf->share_count--;
    } else {
      std::destroy_n(begin(), size());
      if (info.is_dynamic()) {
        buffer::deallocate(dbuf);
      }
    }
  }

public:
  ~socow_vector() noexcept {
    destruct();
  }

  /*** End of cleanup methods ***/
};
