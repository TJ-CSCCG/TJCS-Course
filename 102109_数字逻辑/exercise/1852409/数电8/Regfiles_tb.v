`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/18 19:31:00
// Design Name: 
// Module Name: Regfiles_tb
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


module Regfiles_tb(
    );
    reg clk, rst, we;
    reg [4:0] raddr1;
    reg [4:0] raddr2;
    reg [4:0] waddr;
    reg [31:0] wdata;
    wire [31:0] rdata1;
    wire [31:0] rdata2;
    
    
    initial begin 
        clk = 0; rst = 0; we = 1;
        raddr1 = 0; raddr2 = 0;
        waddr = 0;
    end
    
    always 
        #20 clk = ~clk;
        
    Regfiles uul(clk, rst, we, raddr1, raddr2, waddr, wdata, rdata1, rdata2);
    
    initial begin 
        wdata = 32'h0a2e_5486;
        #15 waddr = 10; wdata = 32'h5894_a5ff;
        #20 waddr = 7; wdata = 32'hffff_ffff;
        #20 waddr = 1; wdata = 32'h0000_0000;
        
        #20 we = 1;
        #20 raddr1 = 1; raddr2 = 7;
        #20 raddr1 = 10; raddr2 = 0;
        #5 rst = 0;
        #20 raddr1 = 5; raddr2 = 4;
        #5 rst = 0;
        #20 raddr1 = 5; raddr2 = 4;        
    end
endmodule
