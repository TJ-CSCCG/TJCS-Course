`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/14 21:00:07
// Design Name: 
// Module Name: DataCompare8_tb
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


module DataCompare8_tb();
    reg [7:0] iData_a;
    reg [7:0] iData_b;
    wire [2:0] oData;
    
    DataCompare8 uul(.iData_a(iData_a), .iData_b(iData_b), .oData(oData));
    
    initial 
    begin
        # 40
        iData_a = 8'b0000_0000;
        iData_b = 8'b0000_0000;
        
        # 40
        iData_a = 8'b0000_0001;
        iData_b = 8'b0000_0000;
        
        # 40
        iData_a = 8'b0000_0000;
        iData_b = 8'b0000_0010;
        
        # 40
        iData_a = 8'b1010_0101;
        iData_b = 8'b0101_1010;
        
        # 40
        iData_a = 8'b1111_1111;
        iData_b = 8'b0000_1111;
        
        # 40
        iData_a = 8'b1111_1111;
        iData_b = 8'b1111_0000;
        
        # 40
        iData_a = 8'b0101_0110;
        iData_b = 8'b1100_1001;
    end
endmodule
