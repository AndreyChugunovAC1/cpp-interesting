#pragma once

#include "nodes.h"

#include <iterator>

namespace auxiliary {
template <typename Tag>
using opposite_tag = std::conditional_t<std::is_same_v<Tag, left_tag>, right_tag, left_tag>;

template <typename T, typename OT, typename Tag>
class iterator_map {
  template <typename, typename, typename, typename>
  friend class ::bimap;

  template <typename, typename, typename, typename, typename>
  friend class map_basic;

public:
  using value_type = T;
  using const_pointer = const value_type*;
  using pointer = const_pointer;
  using const_reference = const value_type&;
  using reference = const_reference;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;

private:
  using traits = bimap_traits<T, OT, Tag>;

  using other_iterator = iterator_map<OT, T, typename traits::other_tag>;

  friend class iterator_map<OT, T, typename traits::other_tag>;

private:
  using node_element_t = node_element<value_type, Tag>;

  node_base* ptr;

  iterator_map(node_base* ptr)
      : ptr(ptr) {}

  // clang-format off
  void step(node_base* node_base::* side_prev, node_base* node_base::* side_next) {
    if (ptr->*side_next != nullptr) {
      ptr = ptr->*side_next;
      while (ptr->*side_prev != nullptr) {
        ptr = ptr->*side_prev;
      }
    } else {
      while (ptr->parent->*side_next == ptr) {
        ptr = ptr->parent;
      }
      ptr = ptr->parent;
    }
  }

  // clang-format on

public:
  iterator_map() = default;

  // rule of 0, but I wrote this comment

  const_reference operator*() const {
    return static_cast<traits::node_element_t*>(ptr)->element;
  }

  const_pointer operator->() const {
    return &static_cast<traits::node_element_t*>(ptr)->element;
  }

  iterator_map& operator++() {
    step(&node_base::left, &node_base::right); // aka from left to right
    return *this;
  }

  iterator_map operator++(int) {
    iterator_map prev = *this;
    ++*this;
    return prev;
  }

  iterator_map& operator--() {
    step(&node_base::right, &node_base::left); // aka from right to left
    return *this;
  }

  iterator_map operator--(int) {
    iterator_map prev = *this;

    --*this;
    return prev;
  }

  traits::other_iterator flip() const {
    if (ptr->right == ptr) {
      return static_cast<traits::other_node_tagged_t*>(
          static_cast<node_empty_mutual*>(static_cast<traits::node_tagged_t*>(ptr))
      );
    }
    return static_cast<traits::other_node_tagged_t*>(
        static_cast<traits::node_mutual_t*>(static_cast<traits::node_tagged_t*>(ptr))
    );
  }

  friend bool operator==(const iterator_map& left, const iterator_map& right) {
    return left.ptr == right.ptr;
  }
};
} // namespace auxiliary
