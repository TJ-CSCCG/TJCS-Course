----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Albert Fazakas, Elod Gyorgy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    14:50:40 03/17/2014 
-- Design Name: 
-- Module Name:    TempDisplay - Behavioral 
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
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use ieee.math_real.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity TempDisplay is
generic(
           X_TMP_COL_WIDTH    : natural := 50;   -- = SZ_TH_WIDTH - width of a TEMP column
           Y_TMP_COL_HEIGHT   : natural := 472;  -- = SZ_TH_HEIGHT - height of a TEMP column
           X_TMP_H_LOC        : natural := 1050; -- X Location of the TEMP Column
           Y_TMP_V_LOC        : natural := 80;   -- Y Location of the TEMP Column
           INPUT_DATA_WIDTH   : natural := 12; -- Data width is 13 for the ADT7420 Temperature Sensor and 
                                               -- 12 for the XADC temperature data and the Accelerometer Temperature Sensor
           TMP_TYPE           : string := "XADC" -- Either "XADC" or "TEMP_ACC"
           );
    Port ( CLK_I     : in STD_LOGIC;
           TEMP_IN   : in STD_LOGIC_VECTOR (INPUT_DATA_WIDTH - 1 downto 0);
           H_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           V_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           -- Temperature Red, Green and Blue signals
           TEMP_R_OUT   : out STD_LOGIC_VECTOR (3 downto 0);
           TEMP_G_OUT   : out STD_LOGIC_VECTOR (3 downto 0);
           TEMP_B_OUT   : out STD_LOGIC_VECTOR (3 downto 0)
          );
end TempDisplay;

architecture Behavioral of TempDisplay is

-- Used as starting point for the Temperature level size
-- The Temperature level size is according to the value of the temperature displayed
constant TEMP_OFFSET : std_logic_vector (11 downto 0) := "001000100110"; --550

constant TEMP_BOTTOM : natural := Y_TMP_V_LOC + Y_TMP_COL_HEIGHT + 1;
-- Maximum temperature
constant TEMP_MAX	: std_logic_vector (23 downto 0) := X"000500"; -- 80C * 16
-- Convert Celsius to pixels such as 0C = 0 pixels, 80C = 480pixels
constant CELSIUS_TO_PIXELS : std_logic_vector(2 downto 0) := "110"; --6 = 480/(80-0)

-- Scale incoming XADC temperature data, according to the XADC datasheet
constant XADC_TMP_SCALE : std_logic_vector(17 downto 0) := "111110111" & "111110011"; --503.975 (18bit)
-- Convert Kelvin to Celsius
constant XADC_TMP_OFFSET : std_logic_vector(30 downto 0) := conv_std_logic_vector(integer(round(273.15)*4096.0), 31);

-- Converted and scaled temperature value
signal temp_value 		: std_logic_vector(9 downto 0);

