/*** index of unique type in typelist ***/
#pragma once

#include <cstddef>

namespace tl {
  /*** get value from list with one element ***/
  template <typename List>
  struct get_elem_from_list_class;

  template <typename List>
  using get_elem_from_list = get_elem_from_list_class<List>::type;

  template <template <typename...> typename List, typename Type>
  struct get_elem_from_list_class<List<Type>> {
    using type = Type;
  };

  /*** index of unique ***/
  template <typename SearchType, typename List>
  struct index_of_unique_class {
    template <typename OtherType>
    struct checker {
      static constexpr bool value = std::is_same_v<SearchType, get_value<OtherType>>;
    };

    static constexpr std::size_t value = get_index<get_elem_from_list<filter<checker, indexate<List>>>>;
  };

  template <typename Type, typename List>
  constexpr std::size_t index_of_unique = index_of_unique_class<Type, List>::value;
} /* namespace tl */
