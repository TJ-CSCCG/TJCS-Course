----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    09:34:59 11/29/2013 
-- Design Name: 
-- Module Name:    top - Behavioral 
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity top is
    Port ( clk : in  STD_LOGIC;
           tone : in  STD_LOGIC_VECTOR (4 downto 0);
           mode : in  STD_LOGIC_VECTOR (2 downto 0);
           pwm_out : out  STD_LOGIC;
			  pwm_sd : out STD_LOGIC);
end top;

architecture Behavioral of top is

signal audio_sample_x : STD_LOGIC_VECTOR(7 downto 0);
signal clk_50M : STD_LOGIC := '0';
signal sd: STD_LOGIC := '1';


component SimpleSynth is port
(
	 ck : in  STD_LOGIC;
    tone : in  std_logic_vector (4 downto 0);
    mode : in  STD_LOGIC_VECTOR (2 downto 0);
    audioSample : out  std_logic_vector (7 downto 0)	-- audio signal samples
);
end component ; 

component PWM is port
(
	 clk: in std_logic;							-- system clock = 50.00MHz;
    x: in std_logic_vector (7 downto 0);	-- the number to be modulated
    PWMOUT: out std_logic						-- the Pulse With Modulated signal
);
end component ;

begin

Simple : component SimpleSynth
    port map
    (
      ck => clk_50M,
		tone => tone,
      mode =>mode,
      audioSample => audio_sample_x 
    );
PWM1 : component PWM
    port map
    (
      clk => clk_50M,
      x =>audio_sample_x,
		PWMOUT=>pwm_out
    );
	 
clock_divide : process(clk)
begin
 if (clk = '1' and clk'event) then
      clk_50M <= not clk_50M;    
 end if;
 pwm_sd<='1';
 end process;
 
 
 

 
 

end Behavioral;