-- Synchronize incoming temperature to the clock
signal temp_sync0, temp_sync : std_logic_vector(TEMP_IN'range);

-- signal storing the scaled XADC temperature data
signal temp_xad_scaled : std_logic_vector(temp_sync'length+XADC_TMP_SCALE'length-1 downto 0); --12bit*18bit=30bit
-- signal storing the offseted XADC temperature data
signal temp_xad_offset : std_logic_vector(XADC_TMP_OFFSET'range); --31bit
-- signal storing XADC temperature data converted to Celsius
signal temp_xad_celsius : std_logic_vector(temp_xad_offset'length-8-1 downto 0); --23bit
-- Signal storing the FPGA temperature limited to between 0C and 80C * 16
signal temp_xad_capped : std_logic_vector(temp_xad_celsius'high-1 downto 0); --no sign bit
-- The temperature scaled to pixels
signal temp_xad_px_scaled : std_logic_vector(temp_xad_capped'high+CELSIUS_TO_PIXELS'high+1 downto temp_xad_capped'low);

-- Signal storing the Temp Sensor or Accelerometer temperature limited to between 0C and 80C * 16
signal temp_capped	: std_logic_vector(temp_sync'high-1 downto temp_sync'low);
-- Signal storing the Temp Sensor or Accelerometer temperature scaled to pixels: 0 = 0C, 480 = 80C * 16
signal temp_scaled	: std_logic_vector(temp_capped'high+CELSIUS_TO_PIXELS'high+1 downto temp_capped'low);

-- Temp Column green and red color components
signal temp_color_red 	: std_logic_vector(3 downto 0);
signal temp_color_green : std_logic_vector(3 downto 0);

-- Temp Column red, green and blue signals
signal temp_red         : std_logic_vector (3 downto 0);
signal temp_green       : std_logic_vector (3 downto 0);
signal temp_blue        : std_logic_vector (3 downto 0);

begin

-- The XADC temperature data will have to scaled with 503.975, 
-- then transformed from Kelvin to Celsius
-- Then limit and scale it to pixels: 0 = 0C, 480 = 80C * 16, and multiply by 0.0625 (i.e. divide by 16)
XADC:
if TMP_TYPE = "XADC" generate
begin
process(CLK_I)
begin
	if CLK_I'EVENT and CLK_I = '1' then
			temp_sync0 <= TEMP_IN; --synchronize with pxl_clk domain
			temp_sync <= temp_sync0;
			
			--30b					12b				18b
			temp_xad_scaled <= temp_sync * XADC_TMP_SCALE; -- ADC * 503.975 (fixed-point; decimal point at 9b)
			
			temp_xad_offset <= '0' & temp_xad_scaled(29 downto 9) - XADC_TMP_OFFSET; -- ADC * 503.975 - 273.15 * 4096
			
			temp_xad_celsius <= temp_xad_offset(temp_xad_offset'high downto 8); -- (ADC * 503.975 - 273.15) / 256; 1LSB=0.625C
			
			if (temp_xad_celsius(temp_xad_celsius'high) = '1') then --if negative, cap to 0
				temp_xad_capped <= (others => '0');
			elsif (temp_xad_celsius(temp_xad_celsius'high-1 downto 0) > TEMP_MAX) then --if too big, cap to maximum scale /0.0625
				temp_xad_capped <= TEMP_MAX(temp_xad_capped'range);
			else
				temp_xad_capped <= temp_xad_celsius(temp_xad_celsius'high-1 downto 0); --get rid of the sign bit
			end if;
			
			temp_xad_px_scaled <= temp_xad_capped * CELSIUS_TO_PIXELS; --scale to pixels
	end if;
end process;

	temp_value <= temp_xad_px_scaled(13 downto 4); -- * 0.0625 (1/2^4)
end generate;   


-- The ADT7420 temperature sensor data and the ADXL362 accelerometer temperature data 
-- will have to be limited and scaled to pixels: 0 = 0C, 480 = 80C * 16, 
-- then multiply by 0.0625 (i.e. divide by 16) 

-- Note that the accelerometer temperature data in fact is 0.065C /LSB. Multiplying it by 0.0625
-- at 80C the error will be about 3..4C 
TEMP_ACC:
if TMP_TYPE = "TEMP_ACC" generate   
   process(CLK_I)
begin
	if CLK_I'EVENT and CLK_I = '1' then
			temp_sync0 <= TEMP_IN; --synchronize with pxl_clk domain
			temp_sync <= temp_sync0;
			
			if (temp_sync(temp_sync'high) = '1') then --if negative, cap to 0
				temp_capped <= (others => '0');
			elsif (temp_sync(temp_sync'high-1 downto 0) > TEMP_MAX) then -- if too big, cap to maximum scale /0.0625
				temp_capped <= TEMP_MAX(temp_capped'range);
			else
				temp_capped <= temp_sync(temp_sync'high-1 downto 0); --get rid of the sign bit
			end if;
			
			temp_scaled <= temp_capped * CELSIUS_TO_PIXELS; --scale to pixels
	end if;
end process;

	temp_value <= temp_scaled(13 downto 4); -- * 0.0625 (1/2^4)

end generate;

-- Temperature Color Decode - As temperature is higher, the color turns from green to red
temp_color_red <=		x"0" when temp_value < 16  else
							x"1" when temp_value < 32	else
							x"2" when temp_value < 48	else
							x"3" when temp_value < 64	else
							x"4" when temp_value < 80	else
							x"5" when temp_value < 96	else
							x"6" when temp_value < 112 else
							x"7" when temp_value < 128 else
							x"8" when temp_value < 144 else
							x"9" when temp_value < 160 else
							x"A" when temp_value < 176 else
							x"B" when temp_value < 192 else
							x"C" when temp_value < 208 else
							x"D" when temp_value < 224 else
							x"E" when temp_value < 240 else
							x"F";
							
temp_color_green <=	x"F" when temp_value < 256 else
							x"E" when temp_value < 272 else
							x"D" when temp_value < 288 else
							x"C" when temp_value < 304 else
							x"B" when temp_value < 320 else
							x"A" when temp_value < 336 else
							x"9" when temp_value < 352 else
							x"8" when temp_value < 368 else
							x"7" when temp_value < 384 else
							x"6" when temp_value < 400 else
							x"5" when temp_value < 416 else
							x"4" when temp_value < 432 else
							x"3" when temp_value < 448 else
							x"2" when temp_value < 464 else
							x"1" when temp_value < 480 else
							x"0";

-- Red, Green and Blue Signals for the Temperature Column
temp_red <=    temp_color_red when (H_COUNT_I > X_TMP_H_LOC and H_COUNT_I < X_TMP_H_LOC + X_TMP_COL_WIDTH)
                               and (V_COUNT_I > (TEMP_OFFSET - temp_value) and V_COUNT_I < TEMP_BOTTOM)
					else x"F";

temp_green <=  temp_color_green when (H_COUNT_I > X_TMP_H_LOC and H_COUNT_I < X_TMP_H_LOC + X_TMP_COL_WIDTH)
                                and (V_COUNT_I > (TEMP_OFFSET - temp_value) and V_COUNT_I < TEMP_BOTTOM)
					else x"F";

-- The Temperature Colum color will be either a green-red combination (from green to orange, then red), or white
temp_blue <=   x"0" when (H_COUNT_I > X_TMP_H_LOC and H_COUNT_I < X_TMP_H_LOC + X_TMP_COL_WIDTH)
                    and (V_COUNT_I > (TEMP_OFFSET - temp_value) and V_COUNT_I < TEMP_BOTTOM)
					else x"F";


-- Assign Outputs
TEMP_R_OUT <= temp_red;
TEMP_G_OUT <= temp_green;
TEMP_B_OUT <= temp_blue;

end Behavioral;

