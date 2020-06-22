----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Sam Bobrowicz
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    20:15:58 03/20/2013 
-- Design Name: 
-- Module Name:    OverlayCtl - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_unsigned.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity OverlayCtl is
    Port ( CLK_I : in  STD_LOGIC;
           VSYNC_I : in  STD_LOGIC;
           ACTIVE_I : in  STD_LOGIC;
           OVERLAY_O : out  STD_LOGIC
           );
end OverlayCtl;

architecture Behavioral of OverlayCtl is

COMPONENT overlay_bram
  PORT (
    clka : IN STD_LOGIC;
    addra : IN STD_LOGIC_VECTOR(20 DOWNTO 0);
    douta : OUT STD_LOGIC_VECTOR(0 DOWNTO 0)
  );
END COMPONENT;

constant RAM_DEPTH : integer := 1310720; -- 1280x1024

-- Address counter, 21 bits to cover the entire screen
signal addr_cntr_reg : STD_LOGIC_VECTOR(20 DOWNTO 0) := (others=>'0');
signal data_dummy : std_logic_vector(0 downto 0);

begin

process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    if (VSYNC_I = '1') then -- Restart Address Counter at the beginning of the screen
      addr_cntr_reg <= (others=>'0');
    elsif (ACTIVE_I = '1') then -- Increment the address counter when in the active screen region
      if (addr_cntr_reg = (RAM_DEPTH - 1)) then
        addr_cntr_reg <= (others=>'0');
      else
        addr_cntr_reg <= addr_cntr_reg + 1;
      end if;
    end if;
  end if;
end process;

-- BRAM containing the overlay data, 
-- content in the OverlayBram.ngc file
Overlay_Memory : overlay_bram
  PORT MAP (
    clka => CLK_I,
    addra => addr_cntr_reg,
    douta => data_dummy
  );

-- Assign output
OVERLAY_O <= data_dummy(0);

end Behavioral;



