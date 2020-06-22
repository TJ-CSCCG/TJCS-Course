----------------------------------------------------------------------------
--	debouncer.vhd -- Signal Debouncer
----------------------------------------------------------------------------
-- Author:  Sam Bobrowicz
--          Copyright 2011 Digilent, Inc.
----------------------------------------------------------------------------
--
----------------------------------------------------------------------------
-- This component is used to debounce signals. It is designed to
-- independently debounce a variable number of signals, the number of which
-- are set using the PORT_WIDTH generic. Debouncing is done by only 
-- registering a change in a button state if it remains constant for 
-- the number of clocks determined by the DEBNC_CLOCKS generic. 
--         				
-- Generic Descriptions:
--
--   PORT_WIDTH - The number of signals to debounce. determines the width
--                of the SIGNAL_I and SIGNAL_O std_logic_vectors
--   DEBNC_CLOCKS - The number of clocks (CLK_I) to wait before registering
--                  a change.
--
-- Port Descriptions:
--
--   SIGNAL_I - The input signals. A vector of width equal to PORT_WIDTH
--   CLK_I  - Input clock
--   SIGNAL_O - The debounced signals. A vector of width equal to PORT_WIDTH
--   											
----------------------------------------------------------------------------
--
----------------------------------------------------------------------------
-- Revision History:
--  08/08/2011(SamB): Created using Xilinx Tools 13.2
--  08/29/2013(SamB): Improved reuseability by using generics
----------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_unsigned.all;
USE IEEE.NUMERIC_STD.ALL;
use IEEE.math_real.all;

entity debouncer is
    Generic ( DEBNC_CLOCKS : INTEGER range 2 to (INTEGER'high) := 2**16;
              PORT_WIDTH : INTEGER range 1 to (INTEGER'high) := 5);
    Port ( SIGNAL_I : in  STD_LOGIC_VECTOR ((PORT_WIDTH - 1) downto 0);
           CLK_I : in  STD_LOGIC;
           SIGNAL_O : out  STD_LOGIC_VECTOR ((PORT_WIDTH - 1) downto 0));
end debouncer;

architecture Behavioral of debouncer is

constant CNTR_WIDTH : integer := natural(ceil(LOG2(real(DEBNC_CLOCKS))));
constant CNTR_MAX : std_logic_vector((CNTR_WIDTH - 1) downto 0) := std_logic_vector(to_unsigned((DEBNC_CLOCKS - 1), CNTR_WIDTH));
type VECTOR_ARRAY_TYPE is array (integer range <>) of std_logic_vector((CNTR_WIDTH - 1) downto 0);

signal sig_cntrs_ary : VECTOR_ARRAY_TYPE (0 to (PORT_WIDTH - 1)) := (others=>(others=>'0'));

signal sig_out_reg : std_logic_vector((PORT_WIDTH - 1) downto 0) := (others => '0');

begin

debounce_process : process (CLK_I)
begin
   if (rising_edge(CLK_I)) then
   for index in 0 to (PORT_WIDTH - 1) loop
      if (sig_cntrs_ary(index) = CNTR_MAX) then
         sig_out_reg(index) <= not(sig_out_reg(index));
      end if;
   end loop;
   end if;
end process;

counter_process : process (CLK_I)
begin
	if (rising_edge(CLK_I)) then
	for index in 0 to (PORT_WIDTH - 1) loop
	
		if ((sig_out_reg(index) = '1') xor (SIGNAL_I(index) = '1')) then
			if (sig_cntrs_ary(index) = CNTR_MAX) then
				sig_cntrs_ary(index) <= (others => '0');
			else
				sig_cntrs_ary(index) <= sig_cntrs_ary(index) + 1;
			end if;
		else
			sig_cntrs_ary(index) <= (others => '0');
		end if;
		
	end loop;
	end if;
end process;

SIGNAL_O <= sig_out_reg;

end Behavioral;

