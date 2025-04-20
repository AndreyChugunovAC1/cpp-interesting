#ifdef __DEFINETELY_UNDEFINED_MACROVALUE
#define _main main
#endif /* __DEFINETELY_UNDEFINED_MACROVALUE */

#include "typelist.h"

#include <tuple>
#include <utility>

// clang-format off

void test_basics() {
  /*** basics ***/
  static_assert(std::is_same_v<
    tl::get_value<std::tuple<std::integral_constant<std::size_t, 3>, int>>,
    int
  >);

  static_assert(std::is_same_v<int, int>);

  static_assert(std::is_same_v<
    tl::concat<std::tuple<>, std::tuple<>, std::tuple<>, std::tuple<>, std::tuple<>>,
    std::tuple<>
  >);

  static_assert(std::is_same_v<
    tl::concat<std::tuple<>, std::tuple<int>, std::tuple<>>,
    std::tuple<int>
  >);

  static_assert(std::is_same_v<
    tl::concat<std::tuple<int, double, float>, std::tuple<char>, std::tuple<short>>,
    std::tuple<int, double, float, char, short>
  >);

  static_assert(std::is_same_v<
    tl::transform<std::add_const_t, std::tuple<>>,
    std::tuple<>
  >);

  static_assert(std::is_same_v<tl::mux<true, int, double>, double>);
  static_assert(std::is_same_v<tl::mux<true, int, float>, float>);
  static_assert(std::is_same_v<tl::mux<false, int, double>, int>);
  static_assert(std::is_same_v<tl::mux<false, int, float>, int>);

  static_assert(std::is_same_v<
    tl::filter<std::is_integral, std::tuple<int, float, long, short, double, char>>,
    std::tuple<int, long, short, char>
  >);

  static_assert(std::is_same_v<
    tl::filter<std::is_pointer, std::tuple<int, double, float*, long, short*, void*>>,
    std::tuple<float*, short*, void*>
  >);

  static_assert(std::is_same_v<
    tl::filter<std::is_integral, std::tuple<>>,
    std::tuple<>
  >);

  static_assert(tl::size<std::tuple<int, float, char>> == 3);

  using ttt = tl::indexate<std::tuple<int, double>>;
  static_assert(std::is_same_v<
    ttt,
    std::tuple<
      tl::type_list<std::integral_constant<std::size_t, 0>, int>, 
      tl::type_list<std::integral_constant<std::size_t, 1>, double>
    >
  >);
  static_assert(tl::get_index<tl::type_list<std::integral_constant<std::size_t, 0>, int>> == 0);
  static_assert(tl::get_index<tl::type_list<std::integral_constant<std::size_t, 2>, int>> == 2);
  static_assert(tl::get_index<tl::type_list<std::integral_constant<std::size_t, 644>, double>> == 644);

  static_assert(std::is_same_v<
    tl::get_all_values<tl::indexate<std::tuple<>>>,
    std::tuple<>
  >);

  static_assert(std::is_same_v<
    tl::get_all_values<tl::indexate<std::tuple<int, float, double, short>>>,
    std::tuple<int, float, double, short>
  >);

  static_assert(tl::get_index<std::tuple<std::integral_constant<std::size_t, 5>, short>> == 5);

  static_assert(std::is_same_v<
    tl::get_first<std::tuple<int, float, double, char, short>, 3>,
    std::tuple<int, float, double>
  >);

  static_assert(std::is_same_v<
    tl::get_first<std::tuple<int, float, double, char, short>, 0>,
    std::tuple<>
  >);

  static_assert(std::is_same_v<
    tl::get_last<std::tuple<int, float, double, char, short>, 0>,
    std::tuple<int, float, double, char, short>
  >);

  static_assert(std::is_same_v<
    tl::get_first<std::tuple<int, float, double, char, short>, 3>,
    std::tuple<int, float, double>
  >);

  static_assert(std::is_same_v<
    tl::get_first<std::tuple<int, float, double, char, short>, 0>,
    std::tuple<>
  >);

  static_assert(std::is_same_v<
    tl::concat<
      tl::get_first<std::tuple<int, float, double, char, short>, 0>,
      tl::get_last<std::tuple<int, float, double, char, short>, 0>
    >,
    std::tuple<int, float, double, char, short>
  >);

  static_assert(std::is_same_v<
    tl::concat<
      tl::get_first<std::tuple<int, float, double, char, short>, 2>,
      tl::get_last<std::tuple<int, float, double, char, short>, 2>
    >,
    std::tuple<int, float, double, char, short>
  >);

  static_assert(std::is_same_v<
    tl::concat<
      tl::get_first<std::tuple<int, float, double, char, short>, 3>,
      tl::get_last<std::tuple<int, float, double, char, short>, 3>
    >,
    std::tuple<int, float, double, char, short>
  >);

  static_assert(std::is_same_v<
    tl::concat<
      tl::get_first<std::tuple<int, float, double, char, short>, 10>,
      tl::get_last<std::tuple<int, float, double, char, short>, 10>
    >,
    std::tuple<int, float, double, char, short>
  >);

  static_assert(tl::size<tl::get_first<std::tuple<int, double, float>, 2>> == 2);
}

