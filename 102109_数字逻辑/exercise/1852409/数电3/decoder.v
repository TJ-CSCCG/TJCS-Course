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


module decoder(
    input [2:0] iData,
    input [1:0] iEna,
    output reg [7:0] oData
    );
    
    always @ (iEna, iData)
    begin
        oData = 8'b1111_1111;
        if (iEna[1] == 1 && iEna[0] == 0)
            oData[iData[0] + 2*iData[1] + 4*iData[2]] = 1'b0;
    end
    
endmodule
