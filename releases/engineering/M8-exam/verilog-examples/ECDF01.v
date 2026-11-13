module ecen21_dff(input clk,rst,d,
  output reg q);
always @(negedge clk or negedge rst)
  if (!rst) q <= 1'b1;
  else q <= d;
endmodule

