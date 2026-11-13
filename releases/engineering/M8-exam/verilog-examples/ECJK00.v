module ecen21_jkff(input clk,rst,j,k,
  output reg q);
always @(posedge clk)
  if (rst) q <= 1'b0;
  else case ({j,k})
    2'b00: q <= q;
    2'b01: q <= 0;
    2'b10: q <= 1;
    2'b11: q <= ~q;
    default: q <= 1'bx;
  endcase
endmodule
