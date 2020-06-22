----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Elod Gyorgy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    13:59:06 04/04/2011 
-- Design Name: 
-- Module Name:    LocalRst - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--    This module generates a synchronous reset signal with a length specified
--    by the RESET_PERIOD parameter
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity LocalRst is
	 Generic ( RESET_PERIOD : natural := 4);
    Port ( RST_I : in  STD_LOGIC;
           CLK_I : in  STD_LOGIC;
           SRST_O : out  STD_LOGIC);
end LocalRst;

architecture Behavioral of LocalRst is
signal RstQ : std_logic_vector(RESET_PERIOD downto 0) := (others => '1');
begin

RstQ(0) <= '0';

RESET_LINE: for i in 1 to RESET_PERIOD generate
	process(CLK_I, RST_I)
	begin
		if (RST_I = '1') then
			RstQ(i) <= '1';
		elsif Rising_Edge(CLK_I) then
			RstQ(i) <= RstQ(i-1);
		end if;
	end process;
end generate;

SRST_O <= RstQ(RESET_PERIOD);

end Behavioral;

