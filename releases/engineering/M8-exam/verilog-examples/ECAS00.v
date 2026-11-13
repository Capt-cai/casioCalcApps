module ecen21_fa1(input a,b,cin,
  output sum,cout);
wire p,g,h;
xor (p,a,b), (sum,p,cin);
and (g,a,b), (h,p,cin);
or (cout,g,h);
endmodule

module ecen21_add_s(
  input [4:0] a,b, input cin,
  output [4:0] sum, output cout);
wire [5:0] carry;
assign carry[0]=cin;
assign cout=carry[5];
genvar i;
generate for(i=0;i<5;i=i+1) begin:stage
  ecen21_fa1 fa(a[i],b[i],carry[i],
    sum[i],carry[i+1]);
end endgenerate
endmodule

