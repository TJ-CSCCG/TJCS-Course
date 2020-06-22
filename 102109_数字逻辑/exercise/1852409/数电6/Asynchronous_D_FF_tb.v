`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/21 20:12:03
// Design Name: 
// Module Name: Asynchronous_D_FF_tb
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


module Asynchronous_D_FF_tb();
    
    reg CLK, D, RST_n;
    wire Q1, Q2;
    Asynchronous_D_FF uul(CLK,D,RST_n, Q1,Q2);
    
    initial CLK = 0;
    always 
        #40 CLK = ~CLK;
    
    initial
    begin
        D = 1;
        RST_n = 0;
        #10 D = 0;
        #5 RST_n = 1;
        #15 D = 0;
        #50 RST_n = 0;
        #10 RST_n = 1;
        #20 D = 1;
        #15 RST_n = 0;
        #15 D = 0;
        #5 RST_n = 1;
        #30 D = 1;
        #5 RST_n = 0;
    end
endmodule
