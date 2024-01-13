----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Mihaita Nagy, Sam Bobrowicz
--          Copyright 2014 Digilent, Inc.
---------------------------------------------------------------------------- 
-- 
-- Create Date:    11:26:53 03/13/2014 
-- Design Name: 
-- Module Name:    RgbLedDisplay - Behavioral 
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
use IEEE.math_real.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity RgbLedDisplay is
generic(
           X_RGB_COL_WIDTH    : natural := 50;   -- = SZ_RGB_WIDTH - width of one RGB column
           Y_RGB_COL_HEIGHT   : natural := 150;  -- = SZ_RGB_HEIGHT - height of one RGB column
           X_RGB_R_LOC        : natural := 1050; -- = FRM_RGB_R_H_LOC - X Location of the RGB LED RED Column
           X_RGB_G_LOC        : natural := 1125; -- = FRM_RGB_G_H_LOC - X Location of the RGB LED GREEN Column
           X_RGB_B_LOC        : natural := 1200; -- = FRM_RGB_B_H_LOC - X Location of the RGB LED BLUE Column
           Y_RGB_1_LOC        : natural := 675;  -- = FRM_RGB_1_V_LOC - Y Location of the RGB LED LD16 Column
           Y_RGB_2_LOC        : natural := 840   -- = FRM_RGB_1_V_LOC + SZ_RGB_HEIGHT + 15 - Y Location of the RGB LED LD17 Column
           );
    Port (
           pxl_clk        : in STD_LOGIC;
           RGB_LED_RED    : in STD_LOGIC_VECTOR (4 downto 0);
           RGB_LED_GREEN  : in STD_LOGIC_VECTOR (4 downto 0);
           RGB_LED_BLUE   : in STD_LOGIC_VECTOR (4 downto 0);
           H_COUNT_I      : in  STD_LOGIC_VECTOR (11 downto 0);
           V_COUNT_I      : in  STD_LOGIC_VECTOR (11 downto 0);
           -- RGB LED RED signal Data for the three columns
           RGB_LED_R_RED_COL   : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_R_GREEN_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_R_BLUE_COL  : out STD_LOGIC_VECTOR (3 downto 0);
           -- RGB LED GREEN signal Data for the three columns
           RGB_LED_G_RED_COL   : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_G_GREEN_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_G_BLUE_COL  : out STD_LOGIC_VECTOR (3 downto 0);
           -- RGB LED BLUE signal Data for the three columns
           RGB_LED_B_RED_COL   : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_B_GREEN_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_B_BLUE_COL  : out STD_LOGIC_VECTOR (3 downto 0)
          );
end RgbLedDisplay;

architecture Behavioral of RgbLedDisplay is

-- RGB columns starting pixels
constant FRM_RGB_1_V_LOC 	: natural := Y_RGB_1_LOC;
constant FRM_RGB_2_V_LOC 	: natural := Y_RGB_2_LOC;
constant FRM_RGB_R_H_LOC 	: natural := X_RGB_R_LOC;
constant FRM_RGB_G_H_LOC 	: natural := X_RGB_G_LOC;
constant FRM_RGB_B_H_LOC 	: natural := X_RGB_B_LOC;

-- LD16 R, G, B Columns Top and Bottom locations
constant RGB1_COL_TOP	 		: natural := FRM_RGB_1_V_LOC - 1;
constant RGB1_COL_BOTTOM		: natural := FRM_RGB_1_V_LOC + Y_RGB_COL_HEIGHT + 1; 
-- LD17 R, G, B Columns Top and Bottom locations
constant RGB2_COL_TOP	 		: natural := FRM_RGB_2_V_LOC - 1;
constant RGB2_COL_BOTTOM		: natural := FRM_RGB_2_V_LOC + Y_RGB_COL_HEIGHT + 1; 

-- Scale factor for the height of the columns
constant SCALE_FACTOR      : natural := natural (round(real(Y_RGB_COL_HEIGHT/30)));

-- Each column will have one moving color: the red column will be either red or white,
-- the green column either green or white, the blue column either blue or white

signal rgb_r_red_col			: std_logic_vector(3 downto 0); -- Red signal for the Red column
signal rgb_g_red_col 		: std_logic_vector(3 downto 0); -- Green signal for the Red column
signal rgb_b_red_col 		: std_logic_vector(3 downto 0); -- Blue signal for the Red column
-- G and B color signals are the same for the red column
signal rgb_gb_red_col 		: std_logic_vector(3 downto 0);

signal rgb_r_green_col		: std_logic_vector(3 downto 0); -- Red signal for the Green column
signal rgb_g_green_col 		: std_logic_vector(3 downto 0); -- Green signal for the Green column
signal rgb_b_green_col 		: std_logic_vector(3 downto 0); -- Blue signal for the Green column
-- R and B color signals are the same for the green column
signal rgb_rb_green_col 	: std_logic_vector(3 downto 0);

signal rgb_r_blue_col		: std_logic_vector(3 downto 0); -- Red signal for the Blue column
signal rgb_g_blue_col 		: std_logic_vector(3 downto 0); -- Green signal for the Blue column
signal rgb_b_blue_col 		: std_logic_vector(3 downto 0); -- Blue signal for the Blue column
-- R and G color signals are the same for the blue column
signal rgb_rg_blue_col 		: std_logic_vector(3 downto 0);

