#pragma once

#include "information.h"

template <typename F>
class function;

template <typename R, typename... Args>
class function<R(Args...)> {
private:
  using information = auxiliary::information<R(Args...)>;
  using empty_tag = auxiliary::empty_tag;

  template <typename F>
  using model = auxiliary::model<F, R(Args...)>;

private:
  alignas(std::max_align_t) std::byte data[auxiliary::SMALL_SIZE];
  information* info;

  template <typename F>
  static information* pointer_to_model() {
    static model<F> instance;
    return &instance;
  }

public:
  function() noexcept
      : info(pointer_to_model<empty_tag>()) {}

  template <typename F>
    requires (!std::is_same_v<std::decay_t<F>, function>)
  function(F&& func)
      : info(pointer_to_model<std::decay_t<F>>()) {
    model<std::decay_t<F>>::construct(data, std::forward<F>(func));
  }

  function(const function& other)
      : info(other.info) {
    info->copy(other.data, data);
  }

  function(function&& other) noexcept
      : info(std::exchange(other.info, pointer_to_model<empty_tag>())) {
    info->move(other.data, data);
    info->destroy(other.data);
  }

  function& operator=(const function& other) {
    if (this != &other) {
      *this = function(other);
    }
    return *this;
  }

  function& operator=(function&& other) noexcept {
    if (this != &other) {
      info->destroy(data);
      other.info->move(other.data, data);
      other.info->destroy(other.data);
      info = std::exchange(other.info, pointer_to_model<empty_tag>());
    }
    return *this;
  }

  ~function() {
    info->destroy(data);
  }

  explicit operator bool() const noexcept {
    return info != pointer_to_model<empty_tag>();
  }

  R operator()(Args... args) const {
    return info->call(data, std::forward<Args>(args)...);
  }

  template <typename T>
  T* target() noexcept {
    return const_cast<T*>(std::as_const(*this).template target<T>());
  }

  template <typename T>
  const T* target() const noexcept {
    return info == pointer_to_model<T>() ? model<T>::get(data) : nullptr;
  }
};
