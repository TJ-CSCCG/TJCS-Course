`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/09 20:01:12
// Design Name: 
// Module Name: logic_gates
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


module logic_gates_1(
    input iA,
    input iB,
    output oAnd,
    output oOr,
    output oNot
    );
    and(oAnd, iA, iB);
    or(oOr, iA, iB);
    not(oNot, iA);
endmodule
