#pragma once

#include <memory>
#include <optional>
#include <type_traits>

template <typename T>
class optional;

template <typename T>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<optional<T>> : std::true_type {};

struct nullopt_t {
  struct creator {};

  explicit constexpr nullopt_t(creator) noexcept {}
};

inline constexpr nullopt_t nullopt(nullopt_t::creator{});

struct in_place_t {
  explicit in_place_t() = default;
};

inline constexpr in_place_t in_place{};

namespace aux {
/* Union */
template <
    typename T,
    bool DefaultConstructability = std::is_trivially_default_constructible_v<T>,
    bool Destructability = std::is_trivially_destructible_v<T>>
union data {
  T elem;
};

template <typename T>
union data<T, true, false> {
  T elem;

  constexpr ~data() {}
};

template <typename T>
union data<T, false, true> {
  T elem;

  constexpr data() noexcept {}
};

template <typename T>
union data<T, false, false> {
  T elem;

  constexpr data() noexcept {}

  constexpr ~data() {}
};

enum class specific {
  TRIVIAL,
  DELETE,
  BASE
};

template <bool ExistChecker, bool TrivialChecker>
inline constexpr specific choose_specific() {
  if constexpr (!ExistChecker) {
    return specific::DELETE;
  } else if constexpr (TrivialChecker) {
    return specific::TRIVIAL;
  } else {
    return specific::BASE;
  }
}

template <typename T>
struct base {
protected:
  data<T> st;

  constexpr explicit base() = default;

  bool available = false;

  constexpr bool has_value() const noexcept {
    return available;
  }

  template <typename... Args>
  constexpr void construct_from(Args&&... args) {
    std::construct_at(operator->(), std::forward<Args>(args)...);
    available = true;
  }

  constexpr void reset() {
    if (available) {
      std::destroy_at(operator->());
      available = false;
    }
  }

  constexpr T& operator*() & noexcept {
    return st.elem;
  }

  constexpr const T& operator*() const& noexcept {
    return st.elem;
  }

  constexpr T&& operator*() && noexcept {
    return std::move(st.elem);
  }

  constexpr const T&& operator*() const&& noexcept {
    return std::move(st.elem);
  }

  constexpr T* operator->() noexcept {
    return std::addressof(st.elem);
  }

  constexpr const T* operator->() const noexcept {
    return std::addressof(st.elem);
  }

  template <typename U, typename Base>
  void assign(Base& other) {
    if (other.has_value()) {
      if (base<T>::has_value()) {
        **this = std::forward<U>(*other);
      } else {
        base<T>::construct_from(std::forward<U>(*other));
      }
    } else {
      base<T>::reset();
    }
  }
};

/* Destructability */
template <typename T, bool TriviallyDestructible = std::is_trivially_destructible_v<T>>
struct destructability : public base<T> {
  using base<T>::base;
};

template <typename T>
struct destructability<T, false> : public base<T> {
  using base<T>::base;

  constexpr destructability() = default;
  constexpr destructability(const destructability&) = default;
  constexpr destructability(destructability&&) = default;
  constexpr destructability& operator=(const destructability&) = default;
  constexpr destructability& operator=(destructability&&) = default;

  constexpr ~destructability() noexcept(std::is_nothrow_destructible_v<T>) {
    base<T>::reset();
  }
};

/* Copy constructability */
template <
    typename T,
    specific S = choose_specific<std::is_copy_constructible_v<T>, std::is_trivially_copy_constructible_v<T>>()>
struct copy_constructability : public destructability<T> {
  using destructability<T>::destructability;
};

template <typename T>
struct copy_constructability<T, specific::BASE> : public destructability<T> {
  using destructability<T>::destructability;

  constexpr copy_constructability() = default;
  constexpr copy_constructability(copy_constructability&&) = default;
  constexpr copy_constructability& operator=(const copy_constructability&) = default;
  constexpr copy_constructability& operator=(copy_constructability&&) = default;

  // clang-format off
  constexpr copy_constructability(const copy_constructability& other)
    noexcept(std::is_nothrow_copy_constructible_v<T>) {
    if (other.has_value()) {
      base<T>::construct_from(*other);
    }
  }

  // clang-format on
};

template <typename T>
struct copy_constructability<T, specific::DELETE> : public destructability<T> {
  using destructability<T>::destructability;

