`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/21 19:08:27
// Design Name: 
// Module Name: Synchronous_D_FF_tb
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


module Synchronous_D_FF_tb();
    
    reg CLK, D, RST_n;
    wire Q1, Q2;
    
    Synchronous_D_FF uul(.CLK(CLK), .D(D), .RST_n(RST_n), .Q1(Q1), .Q2(Q2));
    
    initial
        CLK = 0;
    always 
        #40 CLK = ~CLK;
    
    initial
    begin
        D = 1;
        RST_n = 1;
        
        #40 D = 0;
        #1 RST_n = 0;
        
        #50 D = 1;
        #10 RST_n = 1;
        
        #30 D = 0;
        RST_n = 0;
        #20 D = 1;
        #10RST_n = 1;
        #5 D = 0;
    end
endmodule
