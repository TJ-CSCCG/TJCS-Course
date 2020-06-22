----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Sam Bobrowicz, Albert Fazakas
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    17:20:44 03/19/2013 
-- Design Name: 
-- Module Name:    AccelDisplay - Behavioral 
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
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use ieee.math_real.all;



-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity AccelDisplay is
    Generic 
           (
             X_XY_WIDTH   : natural := 511; -- Width of the Accelerometer frame X-Y region
             X_MAG_WIDTH  : natural := 50;  -- Width of the Accelerometer frame Magnitude region
             Y_HEIGHT     : natural := 511; -- Height of the Accelerometer frame
             X_START      : natural := 385; -- Accelerometer frame X-Y region starting horizontal location
             Y_START      : natural := 80; -- Accelerometer frame starting vertical location
             BG_COLOR : STD_LOGIC_VECTOR (11 downto 0) := x"FFF"; -- Background color - white
             ACTIVE_COLOR : STD_LOGIC_VECTOR (11 downto 0) := x"0F0"; -- Green when inside the threshold box
             WARNING_COLOR : STD_LOGIC_VECTOR (11 downto 0) := x"F00" -- Red when outside the threshold box
           );
    Port
          ( 
            CLK_I : IN std_logic;
            H_COUNT_I : IN std_logic_vector(11 downto 0);
            V_COUNT_I : IN std_logic_vector(11 downto 0);
            ACCEL_X_I : IN std_logic_vector(8 downto 0); -- X acceleration input data
            ACCEL_Y_I : IN std_logic_vector(8 downto 0); -- Y acceleration input data
            ACCEL_MAG_I : IN std_logic_vector(8 downto 0); -- Acceleration magnitude input data
            ACCEL_RADIUS : IN  STD_LOGIC_VECTOR (11 downto 0); -- Size of the box moving according to acceleration data
            LEVEL_THRESH : IN  STD_LOGIC_VECTOR (11 downto 0); -- Size of the threshold box
            -- Accelerometer Red, Green and Blue signals
            RED_O    : OUT std_logic_vector(3 downto 0);
            BLUE_O   : OUT std_logic_vector(3 downto 0);
            GREEN_O  : OUT std_logic_vector(3 downto 0)
         );
end AccelDisplay;

architecture Behavioral of AccelDisplay is

--dependent constants
constant X_WIDTH: natural := X_XY_WIDTH + X_MAG_WIDTH; -- width of the entire Accelerometer frame

-- Horizontal midpoint location of the Accelerometer frame X-Y region
constant FR_XY_H_MID : natural := natural(round(real(X_XY_WIDTH/2)));
-- Vertical midpoint location of the Accelerometer X-Y region
constant FR_XY_V_MID : natural := natural(round(real(Y_HEIGHT/2)));

 -- Starting horizontal position of the Magnitude column
constant X_MAG_START  : natural := X_START + X_XY_WIDTH;
-- End horizontal position of the Magnitude column
constant X_MAG_END : integer := (X_START + X_WIDTH) - 1;

-- End vertical position of the Accelerometer frame
constant Y_END : integer := (Y_HEIGHT + Y_START) - 1;

-- Moving box limits
signal MOVING_BOX_LEFT	: natural;
signal MOVING_BOX_RIGHT	: natural;
signal MOVING_BOX_TOP   : natural;
signal MOVING_BOX_BOTTOM : natural;

--Threshold box limits
signal THRESHOLD_BOX_LEFT	 : natural;
signal THRESHOLD_BOX_RIGHT	 : natural;
signal THRESHOLD_BOX_TOP	 : natural;
signal THRESHOLD_BOX_BOTTOM : natural;

-- Signals showing when to send moving box or magnitude level 
-- or threshold box data
signal draw_moving_box      : std_logic;
signal draw_magnitude_level : std_logic;
signal draw_threshold_box   : std_logic;

-- Moving box color value
signal level_color : std_logic_vector(11 downto 0);
-- Magnitude column color value
signal magnitude_color : std_logic_vector(11 downto 0);

-- Output RGB signal, 4 bits Red, 4 bits Green and 4 bits Blue
signal color_out_reg : std_logic_vector(11 downto 0) := (others=>'0');

begin

