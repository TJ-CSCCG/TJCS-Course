`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    19:44:06 05/22/2013 
// Design Name: 
// Module Name:    sram 
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
module sram(
    input clk,
	 input we,
	 input [7:0] color,
    input [11:0] datain,
    input [11:0] hori,
	 input [11:0] verti,
	 input [11:0] addr_w,
    output [7:0] dataout
    );
	integer k;
//	parameter n=640;
	reg [11:0] data [0:319];
	
	always @(posedge clk)begin
		if(we==1)begin
			data[addr_w>>1]<=datain;
		end
	end

	

	assign dataout=(data[hori>>1]==verti) ? color : 0;
//	assign dataout=8'hff;

endmodule
