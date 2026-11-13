module ecen21_ring(input clk,rst,enable,
  output reg [4:0] q);
always @(posedge clk)
  if (rst) q <= 5'd1;
  else if(enable) q <= {q[3:0],q[4]};
endmodule
