module ecen21_modulo(input clk,rst,enable,
  output reg [3:0] q);
always @(posedge clk)
  if (rst) q <= 0;
  else if(enable)
    if(q>=11) q <= 0;
    else q <= q+1'b1;
endmodule