void test_contains() {
  /*** contains ***/
  static_assert(tl::contains_class<int, std::tuple<int, float>>::value);

  static_assert(tl::contains<char, std::tuple<int, long, float, double, char>>);
  static_assert(!tl::contains<unsigned char, std::tuple<int, long, float, double, char>>);
  static_assert(!tl::contains<char, tl::type_list<>>);
  static_assert(tl::contains<char, tl::type_list<int, char, long>>);
}

void test_flip() {
  /*** flip pair ***/
  static_assert(std::is_same_v<tl::flip_pair<std::tuple<int, double>>, std::tuple<double, int>>);
  static_assert(!std::is_same_v<tl::flip_pair<std::tuple<int, double>>, std::tuple<int, int>>);
  static_assert(!std::is_same_v<tl::flip_pair<std::tuple<int, double>>, std::tuple<int, double>>);
  static_assert(std::is_same_v<
    tl::flip_pair<std::tuple<std::tuple<int, double>, std::tuple<float, long>>>, 
    std::tuple<std::tuple<float, long>, std::tuple<int, double>>
  >);

  static_assert(std::is_same_v<
    tl::transform<std::add_const_t, std::tuple<int, double>>,
    std::tuple<const int, const double>
  >);

  /*** flip ***/
  static_assert(std::is_same_v<
    tl::flip<std::tuple<std::pair<long, double>>>,
    std::tuple<std::pair<double, long>>
  >);

  static_assert(std::is_same_v<
    tl::flip<std::tuple<std::pair<long, double>, std::pair<float, double>>>,
    std::tuple<std::pair<double, long>, std::pair<double, float>>
  >);

  static_assert(std::is_same_v<
    tl::flip<std::tuple<>>,
    std::tuple<>
  >);

  static_assert(std::is_same_v<
    tl::flip<tl::type_list<>>,
    tl::type_list<>
  >);
}

void test_index_of_unique() {
  /*** index of unique ***/
  static_assert(tl::index_of_unique<int, std::tuple<float, int>> == 1);
  static_assert(tl::index_of_unique<int, std::tuple<float, int, double, char>> == 1);
  static_assert(tl::index_of_unique<int, std::tuple<int, float, double, char>> == 0);
  static_assert(tl::index_of_unique<int, std::tuple<char, char, char, char, float, int, long>> == 5);
  static_assert(tl::index_of_unique<unsigned char const, std::tuple<unsigned char const>> == 0);
}

void test_flatten() {
  /*** flatten ***/
  static_assert(std::is_same_v<
    tl::concat<std::tuple<int, double>>,
    std::tuple<int, double>
  >);

  static_assert(std::is_same_v<
    tl::concat<std::tuple<>, std::tuple<>, std::tuple<>>,
    std::tuple<>
  >);

  static_assert(std::is_same_v<
    tl::concat<std::tuple<int, double>, std::tuple<long>>,
    std::tuple<int, double, long>
  >);

  static_assert(std::is_same_v<
    tl::concat<std::tuple<int, double>, std::tuple<long>, std::tuple<char>, std::tuple<>, std::tuple<int>>,
    std::tuple<int, double, long, char, int>
  >);

  static_assert(std::is_same_v<
    tl::concat<std::tuple<int, double>, std::tuple<std::tuple<char, char>, long>, std::tuple<char>, std::tuple<>, std::tuple<int>>,
    std::tuple<int, double, std::tuple<char, char>, long, char, int>
  >);

  static_assert(std::is_same_v<
    tl::concat<
      std::tuple<int, double>,
      std::tuple<std::tuple<char, char>, long>,
      std::tuple<char>,
      std::tuple<>,
      std::tuple<int>>,
    std::tuple<int, double, std::tuple<char, char>, long, char, int>
  >);

  static_assert(std::is_same_v<
    tl::concat<
      tl::type_list<int, double>,
      tl::type_list<std::tuple<char, char>, long>,
      std::tuple<char>,
      tl::type_list<>,
      std::tuple<int>>,
    tl::type_list<int, double, std::tuple<char, char>, long, char, int>
  >);

  static_assert(std::is_same_v<
    tl::flatten<int>,
    tl::type_list<int>
  >);

  // using ttt = tl::flatten<std::tuple<int>>;

  static_assert(std::is_same_v<
    tl::flatten<std::tuple<int>>,
    std::tuple<int>
  >);

  static_assert(std::is_same_v<
    tl::flatten<std::tuple<tl::type_list<int>>>,
    std::tuple<int>
  >);

  static_assert(std::is_same_v<
    tl::flatten<std::tuple<tl::type_list<int>, long, std::tuple<float, tl::type_list<char>>>>,
    std::tuple<int, long, float, char>
  >);

  static_assert(std::is_same_v<
    tl::flatten<std::tuple<tl::type_list<>, long, std::tuple<float, tl::type_list<char>>>>,
    std::tuple<long, float, char>
  >);

  static_assert(std::is_same_v<
    tl::flatten<std::tuple<tl::type_list<>, std::tuple<std::tuple<>, tl::type_list<>>>>,
    std::tuple<>
  >);
}

