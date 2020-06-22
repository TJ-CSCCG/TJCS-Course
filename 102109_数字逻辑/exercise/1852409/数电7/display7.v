`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/23 20:54:52
// Design Name: 
// Module Name: display7
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


module display7(
    input [2:0] iData,
    output reg [6:0] oData
    );
    
    always @ (iData)
    begin
        if (iData[2] == 0 && iData[1] == 0 && iData[0] == 0)
            oData = 7'b100_0000;
        if (iData[2] == 0 && iData[1] == 0 && iData[0] == 1)
            oData = 7'b111_1001;
        if (iData[2] == 0 && iData[1] == 1 && iData[0] == 0)
            oData = 7'b010_0100;
        if (iData[2] == 0 && iData[1] == 1 && iData[0] == 1)
            oData = 7'b011_0000;
        if (iData[2] == 1 && iData[1] == 0 && iData[0] == 0)
            oData = 7'b001_1001;
        if (iData[2] == 1 && iData[1] == 0 && iData[0] == 1)
            oData = 7'b001_0010;
        if (iData[2] == 1 && iData[1] == 1 && iData[0] == 0)
            oData = 7'b000_0010;
        if (iData[2] == 1 && iData[1] == 1 && iData[0] == 1)
            oData = 7'b111_1000;
    end
endmodule
