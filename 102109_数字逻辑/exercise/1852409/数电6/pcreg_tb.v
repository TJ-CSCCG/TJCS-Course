`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/28 18:27:54
// Design Name: 
// Module Name: pcreg_tb
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


module pcreg_tb();
    reg clk;
    reg rst;
    reg ena;
    reg [31:0] data_in;
    wire [31:0] data_out;
    
    pcreg uul(clk, rst, ena, data_in, data_out);
    
    initial begin
        clk = 0;
        rst = 0;
        ena = 1;
        data_in = 32'h0000_0000;
    end
    
    always 
        #20 clk = ~clk;
    
    initial begin
        #20 data_in = 32'hfff0_1100;
        #10 data_in = 32'h11f1_1111;
        
        #30 ena = 0; 
        #20 data_in = 32'h1100_1ff0;
        #10 data_in = 32'h1111_11f1;
        #20 rst = 1;
        
        #40 data_in = 32'h0101_f101;
        #10 ena = 1;
        #10 data_in = 32'h0101_010f;
        #10 rst = 0;
        #10 data_in = 32'h0111_1f10;
        #20 rst = 1;
        
    end

endmodule
