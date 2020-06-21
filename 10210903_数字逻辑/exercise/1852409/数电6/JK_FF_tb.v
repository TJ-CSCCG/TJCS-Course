`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/28 15:44:05
// Design Name: 
// Module Name: JK_FF_tb
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


module JK_FF_tb();
    reg CLK, J, K, RST_n;
    wire Q1,Q2;
    
    JK_FF uul(CLK, J, K, RST_n, Q1, Q2);
    
    initial begin
        J = 0;
        K = 0;
        CLK = 0;
        RST_n = 1;
    end
    always
        #10 CLK = ~CLK;
    
    initial begin
        J = 1;K = 0;
        #40 J = 1; K = 1;
        #40 J = 0; K = 0;
        #40 J = 0; K = 1;
        
        #40 RST_n = 0; J = 1; K = 0;
        #40 J = 1; K = 1;
        #40 J = 0; K = 0;
        #40 J = 0; K = 1;

        #40 RST_n = 1; J = 1; K = 0;
        #40 J = 1; K = 1;
        #40 J = 0; K = 0;
        #40 J = 0; K = 1;
    end
endmodule
