`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/07 21:20:56
// Design Name: 
// Module Name: ram2
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


module ram2(
    input clk,
    input ena,
    input wena,
    input [4:0] addr,
    inout [31:0] data
    );
    
    reg register[0:1023];
    reg [31:0] data_io;
    
    localparam nodeNum = 1024;
    localparam busWidth = 32;
    integer index;
    
    always @ (posedge clk) begin
        if (ena == 1) begin
            if (wena == 1) begin
                for (index = 0; index < busWidth; index = index + 1)
                    register[addr * busWidth + index] = data[index];
            end
            else if (wena == 0) begin
                for (index = 0; index < busWidth; index = index + 1)
                    data_io[index] = register[addr * busWidth + index];
            end
            else ;
        end
        else ;
    end
    
    assign data = ena ? data_io : 32'hzzzz_zzzz;
    
endmodule
