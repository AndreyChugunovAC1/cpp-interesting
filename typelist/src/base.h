/*** basic meta-functions ***/
#pragma once

#include <cstddef>
#include <utility>

namespace tl {
  /* basic typelist -- default for flatten */
  template <typename... Types>
  struct type_list {};

  /*** transform (aka map) ***/
  template <template <typename...> typename Func, template <typename...> typename List, typename... Types>
  inline List<Func<Types>...> transform_func(List<Types...>);

  template <template <typename...> typename Func, typename List>
  using transform = decltype(transform_func<Func>(List()));

  /*** fast concat ***/
  template <typename Type>
  struct identity {
    using type = Type;
  };

  template <typename Identity>
  using get_identity = Identity::type;

  // clang-format off
  template <
    template <typename...> typename List1, template <typename...> typename List2, 
    typename... Types1, typename... Types2
  >
  inline identity<List1<Types1..., Types2...>> operator+(identity<List1<Types1...>>, identity<List2<Types2...>>);
  // clang-format on

  template <typename... Lists>
  using concat = get_identity<decltype((identity<Lists>{} + ... + identity<type_list<>>{}))>;

  /*** binary type multiplexer (aka conditional) ***/
  template <bool Index>
  struct mux_struct;

  template <>
  struct mux_struct<true> {
    template <typename T1, typename T2>
    using type = T2;
  };

  template <>
  struct mux_struct<false> {
    template <typename T1, typename T2>
    using type = T1;
  };

  template <bool Index, typename T1, typename T2>
  using mux = mux_struct<Index>::template type<T1, T2>;

  /*** simple filter meta-function ***/
  template <template <typename...> typename Pred, typename List>
  struct filter_class;

  template <template <typename...> typename Pred, typename List>
  using filter = filter_class<Pred, List>::type;

  template <template <typename...> typename Pred, template <typename...> typename List, typename... Types>
  struct filter_class<Pred, List<Types...>> {
    template <typename Type>
    using single = mux<Pred<Type>::value, List<>, List<Type>>;

    using type = concat<single<Types>...>;
  };

  template <template <typename...> typename Pred, template <typename...> typename List>
  struct filter_class<Pred, List<>> {
    using type = List<>;
  };

  /*** get list size (for indexation) ***/
  template <typename List>
  struct size_class;

  template <typename List>
  constexpr std::size_t size = size_class<List>::value;

  template <template <typename...> typename List, typename... Types>
  struct size_class<List<Types...>> {
    static constexpr std::size_t value = sizeof...(Types);
  };

  /*** simple indexation meta-function ***/
  template <typename List, typename Seq>
  struct indexate_class;

  template <typename List>
  using indexate = indexate_class<List, std::make_index_sequence<size<List>>>::type;

  template <template <typename...> typename List, typename... Types, std::size_t... Indexes>
  struct indexate_class<List<Types...>, std::index_sequence<Indexes...>> {
    using type = List<type_list<std::integral_constant<std::size_t, Indexes>, Types>...>;
  };

  /*** get index from pair, created by indexation ***/
  template <typename T>
  struct get_index_class;

  template <typename T>
  constexpr std::size_t get_index = get_index_class<T>::value;

  template <template <typename...> typename Pair, std::size_t Index, typename Value>
  struct get_index_class<Pair<std::integral_constant<std::size_t, Index>, Value>> {
    static constexpr std::size_t value = Index;
  };

  /*** get value from pair, created by indexation ***/
  template <typename T>
  struct get_value_class;

  template <typename T>
  using get_value = get_value_class<T>::type;

  template <template <typename...> typename Pair, typename IndexType, typename Value>
  struct get_value_class<Pair<IndexType, Value>> {
    using type = Value;
  };

  /*** get all values from indexed pairs ***/
  template <typename List>
  struct get_all_values_class;

  template <typename List>
  using get_all_values = get_all_values_class<List>::type;

  template <template <typename...> typename List>
  struct get_all_values_class<List<>> {
    using type = List<>;
  };

  template <template <typename...> typename List, typename... Types>
  struct get_all_values_class<List<Types...>> {
    using type = List<get_value<Types>...>;
  };
} /* namespace tl */
