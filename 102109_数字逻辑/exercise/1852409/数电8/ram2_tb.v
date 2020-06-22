`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/07 21:29:24
// Design Name: 
// Module Name: ram2_tb
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


module ram2_tb(    
    );
    reg clk;
    reg ena;
    reg wena;
    reg [4:0] addr;
    wire [31:0] data_io;
    reg [31:0] data_in;
    
    
    
    initial begin
        clk = 0;
        ena = 1;
        wena = 0;
        addr = 0;
        data_in = 32'hffff_ffff;
    end
    
    always
        #20 clk = ~clk;
        
    //assign data_io = (ena==1) ? data_in : 32'hzzzz_zzzz;       
    assign data_io = data_in;                
    ram2 uul(clk, ena, wena, addr, data_io);  
    

    
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
