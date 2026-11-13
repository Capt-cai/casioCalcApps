module ecen21_shift(
  input clk,rst,load,shift_right,shift_left,
  input serial_left,serial_right,
  input [0:0] parallel,
  output reg [0:0] q);
always @(posedge clk)
  if (rst) q <= 0;
  else if(load) q <= parallel;
  else if(shift_right)
    q <= serial_left;
  else if(shift_left)
    q <= serial_right;
endmodule

