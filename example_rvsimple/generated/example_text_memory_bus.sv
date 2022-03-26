// RISC-V SiMPLE SV -- program text memory bus
// BSD 3-Clause License
// (c) 2017-2019, Arthur Matos, Marcus Vinicius Lamar, Universidade de Brasília,
//                Marek Materzok, University of Wrocław

`ifndef RVSIMPLE_EXAMPLE_TEXT_MEMORY_BUS_H
`define RVSIMPLE_EXAMPLE_TEXT_MEMORY_BUS_H

`include "config.sv"
`include "constants.sv"
`include "example_text_memory.sv"
`include "metron_tools.sv"

module example_text_memory_bus
(
  input logic clock,
  input logic[31:0] address,
  output logic[31:0] read_data
);
 /*public:*/

  initial begin : init /*text_memory.init();*/ end

  /*logic<32> read_data;*/

  always_comb begin : tock
    logic[31:0] fetched;
    /*text_memory.tock(bx<TEXT_BITS - 2>(address, 2));*/
    text_memory_address = address[TEXT_BITS - 2+1:2];
    fetched = text_memory_q2;

    read_data =
        (address >= TEXT_BEGIN) && (TEXT_END >= address)
        ? fetched
        : 32'x;
  end

 /*private:*/
  example_text_memory text_memory(
    // Inputs
    .clock(clock),
    .address(text_memory_address),
    // Outputs
    .q(text_memory_q),
    .q2(text_memory_q2)
  );
  logic[TEXT_BITS - 2-1:0] text_memory_address;
  logic[31:0] text_memory_q;
  logic[31:0] text_memory_q2;

endmodule;

`endif  // RVSIMPLE_EXAMPLE_TEXT_MEMORY_BUS_H

