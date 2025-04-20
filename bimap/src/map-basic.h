#pragma once

#include "iterator-map.h"

#include <stdexcept>

namespace auxiliary {

template <typename T, typename OT, typename Cmp, typename OCmp, typename Tag>
class map_basic : public Cmp {
protected:
  using iterator = iterator_map<T, OT, Tag>;

private:
  using traits = bimap_traits<T, OT, Tag>;
  using other_map_t = typename traits::template other_map_t<Cmp, OCmp>;
  using map_t = typename traits::template map_t<Cmp, OCmp>;

  template <typename, typename, typename, typename, typename>
  friend class map_basic;

  // traits::node_tagged_t sentinel;

  node_base& sentinel() {
    return static_cast<node_base&>(static_cast<traits::node_tagged_t&>(as_base().sentinel));
  }

  const node_base& sentinel() const {
    return static_cast<const node_base&>(static_cast<const traits::node_tagged_t&>(as_base().sentinel));
  }

  static const T& as_elem(node_base* node) {
    return static_cast<traits::node_element_t*>(node)->element;
  }

  map_t& as_base() {
    return static_cast<map_t&>(*this);
  }

  const map_t& as_base() const {
    return static_cast<const map_t&>(*this);
  }

  other_map_t& as_other() {
    return as_base();
  }

protected:
  map_basic(Cmp comparator)
      : Cmp(std::move(comparator)) {
    // sentinel().make_empty();
  }

  map_basic(const map_basic& other)
      : Cmp(other) {
    // sentinel().make_empty();
  }

  map_basic(map_basic&& other) = default;

  map_basic& operator=(const map_basic& other) {
    static_cast<Cmp&>(*this) = static_cast<const Cmp&>(other);
    return *this;
  }

  map_basic& operator=(map_basic&&) = default;

  void swap(map_basic& other) {
    using std::swap;

    swap(static_cast<Cmp&>(*this), static_cast<Cmp&>(other));
    swap(sentinel(), other.sentinel());
  }

  /*** Iterators ***/
  iterator begin() const {
    return sentinel().parent;
  }

  iterator end() const {
    return const_cast<node_base*>(static_cast<const node_base*>(&sentinel()));
  }

  /*** Add element ***/
  // pre: no element in map
  void insert_by_lower_bound(iterator lb, traits::node_tagged_t* node) {
    node_base* cur = lb.ptr;

    if (cur == &sentinel() && sentinel().empty()) {
      sentinel().link_left(node);
      node->link_left(&sentinel());
      return;
    }

    if (cur->left == &sentinel()) {
      node->link_left(&sentinel());
      cur->link_left(node);
      return;
    }

    if (cur->left == nullptr) {
      cur->link_left(node);
      return;
    }

    (--lb).ptr->link_right(node);
  }

  /*** Delete element block ***/
  bool erase(const T& elem) {
    iterator it = find(elem);

    if (it != end()) {
      erase(it);
      return true;
    }
    return false;
  }

  iterator remove_node(iterator it) {
    node_base* cur = it.ptr;
    ++it;

    if (cur->left == nullptr && cur->right == nullptr) {
      cur->update_parent(nullptr);
    } else if (cur->right == nullptr) {
      cur->update_parent(cur->left);
      cur->left->parent = cur->parent;
    } else {
      if (cur->left != nullptr) {
        node_base* least_right = cur->right;

        while (least_right->left != nullptr) {
          least_right = least_right->left;
        }
        least_right->left = cur->left;
        cur->left->parent = least_right;
      }
      cur->update_parent(cur->right);
      cur->right->parent = cur->parent;
    }
    return it;
  }

  iterator erase(iterator it) {
    as_other().remove_node(it.flip());
    iterator res = remove_node(it);
    delete static_cast<traits::node_mutual_t*>(static_cast<traits::node_tagged_t*>(it.ptr));
    as_base().count--;

    return res;
  }

  iterator erase(iterator first, iterator last) {
    while (first != last) {
      first = erase(first);
    }
    return first;
  }

  /*** Access element block ***/
  const OT& at(const T& key) const {
    iterator it = find(key);

    if (it == end()) {
      throw std::out_of_range("No such element in bimap.");
    }
    return *it.flip();
  }

  const OT& at_or_default(const T& key)
    requires (!std::is_default_constructible_v<OT>)
  {
    return at(key);
  }

  const OT& at_or_default(const T& key)
    requires (std::is_default_constructible_v<OT>)
  {
    iterator it = lower_bound(key);

    if (it == end() || (*this)(key, *it)) {
      OT delem = OT();
      typename traits::other_iterator ir = as_other().lower_bound(delem);
      bool del = (ir == as_other().end() || as_other()(delem, *ir));

      typename traits::node_mutual_t* node;

      if constexpr (std::is_same_v<Tag, left_tag>) {
        node = new traits::node_mutual_t(key, std::move(delem));
      } else {
        node = new traits::node_mutual_t(std::move(delem), key);
      }
      if (del) {
        as_other().erase(ir++);
      }
      as_base().count++;
      insert_by_lower_bound(it, node);
      as_other().insert_by_lower_bound(ir, node);
      return *(--ir);
    }
    return *it.flip();
  }

  iterator find(const T& elem) const {
    iterator it = lower_bound(elem);

    if (it == end() || Cmp::operator()(elem, *it)) {
      return end();
    }
    return it;
  }

  template <typename BoundComparator>
  iterator bound(const T& key, const BoundComparator& cmp) const {
    node_base* cur = sentinel().left; // root
    node_base* potential = nullptr;

    while (cur != nullptr && cur != &sentinel()) {
      if (cmp(key, as_elem(cur))) {
        potential = cur;
        cur = cur->left;
      } else {
        cur = cur->right;
      }
    }
    return potential == nullptr ? end() : potential;
  }

  iterator lower_bound(const T& key) const {
    return bound(key, [this](const T& a, const T& b) -> bool { return !this->operator()(b, a); });
  }

  iterator upper_bound(const T& key) const {
    return bound(key, [this](const T& a, const T& b) -> bool { return this->operator()(a, b); });
  }

  ~map_basic() = default;
};
} // namespace auxiliary
