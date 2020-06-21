`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/23 19:35:17
// Design Name: 
// Module Name: transmisson_tb
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
module transmission_tb();
    reg [7:0] iData;
    wire [7:0] oData;
    reg A, B, C;
    
    transmission8 uul(.iData(iData), .A(A), .B(B), .C(C), .oData(oData));
    initial
    begin
        #40 A = 0;
        B = 0;
        C = 0;
        iData = 8'b0000_1000;
        
        #40 A = 0;
        B = 0;
        C = 1;
        iData = 8'b0010_1000;
        
        #40 A = 0;
        B = 1;
        C = 0;
        iData = 8'b1111_1111;
    end
    
endmodule
