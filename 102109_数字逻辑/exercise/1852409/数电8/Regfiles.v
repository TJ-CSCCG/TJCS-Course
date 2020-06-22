`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/18 19:02:13
// Design Name: 
// Module Name: Regfiles
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


module Regfiles(
    input clk,
    input rst,
    input we,
    input [4:0] raddr1,
    input [4:0] raddr2,
    input [4:0] waddr,
    input [31:0] wdata,
    output reg [31:0] rdata1,
    output reg [31:0] rdata2
    );

    wire [31:0] out_decoder;
    decoder5To32 decoder(waddr, we, out_decoder);
    
    wire [31:0] out_reg0;
    wire [31:0] out_reg1; wire [31:0] out_reg2; wire [31:0] out_reg3; wire [31:0] out_reg4; wire [31:0] out_reg5; wire [31:0] out_reg6; wire [31:0] out_reg7; wire [31:0] out_reg8; wire [31:0] out_reg9; wire [31:0] out_reg10; wire [31:0] out_reg11;
    wire [31:0] out_reg12; wire [31:0] out_reg13; wire [31:0] out_reg14; wire [31:0] out_reg15; wire [31:0] out_reg16; wire [31:0] out_reg17; wire [31:0] out_reg18; wire [31:0] out_reg19; wire [31:0] out_reg20; wire [31:0] out_reg21; wire [31:0] out_reg22;
    wire [31:0] out_reg23; wire [31:0] out_reg24; wire [31:0] out_reg25; wire [31:0] out_reg26; wire [31:0] out_reg27; wire [31:0] out_reg28; wire [31:0] out_reg29; wire [31:0] out_reg30; wire [31:0] out_reg31;
    
    reg32 r0(clk, rst, out_decoder[0], wdata, out_reg0);
    reg32 r1(clk, rst, out_decoder[1], wdata, out_reg1);   
    reg32 r2(clk, rst, out_decoder[2], wdata, out_reg2);    
    reg32 r6(clk, rst, out_decoder[3], wdata, out_reg3);    
    reg32 r3(clk, rst, out_decoder[4], wdata, out_reg4);    
    reg32 r4(clk, rst, out_decoder[5], wdata, out_reg5);    
    reg32 r5(clk, rst, out_decoder[6], wdata, out_reg6);    
    reg32 r7(clk, rst, out_decoder[7], wdata, out_reg7);    
    reg32 r8(clk, rst, out_decoder[8], wdata, out_reg8);    
    reg32 r9(clk, rst, out_decoder[9], wdata, out_reg9);    
    reg32 r10(clk, rst, out_decoder[10], wdata, out_reg10);  
    reg32 r11(clk, rst, out_decoder[11], wdata, out_reg11);    
    reg32 r12(clk, rst, out_decoder[12], wdata, out_reg12);    
    reg32 r13(clk, rst, out_decoder[13], wdata, out_reg13);    
    reg32 r14(clk, rst, out_decoder[14], wdata, out_reg14);    
    reg32 r15(clk, rst, out_decoder[15], wdata, out_reg15);    
    reg32 r16(clk, rst, out_decoder[16], wdata, out_reg16);    
    reg32 r17(clk, rst, out_decoder[17], wdata, out_reg17);    
    reg32 r18(clk, rst, out_decoder[18], wdata, out_reg18);    
    reg32 r19(clk, rst, out_decoder[19], wdata, out_reg19);    
    reg32 r20(clk, rst, out_decoder[20], wdata, out_reg20);    
    reg32 r21(clk, rst, out_decoder[21], wdata, out_reg21);    
    reg32 r22(clk, rst, out_decoder[22], wdata, out_reg22);    
    reg32 r23(clk, rst, out_decoder[23], wdata, out_reg23);    
    reg32 r24(clk, rst, out_decoder[24], wdata, out_reg24);    
    reg32 r25(clk, rst, out_decoder[25], wdata, out_reg25);    
    reg32 r26(clk, rst, out_decoder[26], wdata, out_reg26);    
    reg32 r27(clk, rst, out_decoder[27], wdata, out_reg27);    
    reg32 r28(clk, rst, out_decoder[28], wdata, out_reg28);    
    reg32 r29(clk, rst, out_decoder[29], wdata, out_reg29);   
    reg32 r30(clk, rst, out_decoder[30], wdata, out_reg30);    
    reg32 r31(clk, rst, out_decoder[31], wdata, out_reg31);  
    
    wire [31:0] wrdata1;
    wire [31:0] wrdata2;
    selector32To1 select1(out_reg0, out_reg1, out_reg2, out_reg3, out_reg4, out_reg5, out_reg6, out_reg7, out_reg8, out_reg9, out_reg10, out_reg11,
          out_reg12, out_reg13, out_reg14, out_reg15, out_reg16, out_reg17, out_reg18, out_reg19, out_reg20, out_reg21, out_reg22,
          out_reg23, out_reg24, out_reg25, out_reg26, out_reg27, out_reg28, out_reg29, out_reg30, out_reg31,
          raddr1, ~we, wrdata1);
    selector32To1 select2(out_reg0, out_reg1, out_reg2, out_reg3, out_reg4, out_reg5, out_reg6, out_reg7, out_reg8, out_reg9, out_reg10, out_reg11,
          out_reg12, out_reg13, out_reg14, out_reg15, out_reg16, out_reg17, out_reg18, out_reg19, out_reg20, out_reg21, out_reg22,
          out_reg23, out_reg24, out_reg25, out_reg26, out_reg27, out_reg28, out_reg29, out_reg30, out_reg31,
          raddr2, ~we, wrdata2);
          
    always @ * begin
        rdata1 = wrdata1;
        rdata2 = wrdata2;
    end
    
endmodule
