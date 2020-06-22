`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/21 19:59:03
// Design Name: 
// Module Name: Asynchronous_D_FF
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


module Asynchronous_D_FF(
    input CLK,
    input D,
    input RST_n,
    output reg Q1,
    output reg Q2
    );
    
    always @ (posedge CLK, negedge RST_n)
    begin
        if (!RST_n || !D)
        begin
            Q1 = 0;
            Q2 = 1; 
        end
        else
        begin
            Q1 = 1;
            Q2 = 0;
        end
    end
endmodule