-- Size of the columns according to the incoming RGB LED data
signal rgb_r_col_size 		: natural range 0 to Y_RGB_COL_HEIGHT;
signal rgb_g_col_size 		: natural range 0 to Y_RGB_COL_HEIGHT;
signal rgb_b_col_size 		: natural range 0 to Y_RGB_COL_HEIGHT;


begin

-- Set the sizes of the columns
-- Red column
process (RGB_LED_RED)
begin
   if (conv_integer(unsigned((RGB_LED_RED))) = 31) then
      rgb_r_col_size <= Y_RGB_COL_HEIGHT;
   else 
      rgb_r_col_size <= SCALE_FACTOR * conv_integer(unsigned(RGB_LED_RED));
   end if;
end process;

-- Green column
process (RGB_LED_GREEN)
begin
   if (conv_integer(unsigned((RGB_LED_GREEN))) = 31) then
      rgb_g_col_size <= Y_RGB_COL_HEIGHT;
   else 
      rgb_g_col_size <= SCALE_FACTOR * conv_integer(unsigned(RGB_LED_GREEN));
   end if;
end process;

-- Blue column
process (RGB_LED_BLUE)
begin
   if (conv_integer(unsigned((RGB_LED_BLUE))) = 31) then
      rgb_b_col_size <= Y_RGB_COL_HEIGHT;
   else 
      rgb_b_col_size <= SCALE_FACTOR * conv_integer(unsigned(RGB_LED_BLUE));
   end if;
end process;


-- RGB LED RED COLUMN
rgb_r_red_col <=  x"F"; -- The column color will be either red or white
					
rgb_gb_red_col <= x"0" when (H_COUNT_I > FRM_RGB_R_H_LOC and H_COUNT_I < FRM_RGB_R_H_LOC + X_RGB_COL_WIDTH)
                     and -- Set for both RGB LEDs 
                          (  -- LD16 columns
                           (V_COUNT_I > RGB1_COL_BOTTOM - rgb_r_col_size - 1 and V_COUNT_I < RGB1_COL_BOTTOM)
                          or -- LD17 columns
                           (V_COUNT_I > RGB2_COL_BOTTOM - rgb_r_col_size - 1 and V_COUNT_I < RGB2_COL_BOTTOM)
                           )
                  else x"F";

rgb_g_red_col <= rgb_gb_red_col;
rgb_b_red_col <= rgb_gb_red_col;
					

-- RGB LED GREEN COLUMN				
rgb_rb_green_col <= x"0" when (H_COUNT_I > FRM_RGB_G_H_LOC and H_COUNT_I < FRM_RGB_G_H_LOC + X_RGB_COL_WIDTH)
                     and -- Set for both RGB LEDs 
                          (  -- LD16 columns
                           (V_COUNT_I > RGB1_COL_BOTTOM - rgb_g_col_size - 1 and V_COUNT_I < RGB1_COL_BOTTOM)
                        or   -- LD17 columns
                           (V_COUNT_I > RGB2_COL_BOTTOM - rgb_g_col_size - 1 and V_COUNT_I < RGB2_COL_BOTTOM)
                           )
                  else x"F";

rgb_g_green_col <=  x"F"; -- The column color will be either green or white

rgb_r_green_col <= rgb_rb_green_col;
rgb_b_green_col <= rgb_rb_green_col;


-- RGB LED BLUE COLUMN				
rgb_rg_blue_col <= x"0" when (H_COUNT_I > FRM_RGB_B_H_LOC and H_COUNT_I < FRM_RGB_B_H_LOC + X_RGB_COL_WIDTH)
                     and -- Set for both RGB LEDs 
                          (  -- LD16 columns
                           (V_COUNT_I > RGB1_COL_BOTTOM - rgb_b_col_size - 1 and V_COUNT_I < RGB1_COL_BOTTOM)
                        or   -- LD17 columns
                           (V_COUNT_I > RGB2_COL_BOTTOM - rgb_b_col_size - 1 and V_COUNT_I < RGB2_COL_BOTTOM)
                           )
                   else x"F";

rgb_b_blue_col <=  x"F"; -- The column color will be either blue or white

rgb_r_blue_col <= rgb_rg_blue_col;
rgb_g_blue_col <= rgb_rg_blue_col;


-- Assign outputs
process (pxl_clk)
begin
   if pxl_clk'EVENT and pxl_clk = '1' then
      -- RGB LED RED signal data for the three columns
      RGB_LED_R_RED_COL    <= rgb_r_red_col;
      RGB_LED_R_GREEN_COL  <= rgb_r_green_col;
      RGB_LED_R_BLUE_COL   <= rgb_r_blue_col;
      -- RGB LED GREEN signal data for the three columns
      RGB_LED_G_RED_COL    <= rgb_g_red_col;
      RGB_LED_G_GREEN_COL  <= rgb_g_green_col;
      RGB_LED_G_BLUE_COL   <= rgb_g_blue_col;
      -- RGB LED BLUE signal data for the three columns
      RGB_LED_B_RED_COL    <= rgb_b_red_col;
      RGB_LED_B_GREEN_COL  <= rgb_b_green_col;
      RGB_LED_B_BLUE_COL   <= rgb_b_blue_col;
   end if;
end process;

end Behavioral;

