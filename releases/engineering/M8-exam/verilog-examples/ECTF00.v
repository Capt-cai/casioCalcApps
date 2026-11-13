module ecen21_tff(input clk,rst,t,
  output reg q);
always @(posedge clk)
  if (rst) q <= 1'b0;
  else q <= q ^ t;
endmodule

