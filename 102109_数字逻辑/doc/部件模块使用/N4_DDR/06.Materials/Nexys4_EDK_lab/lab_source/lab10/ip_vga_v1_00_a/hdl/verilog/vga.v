`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    20:55:18 05/22/2013 
// Design Name: 
// Module Name:    vga 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module vga(
    input clk,
	 input [1:0] we, 
	 input [7:0] color,
	 input [11:0] addr_w,
	 input [11:0] datain,
    output [2:0] red,
    output [2:0] green,
    output [1:0] blue,
    output hs,
    output vs
    );
	 

	 reg [11:0] shift;
	 wire clk25mhz;
	 wire [11:0] hori;
	 wire [11:0] verti;
	 wire [7:0] data;
	 
	 clk_div clkout(
		.clk(clk),
		.clk25mhz(clk25mhz)
	 );
	 
	 counter cout(
		.clk25mhz(clk25mhz),
		.hori(hori),
		.verti(verti),
		.hs(hs),
		.vs(vs)
	 );
	 
	 sram bram(
		.clk(clk),
		.we(we[0]),
		.color(color),
		.datain(datain), 
		.hori(hori),
		.verti(verti),
		.addr_w(addr_w),
		.dataout(data)
	 );

	
	assign red=(we[1]==1)? data[7:5] : 3'b000;
	assign green=(we[1]==1)? data[4:2] : 3'b000;
	assign blue=(we[1]==1)? data[1:0] : 2'b00;
endmodule
