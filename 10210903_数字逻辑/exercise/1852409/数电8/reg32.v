`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/28 18:20:24
// Design Name: 
// Module Name: pcreg
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


module reg32(
    input clk,
    input rst,
    input wena,
    input [31:0] wdata,
    output reg [31:0] data_out
    );
    reg [31:0] register;
    
    localparam regWidth = 32;
    integer index;
    
    always @ (posedge clk or posedge rst) begin
    // reset
        index = 0;
        if (rst) 
            data_out = 8'h0000_0000;
    // work
        else
            // write data in register
            if (wena == 1) begin 
                index = 0;
                for (index = 0; index < regWidth; index = index + 1)
                    register[index] = wdata[index];
            end
            // read from register
            else if (wena == 0) begin
                index = 0;
                for (index = 0; index < regWidth; index = index + 1)
                    data_out[index] = register[index];
            end
            else ;
    end
        
endmodule
