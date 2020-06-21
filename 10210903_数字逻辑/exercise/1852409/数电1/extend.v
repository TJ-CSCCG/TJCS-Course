`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/09 21:01:00
// Design Name: 
// Module Name: extend
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


module extend #(parameter WIDTH = 16)(
    input [WIDTH - 1:0] a,
    input sext,
    output [31:0] b
    );
    assign b = sext? {{(32 - WIDTH){a[WIDTH - 1]}}, a} : {27'b0, a};
endmodule
