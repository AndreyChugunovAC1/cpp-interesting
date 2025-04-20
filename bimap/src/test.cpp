#ifdef __DEFINETELY_UNDEFINED_MACRO
#define _main main
#endif

#include "test.h"

#include "bimap.h"

#include <random>
#include <vector>

namespace view {
template <typename Cnt>
static void container(const Cnt& cb) {
  for (auto i : cb) {
    std::cout << i << " ";
  }
  std::cout << "\n";
}

template <typename B>
static void bimap(const B& bm) {
  std::cout << "lefts: ";
  for (auto it = bm.begin_left(); it != bm.end_left(); ++it) {
    std::cout << *it << ' ';
  }
  std::putc('\n', stdout);

  std::cout << "rights: ";
  for (auto it = bm.begin_right(); it != bm.end_right(); ++it) {
    std::cout << *it << ' ';
  }
  std::putc('\n', stdout);
}
} // namespace view

void test_static() {
  using bmp = bimap<long long, double>;
  using node_l = auxiliary::node_tagged<auxiliary::left_tag>;
  using node_r = auxiliary::node_tagged<auxiliary::right_tag>;
  // std::size_t sz = sizeof(node_l) + sizeof(node_r) + sizeof(std::size_t);
  _check(sizeof(node_l) + sizeof(node_r) + sizeof(std::size_t) == sizeof(bmp));
}

void test_empty() {
  using bmp = bimap<int, float>;

  bmp t;
  _check(t.empty());
  _check(t.size() == 0);
  _check(t.begin_left() == t.end_left());
  _check(t.begin_right() == t.end_right());
}

template <typename Bimap>
void check_empty_tree(const Bimap& t) {
  _check(t.begin_left() == t.end_left());
  _check(t.begin_right() == t.end_right());
  _check(t.size() == 0);
  _check(t.empty());
}

void test_move() {
  using bmp = bimap<int, float>;
  bmp t;
  bmp v = std::move(t);
  check_empty_tree(t);
  check_empty_tree(v);

  t.insert(1, 7.0f);
  view::bimap(t);
  t.insert(5, 2.0f);
  view::bimap(t);
  t.insert(6, 3.0f);
  view::bimap(t);
  t.insert(3, 9.0f);
  view::bimap(t);
  t.insert(2, 1.0f);
  view::bimap(t);
  t.insert(4, 0.0f);
  view::bimap(t);

  std::vector<int> lefts;
  std::vector<float> rights;

  for (auto it = t.begin_left(); it != t.end_left(); ++it) {
    lefts.push_back(*it);
  }
  for (auto it = t.begin_right(); it != t.end_right(); ++it) {
    rights.push_back(*it);
  }

  bmp tm = std::move(t);

  view::container(lefts);
  view::container(rights);
  _check(std::equal(tm.begin_left(), tm.end_left(), lefts.begin()));
  _check(std::equal(tm.begin_right(), tm.end_right(), rights.begin()));
  _check(t.empty());
  _check(t.begin_left() == t.end_left());
  _check(t.begin_right() == t.end_right());
}

void test_find() {
  using bmp = bimap<int, float>;
  bmp t;

  t.insert(1, 7.0f);
  t.insert(5, 2.0f);
  t.insert(16, 3.0f);
  t.insert(3, 9.0f);
  t.insert(12, 1.0f);
  t.insert(4, 0.0f);

  _msg("left");
  for (int i : {1, 5, 16, 3, 12, 4}) {
    _check(*t.find_left(i) == i);
  }
  for (int i : {0, 7, -1, 6, 2}) {
    _check(t.find_left(i) == t.end_left());
  }

  _msg("right");
  for (float i : {7.0f, 2.0f, 3.0f, 9.0f, 1.0f, 0.0f}) {
    _check(*t.find_right(i) == i);
  }
  for (float i : {7.5f, 2.5f, 3.5f, 9.5f, 1.5f, 0.5f, -1.0f}) {
    _check(t.find_right(i) == t.end_right());
  }
}

