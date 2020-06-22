`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/14 20:17:58
// Design Name: 
// Module Name: DataCompare64_tb
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

`timescale 1ns / 1ns
module DataCompare4_tb(
    );
    reg [3:0] iData_a;
    reg [3:0] iData_b;
    reg [2:0] iData;
    wire [2:0] oData;
    
    DataCompare4 uul(.iData_a(iData_a), .iData_b(iData_b), .iData(iData), .oData(oData));
    
    initial
    begin
        iData_a = 4'b0000;
        iData_b = 4'b1010;
        iData = 3'b000;
        
        # 40
        iData_a = 4'b1010;
        iData_b = 4'b1010;
        iData = 3'b010;
        
        # 40
        iData_a = 4'b1100;
        iData_b = 4'b1010;
        iData = 3'b100;
        
        # 40
        iData_a = 4'b1001;
        iData_b = 4'b0010;
        iData = 3'b010;
        
        # 40                      
        iData_a = 4'b1010;
        iData_b = 4'b1010;
        iData = 3'b010;          
    end
endmodule
