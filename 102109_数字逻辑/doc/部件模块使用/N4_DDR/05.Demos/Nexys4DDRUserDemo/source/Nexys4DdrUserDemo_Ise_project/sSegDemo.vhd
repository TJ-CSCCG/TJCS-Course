----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Mihaita Nagy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    09:53:08 03/07/2013 
-- Design Name: 
-- Module Name:    sSegDemo - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--    This module represents the Seven-Segment display demo
--    that displays a moving pattern on the 8-digit seven segment display
--
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_signed.all;

entity sSegDemo is
   port(
      clk_i : in std_logic;
      rstn_i : in std_logic;
      seg_o : out std_logic_vector(7 downto 0);
      an_o  : out std_logic_vector(7 downto 0)
   );
end sSegDemo;

architecture Behavioral of sSegDemo is

-- Seven-Segment display multiplexer
component sSegDisplay is
port(
   ck       : in  std_logic;                     -- 100Mhz system clock
   number   : in  std_logic_vector(63 downto 0); -- eght digit hex data to be displayed, active-low
   seg      : out std_logic_vector(7 downto 0);  -- display cathodes
   an       : out std_logic_vector(7 downto 0)); -- display anodes (active-low, due to transistor complementing)
end component;

-- Clock division constant
-- The pattern will move on the seven-segment display at
-- a frequency of 100MHz/8000000 = 12.5Hz
constant CLK_DIV : integer := 8000000;
-- Clock divider counter and signal
signal clkCnt : integer := 0;
signal slowClk : std_logic;

-- dispVal represents the pattern to be displayed
-- The pattern changes i.e. moves according to the value of slowCnt
signal dispVal : std_logic_vector(63 downto 0);
signal slowCnt : integer := 0;

begin

   -- Seven-Segment display multiplexer instantiation
   Disp: sSegDisplay
   port map(
      ck       => clk_i,
      number   => dispVal, -- 64-bit
      seg      => seg_o,
      an       => an_o);
   
   -- clock prescaler
   Prescaler: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if rstn_i = '0' then
            clkCnt <= 0;
         elsif clkCnt = CLK_DIV-1 then
            clkCnt <= 0;
         else
            clkCnt <= clkCnt + 1;
         end if;
      end if;
   end process Prescaler;
   
   slowClk <= '1' when clkCnt = CLK_DIV-1 else '0';

