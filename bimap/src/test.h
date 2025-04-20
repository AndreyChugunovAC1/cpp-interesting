#pragma once

#include <cstdlib>
#include <iostream>
#include <string>

#define _view(expr) std::cout << "\033[1;32m:\033[0m " << #expr << " \033[1;32m==\033[0m " << (expr) << "\n"
#define _run(test)                                                                                                     \
  std::cout << "\033[1;34m==running " << #test << "==\033[0m\n";                                                       \
  test();                                                                                                              \
  std::cout << "\033[1;34m==end of " << #test << "==\033[0m\n";
#define _msg(name) std::cout << "\033[1;35m--" << name << "--\033[0m\n"
#define _check(expr)                                                                                                   \
  (expr) ? (std::cout << "\033[1;32mOk\033[0m {" << #expr << "}\n", 0)                                                 \
         : (std::cout << "\033[1;31mFailed\033[0m {" << #expr << "}\n", exit(0), 0)

#define _iseq(expr, val) _check((expr) == (val))
#define _isneq(expr, val) _check((expr) != (val))
