`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/09 20:06:22
// Design Name: 
// Module Name: logic_gates3
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


module logic_gates_3(
    input iA,
    input iB,
    output reg oAnd,
    output reg oOr,
    output reg oNot
    );
    always @ (*)
    begin
        oAnd = iA & iB;
        oOr = iA | iB;
        oNot = ~iA;
    end
endmodule
