#pragma once

#include <type_traits>
#include <utility>

template <typename, typename, typename, typename>
class bimap;

namespace auxiliary {
class node_base { // extern template
public:
  node_base* parent = nullptr;
  node_base* left = nullptr;
  node_base* right = nullptr;

  node_base() = default;

  node_base(const node_base&) = delete;

  node_base(node_base&& other) {
    *this = std::move(other);
  }

  node_base& operator=(const node_base&) = delete;

  node_base& operator=(node_base&& other) {
    if (other.empty()) {
      make_empty();
    } else {
      assign(other);
      correct_sentinel();
      other.make_empty();
    }
    return *this;
  }

  ~node_base() = default;

  void assign(const node_base& other) {
    parent = other.parent;
    left = other.left;
    right = other.right;
  }

  void make_empty() {
    parent = left = right = this;
  }

  // pre: non empty
  void correct_sentinel() {
    left->parent = parent->left = this;
  }

  bool empty() const {
    return parent == this;
  }

  void update_parent(node_base* node) {
    if (parent->left == this) {
      parent->left = node;
    } else {
      parent->right = node;
    }
  }

  void link_left(node_base* node) {
    left = node;
    node->parent = this;
  }

  void link_right(node_base* node) {
    right = node;
    node->parent = this;
  }
};

class left_tag;
class right_tag;

// template <typename T>
// concept Side = std::is_same_v<T, left_tag> || std::is_same_v<T, right_tag>;

template <typename Tag>
class node_tagged : public node_base {}; // extern template !!!

class node_empty_mutual
    : public node_tagged<left_tag>
    , public node_tagged<right_tag> {
public:
  node_empty_mutual() = default;

  node_empty_mutual(node_empty_mutual&& other)
      : node_tagged<left_tag>(std::move(other))
      , node_tagged<right_tag>(std::move(other)) {}

  void make_empty() {
    node_tagged<left_tag>::make_empty();
    node_tagged<right_tag>::make_empty();
  }
};

template <typename T, typename Tag>
class node_element : public node_tagged<Tag> {
public:
  T element;

  template <typename U>
  // requires(!std::is_same_v<U, node_base>)
  node_element(U&& element)
      : element(std::forward<U>(element)) {}
};

template <typename L, typename R>
struct node_mutual
    : public node_element<L, left_tag>
    , public node_element<R, right_tag> {
  template <typename LF, typename RF>
  node_mutual(LF&& l, RF&& r)
      : node_element<L, left_tag>(std::forward<LF>(l))
      , node_element<R, right_tag>(std::forward<RF>(r)) {}
};

template <typename, typename, typename, typename, typename>
class map_basic;

template <typename, typename, typename>
class iterator_map;

/*** type traits for different-side classes ***/
template <typename T, typename OT, typename Tag>
struct bimap_traits;

template <typename T, typename OT>
struct bimap_traits<T, OT, left_tag> {
  using node_tagged_t = node_tagged<left_tag>;
  using node_element_t = node_element<T, left_tag>;
  using node_mutual_t = node_mutual<T, OT>;

  using other_node_tagged_t = node_tagged<right_tag>;
  using other_iterator = iterator_map<OT, T, right_tag>;
  using other_tag = right_tag;

  template <typename Cmp, typename OCmp>
  using other_map_t = map_basic<OT, T, OCmp, Cmp, right_tag>;

  template <typename Cmp, typename OCmp>
  using map_t = ::bimap<T, OT, Cmp, OCmp>;
};

template <typename T, typename OT>
struct bimap_traits<T, OT, right_tag> {
  using node_tagged_t = node_tagged<right_tag>;
  using node_element_t = node_element<T, right_tag>;
  using node_mutual_t = node_mutual<OT, T>;

  using other_node_tagged_t = node_tagged<left_tag>;
  using other_iterator = iterator_map<OT, T, left_tag>;
  using other_tag = left_tag;

  template <typename Cmp, typename OCmp>
  using other_map_t = map_basic<OT, T, OCmp, Cmp, left_tag>;

  template <typename Cmp, typename OCmp>
  using map_t = ::bimap<OT, T, OCmp, Cmp>;
};
} // namespace auxiliary
