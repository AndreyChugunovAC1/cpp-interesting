#include <cstddef>
#include <limits>

namespace auxiliary {
class size_state {
private:
  std::size_t data = 0;
  // 0 - static, 1 - dynamic

  static constexpr std::size_t STATE_MASK = static_cast<std::size_t>(1)
                                         << (std::numeric_limits<std::size_t>::digits - 1);

public:
  size_state() = default;
  size_state(const size_state&) = default;
  size_state(size_state&&) = default;
  size_state& operator=(const size_state&) = default;
  size_state& operator=(size_state&&) = default;
  ~size_state() = default;

private:
  std::size_t state_int() const {
    return data & STATE_MASK;
  }

public:
  void set_size(std::size_t new_size) {
    data = state_int() + new_size;
  }

  void make_dyamic() {
    data |= STATE_MASK;
  }

  void make_static() {
    data = size();
  }

  bool is_dynamic() const {
    return state_int();
  }

  bool is_static() const {
    return !is_dynamic();
  }

  std::size_t size() const {
    return data & ~STATE_MASK;
  }

  void reset() {
    data = 0;
  }

  size_state& operator++() {
    data++;
    return *this;
  }

  size_state& operator--() {
    data--;
    return *this;
  }
};
} // namespace auxiliary
