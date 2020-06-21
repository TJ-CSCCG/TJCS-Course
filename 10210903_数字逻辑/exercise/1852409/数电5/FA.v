`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/15 14:45:03
// Design Name: 
// Module Name: FA
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


module FA(
    input iA,
    input iB,
    input iC,
    output oS,
    output oC
    );
    
    wire inputA1, inputB1;
    wire inputA2, inputB2;
    wire oxA1B1;
    wire oaA1B1C;
    wire oaA1B1;
    assign inputA1 = iA;
    assign inputA2 = iA;
    assign inputB1 = iB;
    assign inputB2 = iB;
    
    // 0 1 1
    xor xorA1B1(oxA1B1, inputA1, inputB1);
    // 1 0 1
    xor xorOA1B1C(oS, oxA1B1, iC);
    // 0 0 1
    and andA1B1C(oaA1B1C, oxA1B1, iC);
    // 1 1 1
    and andA1B1(oaA1B1, inputA2, inputB2);
    // 1 1 0
    or orA1B1A1B1C(oC, oaA1B1, oaA1B1C);
     
endmodule
