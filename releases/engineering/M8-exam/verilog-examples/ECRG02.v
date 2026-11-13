module ecen21_ring(input clk,rst,enable,
  output reg [7:0] q);
always @(negedge clk or negedge rst)
  if (!rst) q <= 8'd4;
  else if(enable) q <= {q[0],q[7:1]};
endmodule
