`include"metron_tools.sv"

// Static const members become SV localparams

module Module
(
  input logic clock
);
/*public:*/

  always_comb begin /*tock*/
    /*tick()*/;
  end

/*private:*/

  localparam int my_val = 7;

  always_comb begin /*tick*/
    my_reg = my_val;
  end

  logic[6:0] my_reg;
endmodule;

