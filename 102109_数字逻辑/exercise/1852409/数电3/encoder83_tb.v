`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/30 19:53:46
// Design Name: 
// Module Name: encoder83_tb
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
module encoder83_tb();
    reg [7:0] iData;
    wire [2:0] oData;
    encoder83 uul(.iData(iData), .oData(oData));
    
    initial 
    begin
        iData = 8'b0000_0000;
        
        #40 iData = 8'b0000_0001;
        #40 iData = 8'b0000_0010;
        #40 iData = 8'b0000_0100;                      
        #40 iData = 8'b0000_1000;
        #40 iData = 8'b0001_0000;
        #40 iData = 8'b0010_0000;
        #40 iData = 8'b0100_0000;
        #40 iData = 8'b1000_0000;
        
    end
    
endmodule
