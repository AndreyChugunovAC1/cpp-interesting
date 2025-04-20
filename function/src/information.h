#pragma once

#include <cstddef>
#include <exception>
#include <memory>
#include <new>
#include <utility>

class bad_function_call : public std::exception {
public:
  const char* what() const noexcept override {
    return "Bad function call";
  }
};

namespace auxiliary {
inline constexpr std::size_t SMALL_SIZE = sizeof(void*);

template <typename F>
concept Small =
    sizeof(F) <= SMALL_SIZE && alignof(std::max_align_t) % alignof(F) == 0 && std::is_nothrow_move_constructible_v<F>;

/*** Interface for runtime type information ***/
template <typename T>
class information;

template <typename R, typename... Args>
class information<R(Args...)> {
public:
  virtual R call(std::byte*, Args&&...) = 0;
  virtual void copy(std::byte* /* from */, std::byte* /* to */) = 0;
  virtual void move(std::byte* /* from */, std::byte* /* to */) = 0;
  virtual void destroy(std::byte*) = 0;
  virtual ~information() = default;
};

/*** Model declaration ***/
template <typename F, typename T>
class model;

/*** Model with pointer ***/
template <typename F, typename R, typename... Args>
class model<F, R(Args...)> : public information<R(Args...)> {
private:
  static F** as_ppf(std::byte* ptr) {
    return std::launder(reinterpret_cast<F**>(ptr));
  }

public:
  explicit model() = default;

  template <typename RF>
  static void construct(std::byte* where, RF&& func) {
    std::construct_at(reinterpret_cast<F**>(where), new F(std::forward<RF>(func)));
  }

  static F* get(std::byte* ptr) {
    return *as_ppf(ptr);
  }

  R call(std::byte* data, Args&&... args) override {
    return (**as_ppf(data))(std::forward<Args>(args)...);
  }

  void copy(std::byte* from, std::byte* to) override {
    std::construct_at(reinterpret_cast<F**>(to), new F(**as_ppf(from)));
  }

  void move(std::byte* from, std::byte* to) noexcept override {
    std::construct_at(reinterpret_cast<F**>(to), *as_ppf(from));
    *as_ppf(from) = nullptr;
  }

  void destroy(std::byte* where) noexcept override {
    delete *as_ppf(where);
  }
};

/*** Small object model ***/
template <Small F, typename R, typename... Args>
class model<F, R(Args...)> : public information<R(Args...)> {
private:
  static F* as_pf(std::byte* ptr) {
    return std::launder(reinterpret_cast<F*>(ptr));
  }

public:
  explicit model() = default;

  template <typename RF>
  static void construct(std::byte* where, RF&& func) {
    std::construct_at(reinterpret_cast<F*>(where), std::forward<RF>(func));
  }

  static F* get(std::byte* ptr) {
    return as_pf(ptr);
  }

  R call(std::byte* data, Args&&... args) override {
    return (*as_pf(data))(std::forward<Args>(args)...);
  }

  void copy(std::byte* from, std::byte* to) override {
    std::construct_at(reinterpret_cast<F*>(to), *as_pf(from));
  }

  void move(std::byte* from, std::byte* to) noexcept override {
    std::construct_at(reinterpret_cast<F*>(to), std::move(*as_pf(from)));
  }

  void destroy(std::byte* where) noexcept override {
    std::destroy_at(as_pf(where));
  }
};

/*** Empty model ***/
class empty_tag;

template <typename R, typename... Args>
class model<empty_tag, R(Args...)> : public information<R(Args...)> {
public:
  explicit model() = default;

  R call(std::byte*, Args&&...) override {
    throw bad_function_call();
  }

  virtual void copy(std::byte*, std::byte*) override {}

  virtual void move(std::byte*, std::byte*) override {}

  virtual void destroy(std::byte*) override {}
};
} // namespace auxiliary
