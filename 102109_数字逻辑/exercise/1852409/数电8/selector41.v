`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/16 19:46:31
// Design Name: 
// Module Name: selector41
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


module selector32To1(
    input [31:0] iC0,
    input [31:0] iC1,
    input [31:0] iC2,
    input [31:0] iC3,
    input [31:0] iC4,
    input [31:0] iC5,
    input [31:0] iC6,
    input [31:0] iC7,
    input [31:0] iC8,
    input [31:0] iC9,
    input [31:0] iC10,
    input [31:0] iC11,
    input [31:0] iC12,
    input [31:0] iC13,
    input [31:0] iC14,
    input [31:0] iC15,
    input [31:0] iC16,
    input [31:0] iC17,
    input [31:0] iC18,
    input [31:0] iC19,
    input [31:0] iC20,
    input [31:0] iC21,
    input [31:0] iC22,
    input [31:0] iC23,
    input [31:0] iC24,
    input [31:0] iC25,
    input [31:0] iC26,
    input [31:0] iC27,
    input [31:0] iC28,
    input [31:0] iC29,
    input [31:0] iC30,
    input [31:0] iC31,
    
    input [4:0] raddr,
    input rena,
    output reg [31:0] rdata
    );
    
    always @ (*) begin
        if (rena == 1) begin
        case(raddr)
            5'b00000: rdata = iC0;
            5'b00001: rdata = iC1;
            5'b00010: rdata = iC2;
            5'b00011: rdata = iC3;
            5'b00100: rdata = iC4;
            5'b00101: rdata = iC5;
            5'b00110: rdata = iC6;
            5'b00111: rdata = iC7;
            5'b01000: rdata = iC8;
            5'b01001: rdata = iC9;
            5'b01010: rdata = iC10;
            5'b01011: rdata = iC11;
            5'b01100: rdata = iC12;
            5'b01101: rdata = iC13;
            5'b01110: rdata = iC14;
            5'b01111: rdata = iC15;
            5'b10000: rdata = iC16;
            5'b10001: rdata = iC17;
            5'b10010: rdata = iC18;
            5'b10011: rdata = iC19;
            5'b10100: rdata = iC20;
            5'b10101: rdata = iC21;
            5'b10110: rdata = iC22;
            5'b10111: rdata = iC23;
            5'b11000: rdata = iC24;
            5'b11001: rdata = iC25;
            5'b11010: rdata = iC26;
            5'b11011: rdata = iC27;
            5'b11100: rdata = iC28;
            5'b11101: rdata = iC29;
            5'b11110: rdata = iC30;
            5'b11111: rdata = iC31;
            default: rdata = 0;
        endcase
        end
        else ;
    end
    
endmodule
