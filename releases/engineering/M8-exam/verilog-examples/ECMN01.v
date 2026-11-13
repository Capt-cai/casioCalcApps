module ecen21_modulo(input clk,rst,enable,
  output reg [0:0] q);
always @(posedge clk)
  if (rst) q <= 0;
  else if(enable)
    if(q>=0) q <= 0;
    else q <= q+1'b1;
endmodule
