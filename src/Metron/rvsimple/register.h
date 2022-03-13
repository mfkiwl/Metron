// RISC-V SiMPLE SV -- generic register
// BSD 3-Clause License
// (c) 2017-2019, Arthur Matos, Marcus Vinicius Lamar, Universidade de Brasília,
//                Marek Materzok, University of Wrocław

#ifndef RVSIMPLE_REGISTER_H
#define RVSIMPLE_REGISTER_H

#include "metron_tools.h"
#include "config.h"
#include "constants.h"

template<int WIDTH = 32, int INITIAL = 0>
class _register {
public:

  logic<WIDTH> reg_value;
  logic<WIDTH> value;

  void init() {
    reg_value = INITIAL;
  }

  void tick(logic<1> reset, logic<1> write_enable, logic<WIDTH> next) {
    if (reset) reg_value = INITIAL;
    else if (write_enable) reg_value = next;
  }

  void tock() {
    value = reg_value;
  }
};

#endif // RVSIMPLE_REGISTER_H