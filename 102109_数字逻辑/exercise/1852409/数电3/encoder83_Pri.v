`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/09/30 20:45:21
// Design Name: 
// Module Name: encoder83_Pri
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


module encoder83_Pri(
    input [7:0] iData,
    input iEI,
    output reg [2:0] oData,
    output reg oEO
    );
    always @ (*)
    begin
        if (iEI == 1)
        begin
            oData = 3'b111;
            oEO = 0;
        end
        else
        begin
            oEO = 1;
            if (iData == 8'b11111111)
            begin
                oEO = 0;
                oData = 3'b111;
            end
            else if (iData[0] == 0)
                oData = 3'b000;
            else if (iData[1:0] == 2'b01)
                oData = 3'b001;
            else if (iData[2:0] == 3'b011)
                oData = 3'b010;
            else if (iData[3:0] == 4'b0111)
                oData = 3'b011;
            else if (iData[4:0] == 5'b01111)
                oData = 3'b100;
            else if (iData[5:0] == 6'b011111)
                oData = 3'b101;
            else if (iData[6:0] == 7'b0111111)
                oData = 3'b110;
            else if (iData[7:0] == 8'b01111111)
                oData = 3'b111;  
            else;  
        end
    end
endmodule
