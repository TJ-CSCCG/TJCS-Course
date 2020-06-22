`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/22 20:05:49
// Design Name: 
// Module Name: DMUX_tb
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
module DMUX_tb();
    reg iS0;
    reg iS1;
    wire oZ0;
    wire oZ1;
    wire oZ2;
    wire oZ3;
    reg iC;
        
    initial
    begin
        iS0 = 0;
        #80 iS0 = 1;
    end
    
    initial 
    begin
        iS1 = 0;
        #40 iS1 = 1;
        #80 iS1 = 0;
        #40 iS1 = 1;
    end
    
    de_selector14 uul(.iC(iC),.iS0(iS0), .iS1(iS1), .oZ0(oZ0), .oZ1(oZ1), .oZ2(oZ2), .oZ3(oZ3));

endmodule
