`include "metron_tools.sv"

// Writing a register multiple times in the same function is OK.

module Module
(
  input logic clock,
  output logic my_sig
);

  always_comb begin /*tock*/
    logic temp;

    my_sig = 0;
    my_sig = 1;
    temp = my_sig;
  end

endmodule

