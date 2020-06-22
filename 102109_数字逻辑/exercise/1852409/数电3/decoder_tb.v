`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/23 20:31:09
// Design Name: 
// Module Name: decoder_tb
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

`timescale 1ns / 1ns
module decoder_tb();
    
    reg [1:0] iEna;
    reg [2:0] iData;
    wire [7:0] oData;
    
    decoder uul(.iData(iData), .iEna(iEna), .oData(oData));
    
    initial
    begin
        iEna = 2'b01;
        iData = 3'b001;
        
        #40 
        iEna = 2'b10;
        iData = 3'b010;
        #40 iData = 3'b011;
        
        
        #40 iEna = 2'b11;
        #40 iEna = 2'b00;
        #40 iData = 3'b111;
    end
endmodule
