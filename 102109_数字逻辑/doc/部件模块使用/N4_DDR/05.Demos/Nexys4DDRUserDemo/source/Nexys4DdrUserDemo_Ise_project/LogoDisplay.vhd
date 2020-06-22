----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Sam Bobrowicz
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
--
-- Create Date:    15:40:47 03/15/2013 
-- Design Name: 
-- Module Name:    logo_display - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity LogoDisplay is
	 Generic (
			  X_START : integer range 2 to (Integer'high) := 40;
           Y_START : integer := 512
	 );
    Port ( CLK_I : in  STD_LOGIC;
           H_COUNT_I : in  STD_LOGIC_VECTOR(11 downto 0);
           V_COUNT_I : in  STD_LOGIC_VECTOR(11 downto 0);
           RED_O : out  STD_LOGIC_VECTOR(3 downto 0);
           BLUE_O : out  STD_LOGIC_VECTOR(3 downto 0);
           GREEN_O : out  STD_LOGIC_VECTOR(3 downto 0));
end LogoDisplay;

architecture Behavioral of LogoDisplay is

constant SZ_LOGO_WIDTH 	   : natural := 335; -- Width of the logo frame
constant SZ_LOGO_HEIGHT 	: natural := 280; -- Height of the logo frame

COMPONENT BRAM_1
  PORT (
    clka : IN STD_LOGIC;
    addra : IN STD_LOGIC_VECTOR(16 DOWNTO 0);
    douta : OUT STD_LOGIC_VECTOR(11 DOWNTO 0)
  );
END COMPONENT;

signal addr_reg : std_logic_vector(16 downto 0) := (others => '0');
signal douta	: std_logic_vector(11 downto 0);
signal rst		: std_logic;
signal en		: std_logic;

begin

-- BRAM containing the logo data, 
-- content in the BRAM_1.ngc file
 Inst_BRAM_1 : BRAM_1
 PORT MAP (
	clka => CLK_I,
	addra => addr_reg,
	douta => douta
 );

-- Restart Address Counter when Vcount arrives to the beginning of the Logo frame
rst <= '1' when (H_COUNT_I = 0 and V_COUNT_I = Y_START-1) else '0';

-- Increment Address counter only inside the frame
en <= '1' when (H_COUNT_I > X_START-2 and H_COUNT_I < X_START + SZ_LOGO_WIDTH - 1 
            and V_COUNT_I > Y_START and V_COUNT_I < Y_START + SZ_LOGO_HEIGHT -1 ) 
          else '0';

-- Address counter
process (CLK_I, rst, en)
begin
	if(rising_edge(CLK_I))then 
		if(rst = '1') then
			addr_reg <= (others => '0');
		elsif(en = '1') then
			addr_reg <= addr_reg + 1;
		end if;
	end if;	

end process;

-- Assign Outputs
RED_O <= douta(11 downto 8);
BLUE_O <= douta(3 downto 0);
GREEN_O <= douta(7 downto 4);


end Behavioral;