/* test comparator */
template <typename First, typename Second>
struct compare_class {
  static constexpr bool value = (sizeof(First) < sizeof(Second));
};

namespace merge_sort {
  /* test structures with different size */
  #define sizeN_struct(N, Nm1)  \
    struct size ## N {          \
      size1 a;                  \
      size ## Nm1 b;            \
    }

  struct size1 {
    char a;
  };

  // f***ing macroses (it is impossible to create a macro that concatenates name ## N-1 correctly):
  sizeN_struct(2, 1);
  sizeN_struct(3, 2);
  sizeN_struct(4, 3);
  sizeN_struct(5, 4);
  sizeN_struct(6, 5);
  sizeN_struct(7, 6);
  sizeN_struct(8, 7);
  sizeN_struct(9, 8);
  sizeN_struct(10, 9);
  sizeN_struct(11, 10);
  sizeN_struct(12, 11);
  
  void test_merge() {
    /*** comparator ***/
    static_assert(sizeof(size1) == 1);
    static_assert(sizeof(size2) == 2);
    static_assert(sizeof(size3) == 3);
    static_assert(sizeof(size4) == 4);
    static_assert(sizeof(size5) == 5);
    static_assert(sizeof(size6) == 6);
    static_assert(sizeof(size7) == 7);
    static_assert(sizeof(size8) == 8);
    static_assert(sizeof(size9) == 9);
    static_assert(sizeof(size10) == 10);
    static_assert(sizeof(size11) == 11);
    static_assert(sizeof(size12) == 12);

    static_assert(compare_class<size10, size12>::value == 1);
    static_assert(compare_class<size11, size12>::value == 1);
    static_assert(compare_class<size3, size7>::value == 1);
    static_assert(compare_class<size6, size5>::value == 0);
    static_assert(compare_class<size3, size1>::value == 0);

    /*** merge ***/
    static_assert(std::is_same_v<
      tl::merge<compare_class, std::tuple<>, std::tuple<>>,
      std::tuple<>
    >);

    static_assert(std::is_same_v<
      tl::merge<compare_class, std::tuple<size1, size5>, std::tuple<>>,
      std::tuple<size1, size5>
    >);

    static_assert(std::is_same_v<
      tl::merge<compare_class, std::tuple<>, std::tuple<size2, size4, size6>>,
      std::tuple<size2, size4, size6>
    >);

    // using ttt = tl::merge<std::tuple<size1>, std::tuple<size2>, compare_class>;
    static_assert(std::is_same_v<
      tl::merge<compare_class, std::tuple<size1>, std::tuple<size2>>,
      std::tuple<size1, size2>
    >);

    static_assert(std::is_same_v<
      tl::merge<
        compare_class, 
        std::tuple<size1, size7>, 
        std::tuple<size2>
      >,
      std::tuple<size1, size2, size7>
    >);

    static_assert(std::is_same_v<
      tl::merge<
        compare_class, 
        std::tuple<size1, size7, size8>, 
        std::tuple<size2, size3, size4, size5, size9>
      >,
      std::tuple<size1, size2, size3, size4, size5, size7, size8, size9>
    >);

    static_assert(std::is_same_v<
      tl::get_first<std::tuple<>, 0>,
      std::tuple<>
    >);

    static_assert(std::is_same_v<
      tl::get_first<std::tuple<>, 0>,
      tl::get_last<std::tuple<>, 0>
    >);
  }


  void test_merge_sort() {
    using array1 = std::tuple<size5>;

    static_assert(std::is_same_v<
      tl::merge_sort<std::tuple<>, compare_class>,
      std::tuple<>
    >);

    static_assert(std::is_same_v<
      tl::merge_sort<array1, compare_class>,
      array1
    >);

    using array3 = std::tuple<size5, size2>;
    static_assert(std::is_same_v<
      tl::merge_sort<array3, compare_class>,
      std::tuple<size2, size5>
    >);

    using array2 = std::tuple<size5, size2, size6, size7, size4, size3, size1, size4>;
    static_assert(std::is_same_v<
      tl::merge_sort<array2, compare_class>,
      std::tuple<size1, size2, size3, size4, size4, size5, size6, size7>
    >);
  }
}

int _main() {
  test_basics();
  test_contains();
  test_index_of_unique();
  test_flip();
  test_flatten();
  merge_sort::test_merge();
  merge_sort::test_merge_sort();
  return 0;
}

// clang-format on
