`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/23 20:17:42
// Design Name: 
// Module Name: decoder
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


module decoder5To32(
    input [4:0] iData,
    input iEna,
    output reg [31:0] oData
    );
    
    always @ (iEna or iData)
    begin
        oData = 32'h0000_0000;
        if (iEna == 1)
            oData[iData[0] + 2*iData[1] + 4*iData[2] + 8*iData[3] + 16*iData[4]] = 1'b1;
    end
endmodule
