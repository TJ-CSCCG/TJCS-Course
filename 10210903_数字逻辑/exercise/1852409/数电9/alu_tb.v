`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/11 20:20:27
// Design Name: 
// Module Name: alu_tb
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


module alu_tb(
    );
    reg [31:0] a;
    reg [31:0] b;
    reg [3:0] aluc;
    wire [31:0] r;
    wire zero, carry, negative, overflow;
    
    alu uul(a, b, aluc, r, zero, carry, negative, overflow);
    
    integer i = 0;
    parameter aluc_max = 16;
    initial begin
        a = 32'h8fa1_6b42;
        b = 32'hba18_5e29;
        aluc = 0;
        for (i = 1; i < aluc_max; i = i + 1) begin
            if (i == 15 || i == 14 || i == 13) a = 4;
            else a = 32'h8fa1_6b42;
            #30 aluc = i;
        end
        
    end
endmodule
