module ecen21_shift(
  input clk,rst,load,shift_right,shift_left,
  input serial_left,serial_right,
  input [4:0] parallel,
  output reg [4:0] q);
always @(posedge clk)
  if (rst) q <= 0;
  else if(load) q <= parallel;
  else if(shift_right)
    q <= {serial_left,q[4:1]};
  else if(shift_left)
    q <= {q[3:0],serial_right};
endmodule

