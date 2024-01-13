----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Mihaita Nagy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    18:46:55 03/05/2013 
-- Design Name: 
-- Module Name:    pwm - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description:
--    This module represents the 8-bit PWM component, used by the RgbLed module
--    to generate the sweeping colors
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

entity Pwm is
   port(
      clk_i : in std_logic; -- system clock = 100MHz
      data_i : in std_logic_vector(7 downto 0); -- the number to be modulated
      pwm_o : out std_logic
   );
end Pwm;

architecture Behavioral of Pwm is

signal cnt : std_logic_vector(7 downto 0);

begin

   COUNT: process(clk_i)
   begin
      if rising_edge(clk_i) then
         cnt <= cnt + '1';
      end if;
   end process COUNT;
   
   COMPARE: process(data_i, cnt)
   begin
      if unsigned(cnt) < unsigned(data_i) then
         pwm_o <= '1';
      else
         pwm_o <= '0';
      end if;
   end process COMPARE;

end Behavioral;

