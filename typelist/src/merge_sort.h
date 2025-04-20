/*** unfold array of arrays of arrays... to array (of types) ***/
#pragma once

#include "base.h"

#include <cstddef>

// clang-format off
namespace tl {
  /*** splice array ***/
  template <typename List, std::size_t N>
  struct get_first_class {
    template <typename PairIndexValue>
    struct less {
      static constexpr bool value = get_index<PairIndexValue> < N;
    };

    using type = get_all_values<filter<less, indexate<List>>>;
  };

  template <typename List, std::size_t N>
  using get_first = get_first_class<List, N>::type;

  template <typename List, std::size_t N>
  struct get_last_class {
    template <typename PairIndexValue>
    struct not_less {
      static constexpr bool value = get_index<PairIndexValue> >= N;
    };

    using type = get_all_values<filter<not_less, indexate<List>>>;
  };

  template <typename List, std::size_t N>
  using get_last = get_last_class<List, N>::type;

  /*** merge (see non-class version below) ***/
  template <template <typename...> typename Compare, typename List1, typename List2>
  struct merge_class;

  template <template <typename...> typename Compare, typename List1, typename List2>
  using merge = merge_class<Compare, List1, List2>::type;

  // <> merge <>
  template <template <typename...> typename Compare, template <typename...> typename List>
  struct merge_class<Compare, List<>, List<>> {
    using type = List<>;
  };

  // <types> merge <>
  template <
    template <typename...> typename Compare,
    template <typename...> typename List, typename Type, typename... Types
  >
  struct merge_class<Compare, List<Type, Types...>, List<>> {
    using type = List<Type, Types...>;
  };

  // <> merge <types>
  template <
    template <typename...> typename Compare,
    template <typename...> typename List, typename Type, typename... Types
  >
  struct merge_class<Compare, List<>, List<Type, Types...>> {
    using type = List<Type, Types...>;
  };

  // <types1> merge <types2>
  template <
    template <typename...> typename Compare,
    template <typename...> typename List,
    typename Type1, typename... Types1,
    typename Type2, typename... Types2
  >
  struct merge_class<Compare, List<Type1, Types1...>, List<Type2, Types2...>> {
    using type = mux<
      Compare<Type1, Type2>::value,
      concat<List<Type2>, merge<Compare, List<Type1, Types1...>, List<Types2...>>>,
      concat<List<Type1>, merge<Compare, List<Types1...>, List<Type2, Types2...>>>
    >;
  };

  /*** merge_sort ***/
  template<typename List, template<typename...> typename Compare, std::size_t N>
  struct merge_sort_size_class;

  template<typename List, template<typename...> typename Compare, std::size_t N>
  using merge_sort_size = merge_sort_size_class<List, Compare, N>::type;

  template<typename List, template<typename...> typename Compare, std::size_t N>
  struct merge_sort_size_class {
    using type = merge<
      Compare,
      typename merge_sort_size_class<get_first<List, N / 2>, Compare, N / 2>::type,
      typename merge_sort_size_class<get_last<List, N / 2>, Compare, N - N / 2>::type
    >;
  };

  template<typename List, template<typename...> typename Compare>
  struct merge_sort_size_class<List, Compare, 0> {
    using type = List;
  };

  template<typename List, template<typename...> typename Compare>
  struct merge_sort_size_class<List, Compare, 1> {
    using type = List;
  };

  template<typename List, template<typename...> typename Compare>
  using merge_sort = merge_sort_size<List, Compare, size<List>>;
} /* namespace tl */

// clang-format on
