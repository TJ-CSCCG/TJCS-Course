`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/14 20:52:34
// Design Name: 
// Module Name: DataCompare8
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


module DataCompare8(
    input [7:0] iData_a,
    input [7:0] iData_b,
    output [2:0] oData
    );
    wire [2:0] tmp;
    DataCompare4 kk(.iData_a(iData_a[3:0]), .iData_b(iData_b[3:0]), .iData(3'b001), .oData(tmp));
    reg [2:0] inputtmp;
    always @ *
        inputtmp = tmp;
    DataCompare4 ll(.iData_a(iData_a[7:4]), .iData_b(iData_b[7:4]), .iData(inputtmp), .oData(oData));
    
endmodule
