`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/30 20:29:20
// Design Name: 
// Module Name: encoder83_Pri_tb
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

`timescale 1ns / 1ns
module encoder83_Pri_tb();
    reg [7:0] iData;
    wire [2:0] oData;
    encoder83_Pri uul(.iData(iData), .oData(oData));
    
    initial 
    begin
        iData = 8'b1111_1111;
        #40 iData = 8'b0111_1111;
        #40 iData = 8'b0011_1111;                      
        #40 iData = 8'b0001_1111;
        #40 iData = 8'b0000_1111;
        #40 iData = 8'b0000_0111;
        #40 iData = 8'b0000_0011;
        #40 iData = 8'b0000_0001;
        #40 iData = 8'b0000_0000; 
    end
endmodule
