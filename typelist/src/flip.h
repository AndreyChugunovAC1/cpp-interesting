/*** flip every typepair in typelist of typepairs ***/
#pragma once

#include "base.h"

namespace tl {
  /*** flip pair ***/
  template <template <typename...> typename Pair, typename T1, typename T2>
  inline Pair<T2, T1> flip_pair_func(Pair<T1, T2>);

  template <typename Pair>
  using flip_pair = decltype(flip_pair_func(Pair()));

  /*** flip ***/
  template <typename ListOfPairs>
  using flip = transform<flip_pair, ListOfPairs>;
} /* namespace tl */