void test_at() {
  using bmp = bimap<int, float>;
  bmp t;

  t.insert(1, 7.0f);
  t.insert(5, 2.0f);
  t.insert(16, 3.0f);
  t.insert(3, 9.0f);
  t.insert(12, 1.0f);
  t.insert(4, 0.0f);
  _msg("left");
  std::vector<std::pair<int, float>> mapl = {{1, 7.0f}, {5, 2.0f}, {16, 3.0f}, {3, 9.0f}, {12, 1.0f}, {4, 0.0f}};
  for (auto [key, val] : mapl) {
    _check(t.at_left(key) == val);
  }

  for (int key : {-1, 2, 6, 10, 11, -5}) {
    try {
      int val = t.at_left(key);
      _view(val);
      _check(false);
    } catch (const std::out_of_range& e) {
      _check(true);
    }
  }

  _msg("right");
  std::vector<std::pair<float, int>> mapr = {{7.0f, 1}, {2.0f, 5}, {3.0f, 16}, {9.0f, 3}, {1.0f, 12}, {0.0f, 4}};
  for (auto [key, val] : mapr) {
    _check(t.at_right(key) == val);
  }

  for (float key : {7.5f, 2.5f, 3.5f, 9.5f, 1.5f, 0.5f, -1.0f}) {
    try {
      int val = t.at_right(key);
      _view(val);
      _check(false);
    } catch (const std::out_of_range& e) {
      _check(true);
    }
  }
}

void test_at_or_default() {
  using bmp = bimap<int, float>;
  bmp t;

  t.insert(1, 7.0f);
  t.insert(5, 2.0f);
  t.insert(16, 3.0f);
  t.insert(3, 9.0f);
  t.insert(12, 1.0f);
  t.insert(4, 0.0f);

  _check(t.at_left_or_default(80) == 0.0f);
  _check(t.at_right_or_default(80.0f) == 0);
  view::bimap(t);
}

void test_swap() {
  using bmp = bimap<int, float>;
  bmp t, tm;

  t.insert(1, 7.0f);
  t.insert(5, 2.0f);
  t.insert(16, 3.0f);
  t.insert(3, 9.0f);
  t.insert(12, 1.0f);
  t.insert(4, 0.0f);
  // swap(t, b);

  std::vector<int> lefts;
  std::vector<float> rights;

  for (auto it = t.begin_left(); it != t.end_left(); ++it) {
    lefts.push_back(*it);
  }
  for (auto it = t.begin_right(); it != t.end_right(); ++it) {
    rights.push_back(*it);
  }

  swap(t, tm);

  view::container(lefts);
  view::container(rights);
  _check(std::equal(tm.begin_left(), tm.end_left(), lefts.begin()));
  _check(std::equal(tm.begin_right(), tm.end_right(), rights.begin()));
  _check(t.empty());
  _check(t.begin_left() == t.end_left());
  _check(t.begin_right() == t.end_right());
}

void test_simple() {
  bimap<int, int> bm;
  bm.insert(4, 4);
  _check(bm.at_left(4) == 4);
  _check(bm.at_right(4) == 4);
}

void test_copy() {
  using bmp = bimap<int, float>;
  bmp t, tm;

  t.insert(1, 7.0f);
  t.insert(5, 2.0f);
  t.insert(16, 3.0f);
  t.insert(3, 9.0f);
  t.insert(12, 1.0f);
  t.insert(4, 0.0f);
  // swap(t, b);

  std::vector<int> lefts;
  std::vector<float> rights;

  for (auto it = t.begin_left(); it != t.end_left(); ++it) {
    lefts.push_back(*it);
  }
  for (auto it = t.begin_right(); it != t.end_right(); ++it) {
    rights.push_back(*it);
  }

  t = tm;

  view::container(lefts);
  view::container(rights);
  _check(std::equal(tm.begin_left(), tm.end_left(), lefts.begin()));
  _check(std::equal(tm.begin_right(), tm.end_right(), rights.begin()));
  _check(t.empty());
  _check(t.begin_left() == t.end_left());
  _check(t.begin_right() == t.end_right());
}

void test_simple_2() {
  bimap<int, float> b;

  check_empty_tree(b);
  _check(b.end_left().flip() == b.end_right());
  _check(b.end_left() == b.end_right().flip());
}

int _main() {
  _run(test_static);
  _run(test_empty);
  _run(test_move);
  _run(test_find);
  _run(test_at);
  _run(test_at_or_default);
  _run(test_swap);
  _run(test_simple);
  _run(test_copy);
  _run(test_simple_2);
  return 0;
}