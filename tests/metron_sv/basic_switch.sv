`include "metron_tools.sv"

// Simple switch statements are OK.

module Module
(
  input logic clock,
  input logic[1:0] tock_selector
);
/*public:*/

  always_ff @(posedge clock) begin /*tock*/
tick_selector = tock_selector;
        /*tick(selector)*/;
  end

/*private:*/

  logic[1:0] tick_selector;
  always_ff @(posedge clock) begin /*tick*/
    switch(tick_selector) begin
       0: my_reg <= 17;
       1: my_reg <= 22;
       2: my_reg <= 30;
       3, // fallthrough
       4,
       5,
       6: my_reg <= 72;
    end
  end

  logic[7:0] my_reg;
endmodule

