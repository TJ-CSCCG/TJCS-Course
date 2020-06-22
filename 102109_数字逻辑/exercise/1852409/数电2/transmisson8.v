`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/23 19:17:35
// Design Name: 
// Module Name: transmisson8
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



module transmission8(
    input [7:0] iData,
    input A,
    input B,
    input C,
    output reg [7:0] oData
    );
    always @ (*)
    begin
        oData[A*4 + B*2 + C] = iData[A*4 + B*2 + C];
    end
    
endmodule
