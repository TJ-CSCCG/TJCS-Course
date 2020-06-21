`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/28 19:41:14
// Design Name: 
// Module Name: Counter8
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


module Counter8(
    input CLK,
    input rst_n,
    output reg [2:0] oQ,
    output [6:0] oDisplay
    );
    
    wire CLK_div;
    Divider div(CLK, ~rst_n, CLK_div);
    
    wire [2:0] netoQn;
    wire [2:0] netoQ;
    wire Q1_i,Q2_i;
    JK_FF jk1_0(CLK_div, 1, 1, rst_n, netoQ[0], netoQn[0]);
    assign Q1_i = netoQ[0];
    JK_FF jk2_0(CLK_div, Q1_i, Q1_i, rst_n, netoQ[1], netoQn[1]);
    
    and(Q2_i, netoQ[0], netoQ[1]);
    JK_FF jk3_0(CLK_div, Q2_i, Q2_i, rst_n, netoQ[2], netoQn[2]);
    
    always @ *
        oQ = netoQ;
    
    wire [6:0] netDisplay;
    display7 trans(oQ, netDisplay);
    assign oDisplay = netDisplay;
    
endmodule
