`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/07 19:21:05
// Design Name: 
// Module Name: ram
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


module ram(
    input clk,
    input ena,
    input wena,
    input [4:0] addr,
    input [31:0] data_in,
    output reg [31:0] data_out
    );
    reg [1023:0] register;
    always @ (posedge clk) begin
        if (ena) begin
            if (wena)
                register[addr * 32 +: 32] = data_in[0 +: 32];
            else
                data_out[0 +: 32] = register[addr * 32 +: 32];
        end
        else
            data_out = 32'hzzzz_zzzz;
    end
endmodule
