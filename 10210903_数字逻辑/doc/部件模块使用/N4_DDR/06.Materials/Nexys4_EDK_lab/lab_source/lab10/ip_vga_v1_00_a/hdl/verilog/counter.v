`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    20:37:05 05/22/2013 
// Design Name: 
// Module Name:    counter 
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
module counter(
    input clk25mhz,
	 output [11:0] hori,
	 output [11:0] verti,
    output hs,
    output vs
    );
	 
	 reg [11:0] x=0;
	 reg [11:0] y=0;
	 reg h_en;
	 reg v_en;
	 reg h;
	 reg v;
	 reg [11:0] ho;
	 reg [11:0] ve;
	 reg [11:0] add=0;
	 
	always @(posedge clk25mhz)begin
		if(x>=0 && x<96)begin
			x<=x+1;
		end
		else if(x>=96 && x<144)begin
			x<=x+1;
			h<=1;
		end
		else if(x>=144 && x<784)begin
			x<=x+1;
			h<=1;
			h_en<=1;
		end
		else if(x>=784 && x<800)begin
			x<=x+1;
			h_en<=0;
			h<=1;
		end
		else begin
			x<=0;
			h<=0;
			h_en<=0;
		end
			
		if(x==800)begin
			if(y>=0 && y<2)
				y<=y+1;
			else if(y>=2 && y<35)begin
				v<=1;
				y<=y+1;
			end
			else if(y>=35 && y<515)begin
				v<=1;
				y<=y+1;
				v_en<=1;
			end
			else if(y>=515 && y<525)begin
				y<=y+1;
				v_en<=0;
					v<=1;
			end
			else begin
				y<=0;
				v<=0;
				v_en<=0;
			end
		end
			
		if(v_en==1 && h_en==1)begin
			if(ho==639)begin
				ho<=0;
				if(ve==479)
					ve<=0;
				else
					ve<=ve+1;							
			end
			else
				ho<=ho+1;
		end
	end
		
		
	
	assign hori=ho;
	assign verti=ve;
	assign hs=h;
	assign vs=v;


endmodule
