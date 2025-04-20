#pragma once

#include "map-basic.h"

#include <cstddef>
#include <functional>
#include <stdexcept>

template <
    typename Left,
    typename Right,
    typename CompareLeft = std::less<Left>,
    typename CompareRight = std::less<Right>>
class bimap
    : private auxiliary::map_basic<Left, Right, CompareLeft, CompareRight, auxiliary::left_tag>
    , private auxiliary::map_basic<Right, Left, CompareRight, CompareLeft, auxiliary::right_tag> {
public:
  using left_t = Left;
  using right_t = Right;

private:
  // using map_prototype = auxiliary::map_prototype;
  using left_map_t = auxiliary::map_basic<left_t, right_t, CompareLeft, CompareRight, auxiliary::left_tag>;
  using right_map_t = auxiliary::map_basic<right_t, left_t, CompareRight, CompareLeft, auxiliary::right_tag>;

  template <typename, typename, typename, typename, typename>
  friend class auxiliary::map_basic;

  using node_left_t = auxiliary::node_tagged<auxiliary::left_tag>;
  using node_right_t = auxiliary::node_tagged<auxiliary::right_tag>;
  using node_mutual_t = auxiliary::node_mutual<left_t, right_t>;

public:
  using left_iterator = typename left_map_t::iterator;
  using right_iterator = typename right_map_t::iterator;

private:
  auxiliary::node_empty_mutual sentinel;
  std::size_t count = 0;

  left_map_t& as_left() {
    return static_cast<left_map_t&>(*this);
  }

  right_map_t& as_right() {
    return static_cast<right_map_t&>(*this);
  }

  const left_map_t& as_left() const {
    return static_cast<const left_map_t&>(*this);
  }

  const right_map_t& as_right() const {
    return static_cast<const right_map_t&>(*this);
  }

public:
  bimap(CompareLeft compare_left = CompareLeft(), CompareRight compare_right = CompareRight())
      : left_map_t(std::move(compare_left))
      , right_map_t(std::move(compare_right)) {
    sentinel.make_empty();
  }

  bimap(const bimap& other)
      : left_map_t(other.as_left())
      , right_map_t(other.as_right()) {
    sentinel.make_empty();

    try {
      for (auto oi = other.begin_left(); oi != other.end_left(); ++oi) {
        insert(*oi, *oi.flip());
      }
    } catch (...) {
      erase_left(begin_left(), end_left());
      throw;
    }
  }

  bimap(bimap&& other)
      : left_map_t(std::move(other))
      , right_map_t(std::move(other))
      , sentinel(std::move(other.sentinel)) {
    count = std::exchange(other.count, 0);
  }

  bimap& operator=(const bimap& other) {
    bimap(other).swap(*this);
    return *this;
  }

  bimap& operator=(bimap&& other) noexcept {
    bimap(std::move(other)).swap(*this);
    return *this;
  }

  ~bimap() {
    erase_left(begin_left(), end_left());
  }

  void swap(bimap& other) noexcept {
    left_map_t::swap(other);
    right_map_t::swap(other);
    std::swap(count, other.count);
  }

  friend void swap(bimap& lhs, bimap& rhs) noexcept {
    lhs.swap(rhs);
  }

private:
  template <typename T1, typename T2>
  left_iterator insert_template(T1&& left, T2&& right) {
    left_iterator il = lower_bound_left(left);
    right_iterator ir = lower_bound_right(right);

    if ((il != end_left() && !left_map_t::operator()(left, *il)) ||
        (ir != end_right() && !right_map_t::operator()(right, *ir))) {
      return end_left();
    }

    node_mutual_t* node = nullptr;
    // try {
    node = new node_mutual_t(std::forward<T1>(left), std::forward<T2>(right));
    // } catch (std::bad_alloc&) {
    //   throw;
    // } catch (...) {
    //   delete node;
    //   throw;
    // }

    count++;
    left_map_t::insert_by_lower_bound(il, static_cast<node_left_t*>(node));
    right_map_t::insert_by_lower_bound(ir, static_cast<node_right_t*>(node));
    return static_cast<node_left_t*>(node);
  }

public:
  left_iterator insert(const left_t& left, const right_t& right) {
    return insert_template<const left_t&, const right_t&>(left, right);
  }

  left_iterator insert(const left_t& left, right_t&& right) {
    return insert_template<const left_t&, right_t&&>(left, std::move(right));
  }

  left_iterator insert(left_t&& left, const right_t& right) {
    return insert_template<left_t&&, const right_t&>(std::move(left), right);
  }

  left_iterator insert(left_t&& left, right_t&& right) {
    return insert_template<left_t&&, right_t&&>(std::move(left), std::move(right));
  }

  left_iterator erase_left(left_iterator it) {
    return left_map_t::erase(it);
  }

  right_iterator erase_right(right_iterator it) {
    return right_map_t::erase(it);
  }

  bool erase_left(const left_t& left) {
    return left_map_t::erase(left);
  }

  bool erase_right(const right_t& right) {
    return right_map_t::erase(right);
  }

  left_iterator erase_left(left_iterator first, left_iterator last) {
    return left_map_t::erase(first, last);
  }

  right_iterator erase_right(right_iterator first, right_iterator last) {
    return right_map_t::erase(first, last);
  }

  left_iterator find_left(const left_t& left) const {
    return left_map_t::find(left);
  }

  right_iterator find_right(const right_t& right) const {
    return right_map_t::find(right);
  }

  const right_t& at_left(const left_t& key) const {
    return left_map_t::at(key);
  }

  const left_t& at_right(const right_t& key) const {
    return right_map_t::at(key);
  }

  const right_t& at_left_or_default(const left_t& key) {
    return left_map_t::at_or_default(key);
  }

  const left_t& at_right_or_default(const right_t& key) {
    return right_map_t::at_or_default(key);
  }

  left_iterator lower_bound_left(const left_t& left) const {
    return left_map_t::lower_bound(left);
  }

  left_iterator upper_bound_left(const left_t& left) const {
    return left_map_t::upper_bound(left);
  }

  right_iterator lower_bound_right(const right_t& right) const {
    return right_map_t::lower_bound(right);
  }

  right_iterator upper_bound_right(const right_t& right) const {
    return right_map_t::upper_bound(right);
  }

  left_iterator begin_left() const {
    return left_map_t::begin();
  }

  left_iterator end_left() const {
    return left_map_t::end();
  }

  right_iterator begin_right() const {
    return right_map_t::begin();
  }

  right_iterator end_right() const {
    return right_map_t::end();
  }

  bool empty() const {
    return size() == 0;
  }

  std::size_t size() const {
    return count;
  }

  friend bool operator==(const bimap& lhs, const bimap& rhs) {
    bool res = lhs.size() == rhs.size();

    for (auto it1 = lhs.begin_left(), it2 = rhs.begin_left(); res && it1 != lhs.end_left(); ++it1, ++it2) {
      res &= !lhs.as_left()(*it1, *it2) && !lhs.as_left()(*it2, *it1);
      res &= !lhs.as_right()(*it1.flip(), *it2.flip()) && !lhs.as_right()(*it2.flip(), *it1.flip());
    }
    return res;
  }

  friend bool operator!=(const bimap& lhs, const bimap& rhs) {
    return !(lhs == rhs);
  }
};
