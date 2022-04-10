// RISC-V SiMPLE SV -- multiplexer module
// BSD 3-Clause License
// (c) 2017-2019, Arthur Matos, Marcus Vinicius Lamar, Universidade de Brasília,
//                Marek Materzok, University of Wrocław

`ifndef RVSIMPLE_MULTIPLEXER4_H
`define RVSIMPLE_MULTIPLEXER4_H

`include "config.sv"
`include "constants.sv"
`include "metron_tools.sv"

module multiplexer4
#(parameter int WIDTH = 32)
(
   input logic clock,
   input logic[WIDTH-1:0] in0,
   input logic[WIDTH-1:0] in1,
   input logic[WIDTH-1:0] in2,
   input logic[WIDTH-1:0] in3,
   input logic[1:0] sel,
   output logic[WIDTH-1:0] out
);

  always_comb begin /*tock*/
    case (sel)
      0:  out = in0;
      1:  out = in1;
      2:  out = in2;
      3:  out = in3;
      default: out = WIDTH'(1'bx);
    endcase
  end
endmodule

`endif  // RVSIMPLE_MULTIPLEXER4_H

