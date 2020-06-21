`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/10/15 15:32:18
// Design Name: 
// Module Name: Adder
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


module Adder(
    input [7:0] iData_a,
    input [7:0] iData_b,
    input iC,
    output [7:0] oData,
    output oData_C
    );
    
    wire oC1;
    FA FA1(.iA(iData_a[0]), .iB(iData_b[0]), .iC(iC), .oS(oData[0]), .oC(oC1));
    
    wire oC2;
    FA FA2(.iA(iData_a[1]), .iB(iData_b[1]), .iC(oC1), .oS(oData[1]), .oC(oC2));
    
    wire oC3;
    FA FA3(.iA(iData_a[2]), .iB(iData_b[2]), .iC(oC2), .oS(oData[2]), .oC(oC3));
    
    wire oC4;
    FA FA4(.iA(iData_a[3]), .iB(iData_b[3]), .iC(oC3), .oS(oData[3]), .oC(oC4));
    
    wire oC5;
    FA FA5(.iA(iData_a[4]), .iB(iData_b[4]), .iC(oC4), .oS(oData[4]), .oC(oC5));

    wire oC6;
    FA FA6(.iA(iData_a[5]), .iB(iData_b[5]), .iC(oC5), .oS(oData[5]), .oC(oC6));
    
    wire oC7;
    FA FA7(.iA(iData_a[6]), .iB(iData_b[6]), .iC(oC6), .oS(oData[6]), .oC(oC7));

    FA FA8(.iA(iData_a[7]), .iB(iData_b[7]), .iC(oC7), .oS(oData[7]), .oC(oData_C));

endmodule
