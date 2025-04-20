#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace auxiliary {
struct control_block_base {
public:
  std::size_t count;
  std::size_t weak_count;

protected:
  virtual void destroy() = 0;

public:
  control_block_base() noexcept
      : count(1)
      , weak_count(0) {}

  void strong_increment() noexcept {
    ++count;
  }

  void weak_increment() noexcept {
    ++weak_count;
  }

  void strong_decrement() noexcept {
    if (--count == 0) {
      destroy();
      if (weak_count == 0) {
        delete this;
      }
    }
  }

  void weak_decrement() noexcept {
    if (--weak_count == 0 && count == 0) {
      delete this;
    }
  }

  virtual ~control_block_base() = default;
};

template <typename T, typename Deleter = std::default_delete<T>>
struct control_block_ref final : public control_block_base {
  T* ptr;
#ifdef _MSC_VER
  [[msvc::no_unique_address]]
#else
  [[no_unique_address]]
#endif
  Deleter deleter;

  control_block_ref() = default;

  control_block_ref(T* ptr, Deleter deleter) noexcept
      : ptr(ptr)
      , deleter(std::move(deleter)) {}

  void destroy() final {
    deleter(std::exchange(ptr, nullptr));
  }

  ~control_block_ref() final = default;
};

template <typename T>
struct control_block_obj final : public control_block_base {
  union {
    T obj;
  };

  control_block_obj() = default;

  template <typename... Args>
  control_block_obj(Args&&... args)
      : obj(std::forward<Args>(args)...) {}

  void destroy() final {
    std::destroy_at(&obj);
  }

  ~control_block_obj() final {}
};

template <class From, class To>
concept pointer_convertible_to = std::is_convertible_v<From*, To*>;
} // namespace auxiliary

template <typename T>
class shared_ptr {
private:
  T* ptr;
  auxiliary::control_block_base* control_block;

  template <typename>
  friend class shared_ptr;

  template <typename>
  friend class weak_ptr;

  template <typename Y, typename... Args>
  friend shared_ptr<Y> make_shared(Args&&... args);

  shared_ptr(T* ptr, auxiliary::control_block_obj<T>* control_block)
      : ptr(ptr)
      , control_block(control_block) {}

  template <auxiliary::pointer_convertible_to<T> Y>
  shared_ptr(Y* ptr, auxiliary::control_block_base* control_block)
      : ptr(ptr)
      , control_block(control_block) {
    if (control_block) {
      control_block->strong_increment();
    }
  }

public:
  shared_ptr() noexcept
      : ptr(nullptr)
      , control_block(nullptr) {}

  shared_ptr(std::nullptr_t) noexcept
      : shared_ptr() {}

  template <auxiliary::pointer_convertible_to<T> Y>
  explicit shared_ptr(Y* ptr)
      : shared_ptr(ptr, std::default_delete<Y>()) {}

  template <auxiliary::pointer_convertible_to<T> Y, typename Deleter>
  shared_ptr(Y* ptr, Deleter deleter)
      : ptr(ptr) {
    try {
      control_block = new auxiliary::control_block_ref(ptr, std::move(deleter));
    } catch (...) {
      deleter(ptr);
      throw;
    }
  }

