`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/07 19:50:16
// Design Name: 
// Module Name: ram_tb
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


module ram_tb();
    
    reg clk, ena, wena;
    reg [4:0] addr;
    reg [31:0] data_in;
    wire [31:0] data_out;
    
    
    initial begin
        clk = 0;
        ena = 1;
        wena = 0;
        addr = 0;
        data_in = 32'hffff_ffff;
    end
    
    always
        #20 clk = ~clk;
        
    ram uul(clk,ena,wena,addr,data_in,data_out);
    

    
    initial begin
        addr = 1;
        #50 wena = 1;
        #70 addr = 10; data_in = 32'hab10_4588;
        #50 wena = 0;
        #130 ena = 1;
        #50 addr = 8; data_in = 32'h0000_0010;
        #10 ena = 0;
        #30 addr = 7;
        #20 wena = 0;
        #30 addr = 6; data_in = 32'h7896_1255;
        #20 wena = 1;
        #100 ena = 1;
    end
    
    
endmodule
