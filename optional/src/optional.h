#pragma once

#include "optional-specification.h"

#include <compare>

template <typename T>
class optional : protected aux::move_assignability<T> {
public:
  using aux::move_assignability<T>::move_assignability;
  // using move_assignability<T>::operator=;

  using value_type = T;

  using aux::base<T>::has_value;
  using aux::base<T>::operator*;
  using aux::base<T>::operator->;
  using aux::base<T>::reset;

  constexpr optional() noexcept {}

  constexpr optional(nullopt_t) noexcept {}

  constexpr optional(const optional&) = default;
  constexpr optional(optional&&) = default;

  constexpr optional& operator=(const optional&) = default;
  constexpr optional& operator=(optional&&) = default;

  // clang-format off
  template <
      typename U = T,
      typename = std::enable_if_t<
          std::is_constructible_v<T, U&&> && 
          !std::is_same_v<std::remove_cvref_t<U>, optional<T>> &&
          !std::is_same_v<std::remove_cvref_t<U>, in_place_t> &&
          !(std::is_same_v<std::remove_cv_t<T>, bool> && !is_optional<std::remove_cvref_t<U>>::value)>>
  constexpr explicit(!std::is_convertible_v<U&&, T>) optional(U&& value) noexcept(std::is_nothrow_constructible_v<T, U>)
      : optional(in_place, std::forward<U>(value)) {}

  template <
      typename U = T,
      typename = std::enable_if_t<
          !std::is_same_v<std::remove_cvref_t<U>, optional<T>> &&
          (!std::is_scalar_v<T> || !std::is_same_v<T, std::decay_t<U>>) &&
          std::is_constructible_v<T, U> && std::is_assignable_v<T&, U>>>
  constexpr optional& operator=(U&& value
  ) noexcept(std::is_nothrow_constructible_v<T, U> && std::is_nothrow_assignable_v<T&, U>) {
    if (has_value()) {
      **this = std::forward<U>(value);
    } else {
      emplace(std::forward<U>(value));
    }
    return *this;
  }

  // clang-format on

  template <typename... Args>
  explicit constexpr optional(in_place_t, Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    emplace(std::forward<Args>(args)...);
  }

  constexpr optional& operator=(nullopt_t) noexcept(std::is_nothrow_destructible_v<T>) {
    if (has_value()) {
      reset();
    }
    return *this;
  }

  constexpr void swap(optional& other) {
    if (has_value()) {
      if (other.has_value()) {
        using std::swap;
        swap(**this, *other);
        swap(this->available, other.available);
      } else {
        other.emplace(std::move(**this));
        reset();
      }
    } else if (other.has_value()) {
      other.swap(*this);
    }
  }

  constexpr explicit operator bool() const noexcept {
    return has_value();
  }

  template <typename... Args>
  constexpr T& emplace(Args&&... args) noexcept(std::is_nothrow_constructible_v<T, Args...>) {
    reset();
    aux::base<T>::construct_from(std::forward<Args>(args)...);
    return **this;
  }
};

/* Swap function */
template <typename T>
inline constexpr std::enable_if_t<std::is_move_constructible_v<T> && std::is_swappable_v<T>> swap(
    optional<T>& lhs,
    optional<T>& rhs
) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>) {
  lhs.swap(rhs);
}

template <typename T>
std::enable_if_t<!(std::is_move_constructible_v<T> && std::is_swappable_v<T>)>
swap(optional<T>&, optional<T>&) = delete;

// clang-format off
#define _cmp_by(_symbol_) ((bool(lhs) && bool(rhs)) ? (*lhs _symbol_ *rhs) : (bool(lhs) _symbol_ bool(rhs)))
// clang-format on

template <class T>
constexpr std::compare_three_way_result_t<T> operator<=>(const optional<T>& lhs, const optional<T>& rhs) {
  return _cmp_by(<=>);
}

template <typename T>
constexpr bool operator==(const optional<T>& lhs, const optional<T>& rhs) {
  return _cmp_by(==);
}

template <typename T>
constexpr bool operator!=(const optional<T>& lhs, const optional<T>& rhs) {
  return _cmp_by(!=);
}

template <typename T>
constexpr bool operator<(const optional<T>& lhs, const optional<T>& rhs) {
  return _cmp_by(<);
}

template <typename T>
constexpr bool operator<=(const optional<T>& lhs, const optional<T>& rhs) {
  return _cmp_by(<=);
}

template <typename T>
constexpr bool operator>(const optional<T>& lhs, const optional<T>& rhs) {
  return _cmp_by(>);
}

template <typename T>
constexpr bool operator>=(const optional<T>& lhs, const optional<T>& rhs) {
  return _cmp_by(>=);
}

#undef _cmp_by

template <typename T>
optional(T) -> optional<T>;