-- Set the limit signals
-- Moving box limits
MOVING_BOX_LEFT   <= X_START - conv_integer(ACCEL_RADIUS) - 1;
MOVING_BOX_RIGHT  <= X_START + conv_integer(ACCEL_RADIUS) + 1;
MOVING_BOX_TOP	   <= Y_START - conv_integer(ACCEL_RADIUS) - 1;
MOVING_BOX_BOTTOM <= Y_START + conv_integer(ACCEL_RADIUS) + 1;

--Threshold box limits
THRESHOLD_BOX_LEFT	<= FR_XY_H_MID + X_START - conv_integer(LEVEL_THRESH) - 1;
THRESHOLD_BOX_RIGHT	<= FR_XY_H_MID + X_START + conv_integer(LEVEL_THRESH) + 1;
THRESHOLD_BOX_TOP		<= FR_XY_V_MID + Y_START - conv_integer(LEVEL_THRESH) - 1;
THRESHOLD_BOX_BOTTOM <= FR_XY_V_MID + Y_START + conv_integer(LEVEL_THRESH) + 1;

-- Create the moving box signal
-- Note that the accelerometer on the Nexys4 board is turned 90 degrees, 
-- therefore such as X axis is perpendicular to the board text, 
-- so the X axis data will be displayed on the vertical axis
--                                         Using comparison on 12 bits
draw_moving_box <= '1' when H_COUNT_I > (("000" & ACCEL_Y_I) + MOVING_BOX_LEFT)
                        and H_COUNT_I < (("000" & ACCEL_Y_I) + MOVING_BOX_RIGHT)
                        and V_COUNT_I > (("000" & ACCEL_X_I) + MOVING_BOX_TOP)
                        and V_COUNT_I < (("000" & ACCEL_X_I) + MOVING_BOX_BOTTOM)
                    else '0';

--Create the magnitude level signal
draw_magnitude_level <= '1' when   H_COUNT_I >= (X_MAG_START - 1)
                              and  H_COUNT_I <= X_MAG_END
                              and (V_COUNT_I + ("000" & ACCEL_MAG_I)) >= Y_END 
                              and  V_COUNT_I <= Y_END    
                        else '0';
 
--Create the threshold box signal 
draw_threshold_box <= '1' when ((H_COUNT_I = THRESHOLD_BOX_LEFT or H_COUNT_I = THRESHOLD_BOX_RIGHT)  -- Left and Right vertical lines
                            and  V_COUNT_I >= THRESHOLD_BOX_TOP 
                            and  V_COUNT_I <= THRESHOLD_BOX_BOTTOM )
                             or 
                               ((V_COUNT_I = THRESHOLD_BOX_TOP or  V_COUNT_I = THRESHOLD_BOX_BOTTOM) -- Top and Bottom Horizontal Lines
                            and  H_COUNT_I >= THRESHOLD_BOX_LEFT 
                            and  H_COUNT_I <= THRESHOLD_BOX_RIGHT) 
                          
                          else '0';
               
-- The moving box is green when inside the threshold box and red when outside                          
level_color <= ACTIVE_COLOR when  ACCEL_Y_I >= (FR_XY_V_MID - LEVEL_THRESH) -- Upper boundary
                              and ACCEL_Y_I <= (FR_XY_V_MID + LEVEL_THRESH) -- Lower boundary
                              and ACCEL_X_I >= (FR_XY_H_MID - LEVEL_THRESH) -- Left boundary
                              and ACCEL_X_I <= (FR_XY_H_MID + LEVEL_THRESH) -- Right boundary
                            
                            else WARNING_COLOR;

magnitude_color <= ACTIVE_COLOR;              
                     
process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    if (draw_threshold_box = '1') then -- Threshold box is black
      color_out_reg <= x"000";
    elsif (draw_moving_box = '1') then -- Moving box is either green or red
      color_out_reg <= level_color;
    elsif (draw_magnitude_level = '1') then -- Magnitude color will be the same as the active color
      color_out_reg <= magnitude_color;
    else
      color_out_reg <= BG_COLOR;
    end if;
  end if;
end process;

-- Assign Outputs
RED_O   <= color_out_reg(11 downto 8);
GREEN_O <= color_out_reg(7 downto 4);
BLUE_O  <= color_out_reg(3 downto 0);

end Behavioral;

