`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/28 22:22:43
// Design Name: 
// Module Name: Divider_tb
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


module Divider_tb();
    reg I_CLK;
    reg rst;
    wire O_CLK;
    
    
    initial begin
        I_CLK = 0;
        rst = 0;
    end
    always #20 I_CLK = ~I_CLK;
    
    Divider uul(I_CLK, rst, O_CLK);
endmodule
