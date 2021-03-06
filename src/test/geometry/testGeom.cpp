////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2019 Prashant K. Jha
//  Copyright (c) 2019 Patrick Diehl
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include <hpx/hpx_main.hpp>

#include "testGeomLib.h"

int main() {
  //
  // test Fracture class and specifically the bitwise operation
  //
  test::testFracture();

  return EXIT_SUCCESS;
}
