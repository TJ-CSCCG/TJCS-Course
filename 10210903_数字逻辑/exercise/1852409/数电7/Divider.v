`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/28 21:58:37
// Design Name: 
// Module Name: Divider
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


module Divider(
    input I_CLK,
    input rst,
    output reg O_CLK
    );
    
    parameter seg = 50000000;
    integer index = 0;
    initial O_CLK <= 0;
    
    always @ (posedge I_CLK or posedge rst) begin
        if (rst)
            O_CLK <= 0;
        else if (index < seg)
            index <= index + 1;
        else begin
            index <= 0;
            O_CLK <= ~O_CLK;
        end
    end
endmodule