  constexpr copy_constructability() = default;
  constexpr copy_constructability(const copy_constructability& other) = delete;
  constexpr copy_constructability(copy_constructability&&) = default;
  constexpr copy_constructability& operator=(const copy_constructability&) = default;
  constexpr copy_constructability& operator=(copy_constructability&&) = default;
};

/* Move constructability */
template <
    typename T,
    specific S = choose_specific<std::is_move_constructible_v<T>, std::is_trivially_move_constructible_v<T>>()>
struct move_constructability : public copy_constructability<T> {
  using copy_constructability<T>::copy_constructability;
};

template <typename T>
struct move_constructability<T, specific::BASE> : public copy_constructability<T> {
  using copy_constructability<T>::copy_constructability;

  constexpr move_constructability() = default;
  constexpr move_constructability(const move_constructability& other) = default;
  constexpr move_constructability& operator=(const move_constructability&) = default;
  constexpr move_constructability& operator=(move_constructability&&) = default;

  constexpr move_constructability(move_constructability&& other) noexcept(std::is_nothrow_move_constructible_v<T>) {
    if (other.has_value()) {
      base<T>::construct_from(std::move(*other));
    }
  }
};

template <typename T>
struct move_constructability<T, specific::DELETE> : public copy_constructability<T> {
  using copy_constructability<T>::copy_constructability;

  constexpr move_constructability() = default;
  constexpr move_constructability(const move_constructability& other) = default;
  constexpr move_constructability& operator=(const move_constructability&) = default;
  constexpr move_constructability& operator=(move_constructability&&) = default;

  constexpr move_constructability(move_constructability&& other) = delete;
};

/* Copy assign */
template <
    typename T,
    specific S = choose_specific<
        std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>,
        std::is_trivially_copy_constructible_v<T> && std::is_trivially_copy_assignable_v<T> &&
            std::is_trivially_destructible_v<T>>()>
struct copy_assignability : public move_constructability<T> {
  using move_constructability<T>::move_constructability;
};

template <typename T>
struct copy_assignability<T, specific::BASE> : public move_constructability<T> {
  using move_constructability<T>::move_constructability;

  constexpr copy_assignability() = default;
  constexpr copy_assignability(const copy_assignability&) = default;
  constexpr copy_assignability(copy_assignability&&) = default;
  constexpr copy_assignability& operator=(copy_assignability&&) = default;

  constexpr copy_assignability& operator=(const copy_assignability& other
  ) noexcept(std::is_nothrow_copy_constructible_v<T> && std::is_nothrow_copy_assignable_v<T>) {
    base<T>::template assign<const T&>(other);
    return *this;
  }
};

template <typename T>
struct copy_assignability<T, specific::DELETE> : public move_constructability<T> {
  using move_constructability<T>::move_constructability;

  constexpr copy_assignability() = default;
  constexpr copy_assignability(const copy_assignability&) = default;
  constexpr copy_assignability(copy_assignability&&) = default;
  constexpr copy_assignability& operator=(const copy_assignability&) = delete;
  constexpr copy_assignability& operator=(copy_assignability&&) = default;
};

/* Move assign */
template <
    typename T,
    specific S = choose_specific<
        std::is_move_constructible_v<T> && std::is_move_assignable_v<T>,
        std::is_trivially_move_constructible_v<T> && std::is_trivially_move_assignable_v<T> &&
            std::is_trivially_destructible_v<T>>()>
struct move_assignability : copy_assignability<T> {
  using copy_assignability<T>::copy_assignability;
};

template <typename T>
struct move_assignability<T, specific::BASE> : public copy_assignability<T> {
  using copy_assignability<T>::copy_assignability;

  constexpr move_assignability() = default;
  constexpr move_assignability(const move_assignability&) = default;
  constexpr move_assignability(move_assignability&&) = default;
  constexpr move_assignability& operator=(const move_assignability&) = default;

  constexpr move_assignability& operator=(move_assignability&& other
  ) noexcept(std::is_nothrow_move_assignable_v<T> && std::is_nothrow_move_constructible_v<T>) {
    base<T>::template assign<T&&>(other);
    return *this;
  }
};

template <typename T>
struct move_assignability<T, specific::DELETE> : public copy_assignability<T> {
  using copy_assignability<T>::copy_assignability;

  constexpr move_assignability() = default;
  constexpr move_assignability(const move_assignability&) = default;
  constexpr move_assignability(move_assignability&&) = default;
  constexpr move_assignability& operator=(const move_assignability&) = default;
  constexpr move_assignability& operator=(move_assignability&&) = delete;
};

} // namespace aux
