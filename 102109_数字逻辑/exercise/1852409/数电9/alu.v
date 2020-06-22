`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2019/11/11 19:07:15
// Design Name: 
// Module Name: alu
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


module alu(
    input [31:0] a,
    input [31:0] b,
    input [3:0] aluc,
    output reg [31:0] r,
    output reg zero,
    output reg carry,
    output reg negative,
    output reg overflow
    );
    integer i = 0;
    
    always @ (*) begin
    case (aluc)
    // add unsigned:zero,carry,negative
        4'b0000:begin 
            r = a + b;
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31:0] < a[31:0] && r[31:0] < b[31:0]) carry = 1;
            else carry = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // add signed:zero,negative,overflow
        4'b0010:begin 
            r = a + b;
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
            if (((a > 0 && b > 0) || (a < 0 && b < 0)) && (r[31] != a[31] || r[31] != b[31])) overflow = 1;
            else overflow = 0;
        end
    // sub unsigned:zero,carry,negative
        4'b0001:begin
            r = a - b;
            if (r == 0) zero = 1;
            else zero = 0;
            if (a < b) carry = 1;
            else carry = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // sub signed:zero,negative,overflow
        4'b0011:begin
            r = a - b;
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
            if (((a > 0 && b > 0) || (a < 0 && b < 0)) && (r[31] != a[31] || r[31] != b[31])) overflow = 1;
            else overflow = 0;
        end
    // and: zero,negative
        4'b0100:begin
            r = a & b;
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // or: zero,negative
        4'b0101:begin
            r = a | b;
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // xor: zero,negative
        4'b0110:begin
            r = a ^ b;
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // nor: zero, negative
        4'b0111:begin
            r = ~(a | b);
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // lui: zero,negative
        4'b1000, 4'b1001:begin
            r = {b[15:0], 16'b0};
            
        end
    // slt: zero,negative
        4'b1011:begin
            if (a[31] == 1 && b[31] == 1) r = (a[30:0] > b[30:0]) ? 1 : 0;
            else if (a[31] == 0 && b[31] == 0) r = (a[30:0] < b[30:0]) ? 1 : 0;
            else if (a[31] == 0 && b[31] == 1) r = 0;
            else r = 1;
            
            if (a == b) zero = 1;
            else zero = 0;
            if (a < b) negative = 1;
            else negative = 0;
        end
    // slt unsigned: zero,negative
        4'b1010:begin
            r = (a < b) ? 1 : 0;
            if (a == b) zero = 1;
            else zero = 0;
            if (a < b) negative = 1;
            else negative = 0;
        end
    // sra: zero,carry,negative
        4'b1100:begin
            carry = b[a-1];
            // set 1 or 0
            if (b[31] == 1) begin 
            r= b >> a;
            for (i = 0; i < a; i = i + 1)
                r[31 - i] = 1;
            end
            else begin 
            r = b >> a; 
            for (i = 0; i < a; i = i + 1)
                r[31 - i] = 0;
            end
            
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // sll/slr: zero,carry,negative
        4'b1110, 4'b1111:begin
            if (a != 0)  carry = b[32-a];
            else carry = 0;
            r = b << a;
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    // srl: zero,carry,negative
        4'b1101:begin
            carry = b[a-1];
            r = b >> a;
            // set 0
            for (i = 0; i < a; i = i + 1)
                r[31 - i] = 0;
                
            if (r == 0) zero = 1;
            else zero = 0;
            if (r[31] == 1) negative = 1;
            else negative = 0;
        end
    default:
        begin r = 0;end
    endcase
    end
    
endmodule