-- slow counter
   ProcCnt: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if rstn_i = '0' then
            slowCnt <= 0;
         elsif slowClk = '1' then
            if slowCnt = 98 then
               slowCnt <= 15;
            else
               slowCnt <= slowCnt + 1;
            end if;
         end if;
      end if;
   end process ProcCnt;
   
   -- create the 64-bit memory that holds the pattern for dispVal
   with slowCnt select
      dispVal <=  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"fe" when 0,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"fe" & x"fe" when 1,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"fe" & x"fe" & x"fe" when 2,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"fe" & x"fe" & x"fe" & x"fe" when 3,
                  x"ff" & x"ff" & x"ff" & x"fe" & x"fe" & x"fe" & x"fe" & x"fe" when 4,
                  x"ff" & x"ff" & x"fe" & x"fe" & x"fe" & x"fe" & x"fe" & x"ff" when 5,
                  x"ff" & x"fe" & x"fe" & x"fe" & x"fe" & x"fe" & x"ff" & x"ff" when 6,
                  x"fe" & x"fe" & x"fe" & x"fe" & x"fe" & x"ff" & x"ff" & x"ff" when 7,
                  x"de" & x"fe" & x"fe" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" when 8,
                  x"ce" & x"fe" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 9,
                  x"86" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 10,
                  x"86" & x"77" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 11,
                  x"87" & x"77" & x"77" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 12,
                  x"67" & x"77" & x"77" & x"77" & x"ff" & x"ff" & x"ff" & x"ff" when 13,
                  x"77" & x"77" & x"77" & x"77" & x"77" & x"ff" & x"ff" & x"ff" when 14,
                  x"ff" & x"77" & x"77" & x"77" & x"77" & x"77" & x"ff" & x"ff" when 15,
                  x"ff" & x"ff" & x"77" & x"77" & x"77" & x"77" & x"77" & x"ff" when 16,
                  x"ff" & x"ff" & x"ff" & x"77" & x"77" & x"77" & x"77" & x"77" when 17,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"77" & x"77" & x"77" & x"73" when 18,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"77" & x"77" & x"71" when 19,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"77" & x"70" when 20,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"50" when 21,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"c8" when 22,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"f7" & x"cc" when 23,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"e7" & x"ce" when 24,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"c7" & x"cf" when 25,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"fe" & x"c7" & x"ef" when 26,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"de" & x"c7" & x"ff" when 27,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ce" & x"cf" & x"ff" when 28,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"f7" & x"ce" & x"df" & x"ff" when 29,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"e7" & x"ce" & x"ff" & x"ff" when 30,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"c7" & x"cf" & x"ff" & x"ff" when 31,
                  x"ff" & x"ff" & x"ff" & x"fe" & x"c7" & x"ef" & x"ff" & x"ff" when 32,
                  x"ff" & x"ff" & x"ff" & x"de" & x"c7" & x"ff" & x"ff" & x"ff" when 33,
                  x"ff" & x"ff" & x"ff" & x"ce" & x"cf" & x"ff" & x"ff" & x"ff" when 34,
                  x"ff" & x"ff" & x"f7" & x"ce" & x"df" & x"ff" & x"ff" & x"ff" when 35,
                  x"ff" & x"ff" & x"e7" & x"ce" & x"ff" & x"ff" & x"ff" & x"ff" when 36,
                  x"ff" & x"ff" & x"c7" & x"cf" & x"ff" & x"ff" & x"ff" & x"ff" when 37,
                  x"ff" & x"fe" & x"c7" & x"ef" & x"ff" & x"ff" & x"ff" & x"ff" when 38,
                  x"ff" & x"de" & x"c7" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 39,
                  x"ff" & x"ce" & x"cf" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 40,
                  x"f7" & x"ce" & x"df" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 41,
                  x"e7" & x"ce" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 42,
                  x"a7" & x"cf" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 43,
                  x"a7" & x"af" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 44,
                  x"a7" & x"bf" & x"bf" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 45,
                  x"af" & x"bf" & x"bf" & x"bf" & x"ff" & x"ff" & x"ff" & x"ff" when 46,
                  x"bf" & x"bf" & x"bf" & x"bf" & x"bf" & x"ff" & x"ff" & x"ff" when 47,
                  x"ff" & x"bf" & x"bf" & x"bf" & x"bf" & x"bf" & x"ff" & x"ff" when 48,
                  x"ff" & x"ff" & x"bf" & x"bf" & x"bf" & x"bf" & x"bf" & x"ff" when 49,
                  x"ff" & x"ff" & x"ff" & x"bf" & x"bf" & x"bf" & x"bf" & x"bf" when 50,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"bf" & x"bf" & x"bf" & x"bd" when 51,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"bf" & x"bf" & x"bc" when 52,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"be" & x"bc" when 53,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"fe" & x"fe" & x"bc" when 54,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"fe" & x"fe" & x"fe" & x"fc" when 55,
                  x"ff" & x"ff" & x"ff" & x"fe" & x"fe" & x"fe" & x"fe" & x"fe" when 56,
                  x"ff" & x"ff" & x"fe" & x"fe" & x"fe" & x"fe" & x"fe" & x"ff" when 57,
                  x"ff" & x"fe" & x"fe" & x"fe" & x"fe" & x"fe" & x"ff" & x"ff" when 58,
                  x"fe" & x"fe" & x"fe" & x"fe" & x"fe" & x"ff" & x"ff" & x"ff" when 59,
                  x"de" & x"fe" & x"fe" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" when 60,
                  x"ce" & x"fe" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 61,
                  x"c6" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 62,
                  x"c2" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 63,
                  x"c1" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 64,
                  x"e1" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 65,
                  x"f1" & x"fc" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 66,
                  x"f9" & x"f8" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 67,
                  x"fd" & x"f8" & x"f7" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 68,
                  x"ff" & x"f8" & x"f3" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 69,
                  x"ff" & x"f9" & x"f1" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 70,
                  x"ff" & x"fb" & x"f1" & x"fe" & x"ff" & x"ff" & x"ff" & x"ff" when 71,
                  x"ff" & x"ff" & x"f1" & x"fc" & x"ff" & x"ff" & x"ff" & x"ff" when 72,
                  x"ff" & x"ff" & x"f9" & x"f8" & x"ff" & x"ff" & x"ff" & x"ff" when 73,
                  x"ff" & x"ff" & x"fd" & x"f8" & x"f7" & x"ff" & x"ff" & x"ff" when 74,
                  x"ff" & x"ff" & x"ff" & x"f8" & x"f3" & x"ff" & x"ff" & x"ff" when 75,
                  x"ff" & x"ff" & x"ff" & x"f9" & x"f1" & x"ff" & x"ff" & x"ff" when 76,
                  x"ff" & x"ff" & x"ff" & x"fb" & x"f1" & x"fe" & x"ff" & x"ff" when 77,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"f1" & x"fc" & x"ff" & x"ff" when 78,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"f9" & x"f8" & x"ff" & x"ff" when 79,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"fd" & x"f8" & x"f7" & x"ff" when 80,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"f8" & x"f3" & x"ff" when 81,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"f9" & x"f1" & x"ff" when 82,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"fb" & x"f1" & x"fe" when 83,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"f1" & x"fc" when 84,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"f9" & x"bc" when 85,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"bd" & x"bc" when 86,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"bf" & x"bf" & x"bc" when 87,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"bf" & x"bf" & x"bf" & x"bd" when 88,
                  x"ff" & x"ff" & x"ff" & x"bf" & x"bf" & x"bf" & x"bf" & x"bf" when 89,
                  x"ff" & x"ff" & x"bf" & x"bf" & x"bf" & x"bf" & x"bf" & x"ff" when 90,
                  x"ff" & x"bf" & x"bf" & x"bf" & x"bf" & x"bf" & x"ff" & x"ff" when 91,
                  x"bf" & x"bf" & x"bf" & x"bf" & x"bf" & x"ff" & x"ff" & x"ff" when 92,
                  x"af" & x"bf" & x"bf" & x"bf" & x"ff" & x"ff" & x"ff" & x"ff" when 93,
                  x"27" & x"bf" & x"bf" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 94,
                  x"27" & x"37" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 95,
                  x"27" & x"77" & x"77" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when 96,
                  x"67" & x"77" & x"77" & x"77" & x"ff" & x"ff" & x"ff" & x"ff" when 97,
                  x"77" & x"77" & x"77" & x"77" & x"77" & x"ff" & x"ff" & x"ff" when 98,
                  x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" & x"ff" when others;

end Behavioral;

