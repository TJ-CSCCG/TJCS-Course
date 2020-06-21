----------------------------------------------------------------------------
--	RGB_controller.vhd -- Nexys4 RGB LED controller
----------------------------------------------------------------------------
-- Author:  Marshall Wingerson 
--          Copyright 2013 Digilent, Inc.
----------------------------------------------------------------------------
--
----------------------------------------------------------------------------
-- Revision History:
--  08/08/2013(MarshallW): Created 
--  08/30/2013(SamB): Modified RGB pattern
--                    Added comments
----------------------------------------------------------------------------

LIBRARY IEEE;
USE IEEE.STD_LOGIC_1164.ALL;
USE IEEE.STD_LOGIC_UNSIGNED.ALL;
USE IEEE.NUMERIC_STD.ALL;

entity RGB_controller is
	port(
	GCLK 			: in std_logic;
	RGB_LED_1_O	: out std_logic_vector(2 downto 0);
	RGB_LED_2_O	: out std_logic_vector(2 downto 0)
	);
end RGB_controller;

architecture Behavioral of RGB_controller is
--counter signals
	constant window: std_logic_vector(7 downto 0) := "11111111";
	signal windowcount: std_logic_vector(7 downto 0) := (others => '0');
	
    constant deltacountMax: std_logic_vector(19 downto 0) := std_logic_vector(to_unsigned(1000000, 20));
	signal deltacount: std_logic_vector(19 downto 0) := (others => '0');
		
    constant valcountMax: std_logic_vector(8 downto 0) := "101111111";
    signal valcount: std_logic_vector(8 downto 0) := (others => '0');

--color intensity signals
    signal incVal: std_logic_vector(7 downto 0);
    signal decVal: std_logic_vector(7 downto 0);

	signal redVal: std_logic_vector(7 downto 0);
	signal greenVal: std_logic_vector(7 downto 0);
	signal blueVal: std_logic_vector(7 downto 0);
	
	signal redVal2: std_logic_vector(7 downto 0);
	signal greenVal2: std_logic_vector(7 downto 0);
	signal blueVal2: std_logic_vector(7 downto 0);
	
--PWM registers
    signal rgbLedReg1: std_logic_vector(2 downto 0);
    signal rgbLedReg2: std_logic_vector(2 downto 0);
	
begin

window_counter:process(GCLK)
begin
	if(rising_edge(GCLK)) then
		if windowcount < (window) then
			windowcount <= windowcount + 1;
		else
			windowcount <= (others => '0');
		end if;
	end if;
end process;

color_change_counter:process(GCLK)
begin
    if(rising_edge(GCLK)) then
		if(deltacount < deltacountMax) then
			deltacount <= deltacount + 1;
		else
			deltacount <= (others => '0');
		end if;
	end if;
end process;

color_intensity_counter:process(GCLK)
begin
	if(rising_edge(GCLK)) then
	if(deltacount = 0) then
		if(valcount < valcountMax) then
			valcount <= valcount + 1;
		else
			valcount <= (others => '0');
		end if;
    end if;
	end if;
end process;

incVal <= "0" & valcount(6 downto 0);

--The folowing code sets decVal to (128 - incVal)
decVal(7) <= '0';
decVal(6) <= not(valcount(6));
decVal(5) <= not(valcount(5));
decVal(4) <= not(valcount(4));
decVal(3) <= not(valcount(3));
decVal(2) <= not(valcount(2));
decVal(1) <= not(valcount(1));
decVal(0) <= not(valcount(0)); 

redVal <= incVal when (valcount(8 downto 7) = "00") else
          decVal when (valcount(8 downto 7) = "01") else
          (others => '0');       
greenVal <= decVal when (valcount(8 downto 7) = "00") else
          (others => '0') when (valcount(8 downto 7) = "01") else
          incVal;
blueVal <= (others => '0') when (valcount(8 downto 7) = "00") else
           incVal when (valcount(8 downto 7) = "01") else
           decVal;
           
redVal2 <= incVal when (valcount(8 downto 7) = "00") else
         decVal when (valcount(8 downto 7) = "01") else
         (others => '0');       
greenVal2 <= decVal when (valcount(8 downto 7) = "00") else
         (others => '0') when (valcount(8 downto 7) = "01") else
         incVal;
blueVal2 <= (others => '0') when (valcount(8 downto 7) = "00") else
          incVal when (valcount(8 downto 7) = "01") else
          decVal;


--red processes
red_comp:process(GCLK)
begin
	if(rising_edge(GCLK)) then
		if((redVal) > windowcount) then
			rgbLedReg1(2) <= '1';
		else
			rgbLedReg1(2) <= '0';
		end if;
	end if;
end process;


--green processes
green_comp:process(GCLK)
begin
	if(rising_edge(GCLK)) then
		if((greenVal) > windowcount) then
			rgbLedReg1(1) <= '1';
		else
			rgbLedReg1(1) <= '0';
		end if;
	end if;
end process;

			
--blue processes
blue_comp:process(GCLK)
begin
	if(rising_edge(GCLK)) then
		if((blueVal) > windowcount) then
			rgbLedReg1(0) <= '1';
		else
			rgbLedReg1(0) <= '0';
		end if;
	end if;
end process;
	
--RGB2 processes---	
--red2 processes
red2_comp:process(GCLK)
begin
	if(rising_edge(GCLK)) then
		if((redVal2) > windowcount) then
			rgbLedReg2(2) <= '1';
		else
			rgbLedReg2(2) <= '0';
		end if;
	end if;
end process;


--green2 processes
green2_comp:process(GCLK)
begin
	if(rising_edge(GCLK)) then
		if((greenVal2) > windowcount) then
			rgbLedReg2(1) <= '1';
		else
			rgbLedReg2(1) <= '0';
		end if;
	end if;
end process;

			
--blue2 processes
blue2_comp:process(GCLK)
begin
	if(rising_edge(GCLK)) then
		if((blueVal2) > windowcount) then
			rgbLedReg2(0) <= '1';
		else
			rgbLedReg2(0) <= '0';
		end if;
	end if;
end process;

RGB_LED_1_O <= rgbLedReg1;
RGB_LED_2_O <= rgbLedReg2;	

end Behavioral;

