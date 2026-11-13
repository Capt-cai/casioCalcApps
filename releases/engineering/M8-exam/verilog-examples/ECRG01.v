module ecen21_ring(input clk,rst,enable,
  output reg [0:0] q);
always @(posedge clk)
  if (rst) q <= 1'd1;
  else if(enable) q <= q;
endmodule
