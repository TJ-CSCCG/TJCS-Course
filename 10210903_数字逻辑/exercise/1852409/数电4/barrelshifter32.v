`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/12 19:05:15
// Design Name: 
// Module Name: barrelshifter32
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


module barrelshifter32(
    input [31:0] a,
    input [4:0] b,
    input [1:0] aluc,
    output reg [31:0] c
    );    
    
    integer i;
    always @ *
    begin
        c[31:0] = a[31:0];
        if (aluc[0] == 0)
        begin
            if (aluc[1] == 0)
            begin
                c = c >>> b;
            end
            else if (aluc[1] == 1)
            begin
                if (c[31] == 1)
                begin
                    c = c >> b;
                    for (i = 0; i < b; i = i + 1)
                        c[31 - i] = 1;
                end
                else
                    c = c >> b;
            end
            else
                ;
        end
        else if (aluc[0] == 1)
        begin 
            c = c <<< b;
        end
        else
            ;
    end
endmodule


