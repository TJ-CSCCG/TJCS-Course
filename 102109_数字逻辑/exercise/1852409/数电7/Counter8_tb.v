`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/28 19:55:07
// Design Name: 
// Module Name: Counter8_tb
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


module Counter8_tb(
    );
    reg CLK, rst_n;
    wire [2:0] oQ;
    wire [6:0] oDisplay;
    
    Counter8 uul(CLK, rst_n, oQ, oDisplay);
    
    initial begin
        CLK = 0;
        rst_n = 1;
    end
    
    always 
        #10 CLK = ~CLK;
        
    
    
endmodule
