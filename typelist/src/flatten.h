/*** unfold array of arrays of arrays... to array (of types) ***/
#pragma once

#include "base.h"

#include <cstddef>

namespace tl {
  template <typename Type>
  struct flatten_class {
    using type = type_list<Type>;
  };

  /* meta-function itself */
  template <typename List>
  using flatten = flatten_class<List>::type;

  template <template <typename...> typename List, typename... Types>
  struct flatten_class<List<Types...>> {
    using type = concat<List<>, flatten<Types>...>;
  };
} /* namespace tl */
