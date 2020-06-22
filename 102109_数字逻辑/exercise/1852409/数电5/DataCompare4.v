`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/14 19:59:45
// Design Name: 
// Module Name: DataCompare4
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


module DataCompare4(
    input [3:0] iData_a,
    input [3:0] iData_b,
    input [2:0] iData,
    output reg [2:0] oData
    );
    
    always @ *
    begin
        if (iData_a == iData_b)
            oData = iData;
        else if (iData_a < iData_b)
            oData = 3'b010;
        else if (iData_a > iData_b)
            oData = 3'b100;
        else;
    end
    
endmodule