  template <typename Y>
  shared_ptr(const shared_ptr<Y>& other, T* ptr) noexcept
      : shared_ptr(ptr, other.control_block) {}

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other, T* ptr) noexcept
      : ptr(ptr)
      , control_block(std::exchange(other.control_block, nullptr)) {
    other.ptr = nullptr;
  }

  shared_ptr(const shared_ptr& other) noexcept
      : shared_ptr(other.ptr, other.control_block) {}

  template <auxiliary::pointer_convertible_to<T> Y>
  shared_ptr(const shared_ptr<Y>& other) noexcept
      : shared_ptr(other.ptr, other.control_block) {}

  shared_ptr(shared_ptr&& other) noexcept
      : ptr(std::exchange(other.ptr, nullptr))
      , control_block(std::exchange(other.control_block, nullptr)) {}

  template <auxiliary::pointer_convertible_to<T> Y>
  shared_ptr(shared_ptr<Y>&& other) noexcept
      : ptr(std::exchange(other.ptr, nullptr))
      , control_block(std::exchange(other.control_block, nullptr)) {}

  shared_ptr& operator=(const shared_ptr& other) noexcept {
    if (this != &other) {
      shared_ptr(other).swap(*this);
    }
    return *this;
  }

  template <auxiliary::pointer_convertible_to<T> Y>
  shared_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    shared_ptr(other).swap(*this);
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    if (this != &other) {
      shared_ptr(std::move(other)).swap(*this);
    }
    return *this;
  }

  template <auxiliary::pointer_convertible_to<T> Y>
  shared_ptr& operator=(shared_ptr<Y>&& other) noexcept {
    shared_ptr(std::move(other)).swap(*this);
    return *this;
  }

  T* get() const noexcept {
    return ptr;
  }

  explicit operator bool() const noexcept {
    return static_cast<bool>(get());
  }

  T& operator*() const noexcept {
    return *get();
  }

  T* operator->() const noexcept {
    return get();
  }

  std::size_t use_count() const noexcept {
    return control_block ? control_block->count : 0;
  }

  void swap(shared_ptr& other) {
    std::swap(ptr, other.ptr);
    std::swap(control_block, other.control_block);
  }

  void reset() noexcept {
    shared_ptr().swap(*this);
  }

  template <auxiliary::pointer_convertible_to<T> Y>
  void reset(Y* new_ptr) {
    shared_ptr(new_ptr).swap(*this);
  }

  template <auxiliary::pointer_convertible_to<T> Y, typename Deleter>
  void reset(Y* new_ptr, Deleter deleter) {
    shared_ptr(new_ptr, std::move(deleter)).swap(*this);
  }

  friend bool operator==(const shared_ptr& lhs, const shared_ptr& rhs) noexcept {
    return lhs.get() == rhs.get();
  }

  friend bool operator!=(const shared_ptr& lhs, const shared_ptr& rhs) noexcept {
    return lhs.get() != rhs.get();
  }

  ~shared_ptr() {
    if (control_block) {
      control_block->strong_decrement();
    }
  }
};

template <typename T>
class weak_ptr {
private:
  T* ptr;
  auxiliary::control_block_base* control_block;

  template <typename>
  friend class weak_ptr;

  template <auxiliary::pointer_convertible_to<T> Y>
  weak_ptr(Y* ptr, auxiliary::control_block_base* control_block) noexcept
      : ptr(ptr)
      , control_block(control_block) {
    if (control_block) {
      control_block->weak_increment();
    }
  }

public:
  weak_ptr() noexcept
      : ptr(nullptr)
      , control_block(nullptr) {}

  weak_ptr(const weak_ptr& other) noexcept
      : weak_ptr(other.ptr, other.control_block) {}

  template <auxiliary::pointer_convertible_to<T> Y>
  weak_ptr(const shared_ptr<Y>& other) noexcept
      : weak_ptr(other.ptr, other.control_block) {}

  template <auxiliary::pointer_convertible_to<T> Y>
  weak_ptr(const weak_ptr<Y>& other) noexcept
      : weak_ptr(other.ptr, other.control_block) {}

  weak_ptr(weak_ptr&& other) noexcept
      : ptr(std::exchange(other.ptr, nullptr))
      , control_block(std::exchange(other.control_block, nullptr)) {}

  template <auxiliary::pointer_convertible_to<T> Y>
  weak_ptr(weak_ptr<Y>&& other) noexcept
      : ptr(std::exchange(other.ptr, nullptr))
      , control_block(std::exchange(other.control_block, nullptr)) {}

  void swap(weak_ptr& other) noexcept {
    std::swap(ptr, other.ptr);
    std::swap(control_block, other.control_block);
  }

  template <auxiliary::pointer_convertible_to<T> Y>
  weak_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(const weak_ptr& other) noexcept {
    if (this != &other) {
      weak_ptr(other).swap(*this);
    }
    return *this;
  }

  template <auxiliary::pointer_convertible_to<T> Y>
  weak_ptr& operator=(const weak_ptr<Y>& other) noexcept {
    weak_ptr(other).swap(*this);
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    if (this != &other) {
      weak_ptr(std::move(other)).swap(*this);
    }
    return *this;
  }

  template <auxiliary::pointer_convertible_to<T> Y>
  weak_ptr& operator=(weak_ptr<Y>&& other) noexcept {
    weak_ptr(std::move(other)).swap(*this);
    return *this;
  }

  bool expired() const noexcept {
    return control_block ? control_block->count == 0 : true;
  }

  shared_ptr<T> lock() const noexcept {
    return expired() ? shared_ptr<T>() : shared_ptr<T>(ptr, control_block);
  }

  void reset() noexcept {
    weak_ptr().swap(*this);
  }

  ~weak_ptr() {
    if (control_block) {
      control_block->weak_decrement();
    }
  }
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  auto* cb = new auxiliary::control_block_obj<T>(std::forward<Args>(args)...);

  return shared_ptr<T>(&cb->obj, cb);
}
