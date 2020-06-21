`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/16 19:46:31
// Design Name: 
// Module Name: selector41
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


module selector41(
    input [3:0] iC0,
    input [3:0] iC1,
    input [3:0] iC2,
    input [3:0] iC3,
    input iS1,
    input iS0,
    output [3:0] oZ
    );
    
    assign oZ = (iS1 == 0 && iS0 == 0) ? iC0 : ((iS1 == 0 && iS0 == 1) ? iC1 : ((iS1 == 1 && iS0 == 0) ? iC2 : iC3));
    
endmodule
