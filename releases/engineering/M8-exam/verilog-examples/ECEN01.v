module ecen21_encoder(input [7:0] in,
  output reg [2:0] code, output reg valid);
integer i,ones;
always @(*) begin
  code=0; ones=0;
  for(i=0;i<8;i=i+1)
    if(in[i]) begin
      code=code|i; ones=ones+1;
    end
  valid=(ones==1);
end
endmodule
