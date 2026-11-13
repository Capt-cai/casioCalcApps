module ecen21_add_b(
  input [4:0] a,b, input cin,
  output reg [4:0] sum, output reg cout);
always @(*) {cout,sum} = a+b+cin;
endmodule

