`include "metron_tools.sv"

// Simple switch statements are OK.

module Module
(
  input logic clock,
  input logic[1:0] tick_selector
);
/*public:*/

  always_ff @(posedge clock) begin : tick
    case(tick_selector)
      0: my_reg <= 17;
      1: my_reg <= 22;
      2: my_reg <= 30;
      3, // fallthrough
      4,
      5,
      6: my_reg <= 72;
    endcase
  end

/*private:*/
  logic[7:0] my_reg;
endmodule

