/*** check if typelist contains type ***/
#pragma once

#include "base.h"

#include <type_traits>

namespace tl {
  template <typename SearchType, typename List>
  struct contains_class {
    template <typename OtherType>
    struct checker {
      static constexpr bool value = std::is_same_v<SearchType, OtherType>;
    };

    static constexpr bool value = size<filter<checker, List>> > 0;
  };

  /* meta itself */
  template <typename SearchType, typename List>
  constexpr bool contains = contains_class<SearchType, List>::value;
} /* namespace tl */
