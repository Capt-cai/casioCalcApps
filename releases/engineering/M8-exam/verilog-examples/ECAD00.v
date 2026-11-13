module ecen21_add_d(
  input [4:0] a,b, input cin,
  output [4:0] sum, output cout);
assign {cout,sum} = a+b+cin;
endmodule
