----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Albert Fazakas adapted from Alec Wyen and Mihaita Nagy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    13:01:51 02/15/2013 
-- Design Name: 
-- Module Name:    Vga - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--       This module represents the Vga controller that creates the HSYNC and VSYNC signals
--    for the VGA screen and formats the 4-bit R, G and B signals to display various items
--    on the screen:
--       - A moving colorbar in the background
--       - A Digilent - Analog Devices logo for the Nexys4 board, the RGB data is provided 
--    by the LogoDisplay component. The logo bitmap is stored in the BRAM_1 Block RAM in .ngc format.
--       - The FPGA temperature on a 0..80C scale. Temperature data is taken from the XADC
--    component in the Artix-7 FPGA, provided by the upper level FPGAMonitor component and the RGB data is
--    provided by the Inst_XadcTempDisplay instance of the TempDisplay component.
--       - The Nexys4 Onboard ADT7420 Temperature Sensor temperature on a 0..80C scale. 
--    Temperature data is provided by the upper level TempSensorCtl component and the RGB data is
--    provided by the Inst_Adt7420TempDisplay instance of the TempDisplay component.
--       - The Nexys4 Onboard ADXL362 Accelerometer Temperature Sensor temperature on a 0..80C scale. 
--    Temperature data is provided by the upper level AccelerometerCtl component and the RGB data is
--    provided by the Inst_Adxl362TempDisplay instance of the TempDisplay component.
--       - The R, G and B data which is also sent to the Nexys4 onboard RGB Leds LD16 and LD17. The 
--    incomming RGB Led data is taken from the upper level RgbLed component and the formatted RGB data is provided
--    by the RGBLedDisplay component.
--       - The audio signal coming from the Nexys4 Onboard ADMP421 Omnidirectional Microphone. The formatted
--    RGB data is provided by the MicDisplay component.
--       - The X and Y acceleration in a form of a moving box and the acceleration magnitude determined by 
--    the SQRT (X^2 + Y^2 + Z^2) formula. The acceleration and magnitude data is provided by the upper level 
--    AccelerometerCtl component and the formatted RGB data is provided by the AccelDisplay component.
--       - The mouse cursor on the top on all of the items. The USB mouse should be connected to the Nexys4 board before 
--    the FPGA is configured. The mouse cursor data is provided by the upper level MouseCtl component and the 
--    formatted RGB data for the mouse cursor shape is provided by the MouseDisplay component.
--       - An overlay that displayed the frames and text for the displayed items described above. The overlay data is
--    stored in the overlay_bram Block RAM in the .ngc format and the data is provided by the OverlayCtl component.
--       The Vga controller holds the synchronization signal generation, the moving colorbar generation and the main
--    multiplexers for the outgoing R, G and B signals. Also the 108 MHz pixel clock (pxl_clk) generator is instantiated
--    inside the Vga controller.
--       The current resolution is 1280X1024 pixels, however, other resolutions can also be selected by 
--    commenting/uncommenting the corresponding VGA resolution constants. In the case when a different resolution
--    is selected, the pixel clock generator output frequency also has to be updated accordingly.
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

entity Vga is
    Port ( CLK_I : in  STD_LOGIC;
           -- VGA Output Signals
           VGA_HS_O : out  STD_LOGIC; -- HSYNC OUT
           VGA_VS_O : out  STD_LOGIC; -- VSYNC OUT
           VGA_RED_O    : out  STD_LOGIC_VECTOR (3 downto 0); -- Red signal going to the VGA interface
           VGA_GREEN_O  : out  STD_LOGIC_VECTOR (3 downto 0); -- Green signal going to the VGA interface
           VGA_BLUE_O   : out  STD_LOGIC_VECTOR (3 downto 0); -- Blue signal going to the VGA interface
           -- Input Signals
           -- RGB LED
           RGB_LED_RED    : in STD_LOGIC_VECTOR (7 downto 0);
           RGB_LED_GREEN  : in STD_LOGIC_VECTOR (7 downto 0);
           RGB_LED_BLUE   : in STD_LOGIC_VECTOR (7 downto 0);
           -- Accelerometer
           ACCEL_RADIUS : in  STD_LOGIC_VECTOR (11 downto 0); -- Size of the box moving when the board is tilted
           LEVEL_THRESH : in  STD_LOGIC_VECTOR (11 downto 0); -- Size of the internal box in which the moving box is green
           ACL_X_IN       : in  STD_LOGIC_VECTOR (8 downto 0); -- X Acceleration Data
           ACL_Y_IN       : in  STD_LOGIC_VECTOR (8 downto 0); -- Y Acceleration Data
           ACL_MAG_IN     : in  STD_LOGIC_VECTOR (11 downto 0); -- Acceleration Magnitude
           -- Microphone
           MIC_M_DATA_I : IN STD_LOGIC; -- Input microphone data
           MIC_M_CLK_RISING  : IN STD_LOGIC; -- Active when the data from the microphone is read
           -- Mouse signals
           MOUSE_X_POS :  in std_logic_vector (11 downto 0); -- X position from the mouse
           MOUSE_Y_POS :  in std_logic_vector (11 downto 0); -- Y position from the mouse
           -- Temperature data signals
           XADC_TEMP_VALUE_I     : in std_logic_vector (11 downto 0); -- FPGA Temperature data from the XADC
           ADT7420_TEMP_VALUE_I  : in std_logic_vector (12 downto 0); -- Temperature data from the Onboard Temperature Sensor
           ADXL362_TEMP_VALUE_I  : in std_logic_vector (11 downto 0)  -- Temperature Data from the Accelerometer
           );
end Vga;

architecture Behavioral of Vga is

-------------------------------------------------------------------------

-- Component Declarations

-------------------------------------------------------------------------


   -- To generate the 108 MHz Pixel Clock
   -- needed for a resolution of 1280*1024 pixels
   COMPONENT PxlClkGen
   PORT
    (-- Clock in ports
     CLK_IN1           : in std_logic;
     -- Clock out ports
     CLK_OUT1          : out std_logic;
     -- Status and control signals
     LOCKED            : out std_logic
    );
   END COMPONENT;

   -- Display the Digilent Nexys 4 and Analog Devices Logo
	COMPONENT LogoDisplay
	GENERIC(
      X_START : integer range 2 to (Integer'high) := 40; -- Logo Starting Horizontal Location
      Y_START : integer := 512 -- Logo Starting Vertical Location
	);
	PORT(
		CLK_I : IN std_logic;
		H_COUNT_I : IN std_logic_vector(11 downto 0);
		V_COUNT_I : IN std_logic_vector(11 downto 0);
      -- Logo Red, Green and Blue signals
		RED_O : OUT std_logic_vector(3 downto 0);
		BLUE_O : OUT std_logic_vector(3 downto 0);
		GREEN_O : OUT std_logic_vector(3 downto 0)
		);
	END COMPONENT;
   
   -- Display the overlay
   COMPONENT OverlayCtl
	PORT(
		CLK_I : IN std_logic;
		VSYNC_I : IN std_logic;
		ACTIVE_I : IN std_logic;          
		OVERLAY_O : OUT std_logic
		);
	END COMPONENT;
   
   -- Display the LD16 and LD17 RGB LED data
   COMPONENT RgbLedDisplay
   GENERIC(
           X_RGB_COL_WIDTH    : natural := 50; -- SZ_RGB_WIDTH - width of one RGB column
           Y_RGB_COL_HEIGHT   : natural := 150; -- SZ_RGB_HEIGHT - height of one RGB column
           X_RGB_R_LOC        : natural := 1050; -- FRM_RGB_R_H_LOC
           X_RGB_G_LOC        : natural := 1125; -- FRM_RGB_G_H_LOC
           X_RGB_B_LOC        : natural := 1200; -- FRM_RGB_B_H_LOC
           Y_RGB_1_LOC        : natural := 675; -- FRM_RGB_1_V_LOC
           Y_RGB_2_LOC        : natural := 840  -- FRM_RGB_1_V_LOC + SZ_RGB_HEIGHT + 15
           );
    PORT ( 
           pxl_clk        : in std_logic;
           RGB_LED_RED    : in STD_LOGIC_VECTOR (4 downto 0);
           RGB_LED_GREEN  : in STD_LOGIC_VECTOR (4 downto 0);
           RGB_LED_BLUE   : in STD_LOGIC_VECTOR (4 downto 0);
           H_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           V_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           -- Red Columns RGB LED Data
           RGB_LED_R_RED_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_R_GREEN_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_R_BLUE_COL : out STD_LOGIC_VECTOR (3 downto 0);
           -- Green Columns RGB LED Data
           RGB_LED_G_RED_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_G_GREEN_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_G_BLUE_COL : out STD_LOGIC_VECTOR (3 downto 0);
           -- Blue Columns RGB LED Data
           RGB_LED_B_RED_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_B_GREEN_COL : out STD_LOGIC_VECTOR (3 downto 0);
           RGB_LED_B_BLUE_COL : out STD_LOGIC_VECTOR (3 downto 0)
          );
   END COMPONENT;

   -- Display the FPGA, Temp Sensor and Accelerometer Temperature
   COMPONENT TempDisplay
   GENERIC(
           X_TMP_COL_WIDTH    : natural := 50;   -- = SZ_TH_WIDTH - width of a TMP column
           Y_TMP_COL_HEIGHT   : natural := 472;  -- = SZ_TH_HEIGHT - height of a TMP column
           X_TMP_H_LOC        : natural := 1050; -- X Location of the TMP Column
           Y_TMP_V_LOC        : natural := 80;   -- Y Location of the TMP Column
           INPUT_DATA_WIDTH   : natural := 13; -- Data width is 13 for XADC and 12 for Temperature Sensor and 
                                               -- Accelerometer Temperature Sensor
           TMP_TYPE           : string := "XADC"
           );
    PORT ( CLK_I     : in STD_LOGIC;
           TEMP_IN   : in STD_LOGIC_VECTOR (INPUT_DATA_WIDTH - 1 downto 0); -- Input Temperature Data
           H_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           V_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           -- Temperature Red, Green and Blue signals
           TEMP_R_OUT   : out STD_LOGIC_VECTOR (3 downto 0);
           TEMP_G_OUT   : out STD_LOGIC_VECTOR (3 downto 0);
           TEMP_B_OUT   : out STD_LOGIC_VECTOR (3 downto 0)
          );
   END COMPONENT;

   -- Display signal from the Onboard Microphone
	COMPONENT MicDisplay
	GENERIC(
		X_WIDTH : integer := 1000;
      Y_HEIGHT : integer := 400;
      X_START : integer range 2 to (Integer'high) := 40;
      Y_START : integer := 512;
      PXLCLK_FREQ_HZ : integer := 108000000;
      H_MAX : integer := 1688;
      SAMPLE_RATE_DIV : integer := 4096;
      BG_COLOR : STD_LOGIC_VECTOR (11 downto 0) := x"FFF";
      ACTIVE_COLOR : STD_LOGIC_VECTOR (11 downto 0) := x"008" -- Light blue
	);
	PORT(
		CLK_I : IN std_logic;
      SYSCLK : IN std_logic;
		H_COUNT_I : IN std_logic_vector(11 downto 0);
		V_COUNT_I : IN std_logic_vector(11 downto 0);          
		MIC_M_DATA_I : IN std_logic;
      MIC_M_CLK_RISING  : IN STD_LOGIC;
      RED_O : out  STD_LOGIC_VECTOR (3 downto 0);
      GREEN_O : out  STD_LOGIC_VECTOR (3 downto 0);
      BLUE_O : out  STD_LOGIC_VECTOR (3 downto 0)
      );
	END COMPONENT;

   -- Display the moving box and acceleration magnitude according to accelerometer data
	COMPONENT AccelDisplay
  	GENERIC
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
	PORT
   (
		CLK_I : IN std_logic;
		H_COUNT_I : IN std_logic_vector(11 downto 0);
		V_COUNT_I : IN std_logic_vector(11 downto 0);
		ACCEL_X_I : IN std_logic_vector(8 downto 0); -- X acceleration input data
		ACCEL_Y_I : IN std_logic_vector(8 downto 0); -- Y acceleration input data
		ACCEL_MAG_I : IN std_logic_vector(8 downto 0); -- Acceleration magnitude input data
      ACCEL_RADIUS : IN  STD_LOGIC_VECTOR (11 downto 0); -- Size of the box moving according to acceleration data
      LEVEL_THRESH : IN  STD_LOGIC_VECTOR (11 downto 0); -- Size of the threshold box
      -- Acceleerometer Red, Green and Blue signals
		RED_O    : OUT std_logic_vector(3 downto 0);
		BLUE_O   : OUT std_logic_vector(3 downto 0);
		GREEN_O  : OUT std_logic_vector(3 downto 0)
	);
	END COMPONENT;
   
   -- Display the Mouse cursor
   COMPONENT MouseDisplay
   PORT (
      pixel_clk: in std_logic;
      xpos     : in std_logic_vector(11 downto 0); -- Mouse cursor X position
      ypos     : in std_logic_vector(11 downto 0); -- Mouse cursor Y position 

      hcount   : in std_logic_vector(11 downto 0);
      vcount   : in std_logic_vector(11 downto 0);
      --blank    : in std_logic; -- blank the screen in overlay mode, here is not used
      
      enable_mouse_display_out : out std_logic; -- When active, the mouse cursor signal is sent to the VGA display
      
      --red_in   : in std_logic_vector(3 downto 0); -- Red, Green and Blue input signal in overlay mode, here are not used
      --green_in : in std_logic_vector(3 downto 0);
      --blue_in  : in std_logic_vector(3 downto 0);
      -- Output Red, blue and Green Signals
      red_out  : out std_logic_vector(3 downto 0);
      green_out: out std_logic_vector(3 downto 0);
      blue_out : out std_logic_vector(3 downto 0)
   );
  END COMPONENT;


-------------------------------------------------------------

-- Constants for various VGA Resolutions

-------------------------------------------------------------

--***640x480@60Hz***--  
--constant FRAME_WIDTH : natural := 640;
--constant FRAME_HEIGHT : natural := 480;

--constant H_FP : natural := 16; --H front porch width (pixels)
--constant H_PW : natural := 96; --H sync pulse width (pixels)
--constant H_MAX : natural := 800; --H total period (pixels)
--
--constant V_FP : natural := 10; --V front porch width (lines)
--constant V_PW : natural := 2; --V sync pulse width (lines)
--constant V_MAX : natural := 525; --V total period (lines)

--constant H_POL : std_logic := '0';
--constant V_POL : std_logic := '0';

--***800x600@60Hz***--
--constant FRAME_WIDTH : natural := 800;
--constant FRAME_HEIGHT : natural := 600;
--
--constant H_FP : natural := 40; --H front porch width (pixels)
--constant H_PW : natural := 128; --H sync pulse width (pixels)
--constant H_MAX : natural := 1056; --H total period (pixels)
--
--constant V_FP : natural := 1; --V front porch width (lines)
--constant V_PW : natural := 4; --V sync pulse width (lines)
--constant V_MAX : natural := 628; --V total period (lines)
--
--constant H_POL : std_logic := '1';
--constant V_POL : std_logic := '1';

--***1280x1024@60Hz***--
constant FRAME_WIDTH : natural := 1280;
constant FRAME_HEIGHT : natural := 1024;

constant H_FP : natural := 48; --H front porch width (pixels)
constant H_PW : natural := 112; --H sync pulse width (pixels)
constant H_MAX : natural := 1688; --H total period (pixels)

constant V_FP : natural := 1; --V front porch width (lines)
constant V_PW : natural := 3; --V sync pulse width (lines)
constant V_MAX : natural := 1066; --V total period (lines)

constant H_POL : std_logic := '1';
constant V_POL : std_logic := '1';

--***1920x1080@60Hz***--
--constant FRAME_WIDTH : natural := 1920;
--constant FRAME_HEIGHT : natural := 1080;
--
--constant H_FP : natural := 88; --H front porch width (pixels)
--constant H_PW : natural := 44; --H sync pulse width (pixels)
--constant H_MAX : natural := 2200; --H total period (pixels)
--
--constant V_FP : natural := 4; --V front porch width (lines)
--constant V_PW : natural := 5; --V sync pulse width (lines)
--constant V_MAX : natural := 1125; --V total period (lines)
--
--constant H_POL : std_logic := '1';
--constant V_POL : std_logic := '1';

------------------------------------------------------------------

-- Constants for setting the displayed logo size and coordinates

------------------------------------------------------------------
constant SZ_LOGO_WIDTH 	   : natural := 335; -- Width of the logo frame
constant SZ_LOGO_HEIGHT 	: natural := 280; -- Height of the logo frame

constant FRM_LOGO_H_LOC 	: natural := 25; --  Starting horizontal location of the logo frame
constant FRM_LOGO_V_LOC 	: natural := 176; -- Starting vertical location of the logo frame

-- Logo frame limits
constant LOGO_LEFT 			: natural := FRM_LOGO_H_LOC - 1;
constant LOGO_RIGHT 		   : natural := FRM_LOGO_H_LOC + SZ_LOGO_WIDTH + 1;
constant LOGO_TOP 			: natural := FRM_LOGO_V_LOC - 1;
constant LOGO_BOTTOM 		: natural := FRM_LOGO_V_LOC + SZ_LOGO_HEIGHT + 1;

------------------------------------------------------------------------------

-- Constants for setting the temperature display columns size and coordinates

-------------------------------------------------------------------------------
constant SZ_TEMP_WIDTH		: natural := 50; -- Width of a Temp Column
constant SZ_TEMP_HEIGHT		: natural := 472; -- Height of a Temp column

-- Starting Horizontal and Vertical locations of the three temperature columns
-- FPGA Temperature
constant FRM_XADC_TEMP_H_LOC	   : natural := 1050;
constant FRM_XADC_TEMP_V_LOC	   : natural := 80;
-- ADT7420 Temperature Sensor
constant FRM_ADT7420_TEMP_H_LOC	: natural := 1125;
constant FRM_ADT7420_TEMP_V_LOC	: natural := 80;
-- ADXL362 Accelerometer temperature
constant FRM_ADXL362_TEMP_H_LOC  : natural := 1200;
constant FRM_ADXL362_TEMP_V_LOC	: natural := 80;

-- Limits of the Temperature Column Frames
-- FPGA
constant XADC_TEMP_RIGHT	   : natural := FRM_XADC_TEMP_H_LOC + SZ_TEMP_WIDTH + 1;
constant XADC_TEMP_LEFT		   : natural := FRM_XADC_TEMP_H_LOC - 1;
constant XADC_TEMP_TOP		   : natural := FRM_XADC_TEMP_V_LOC - 1;
constant XADC_TEMP_BOTTOM	   : natural := FRM_XADC_TEMP_V_LOC + SZ_TEMP_HEIGHT + 1;
-- ADT7420
constant ADT7420_TEMP_RIGHT	: natural := FRM_ADT7420_TEMP_H_LOC + SZ_TEMP_WIDTH + 1;
constant ADT7420_TEMP_LEFT		: natural := FRM_ADT7420_TEMP_H_LOC - 1;
constant ADT7420_TEMP_TOP		: natural := FRM_ADT7420_TEMP_V_LOC - 1;
constant ADT7420_TEMP_BOTTOM	: natural := FRM_ADT7420_TEMP_V_LOC + SZ_TEMP_HEIGHT + 1;
--ADXL362
constant ADXL362_TEMP_RIGHT	: natural := FRM_ADXL362_TEMP_H_LOC + SZ_TEMP_WIDTH + 1;
constant ADXL362_TEMP_LEFT		: natural := FRM_ADXL362_TEMP_H_LOC - 1;
constant ADXL362_TEMP_TOP		: natural := FRM_ADXL362_TEMP_V_LOC - 1;
constant ADXL362_TEMP_BOTTOM	: natural := FRM_ADXL362_TEMP_V_LOC + SZ_TEMP_HEIGHT + 1;

----------------------------------------------------------------------------------------------------

-- Constants for setting size and location for TBOX - the white box holding the temperature columns

-----------------------------------------------------------------------------------------------------
constant SZ_TBOX_WIDTH 		: natural := 278; -- TBOX width
constant SZ_TBOX_HEIGHT		: natural := 553; -- TBOX height

constant FRM_TBOX_H_LOC		: natural := 985; -- TBOX starting horizontal location
constant FRM_TBOX_V_LOC		: natural := 40; -- TBOX starting vertical location
-- TBOX frame limits
constant TBOX_LEFT			: natural := FRM_TBOX_H_LOC - 1;
constant TBOX_RIGHT			: natural := FRM_TBOX_H_LOC + SZ_TBOX_WIDTH + 1;
constant TBOX_TOP				: natural := FRM_TBOX_V_LOC - 1;
constant TBOX_BOTTOM			: natural := FRM_TBOX_V_LOC + SZ_TBOX_HEIGHT + 1;

---------------------------------------------------------------------

-- Constants for setting size and locations for RGB LED data display 
-- Three columns: R, G and B are displayed for both LD16 and LD17

---------------------------------------------------------------------
constant SZ_RGB_WIDTH  		: natural := 50; -- width of one RGB column
constant SZ_RGB_HEIGHT 		: natural := 150; -- height of one RGB column

-- For one RGB LED for which RGB data is displayed, the R, G and B columns are horizontally aligned
-- The R, G and B columns are vertically aligned between the two LEDs
constant FRM_RGB_1_V_LOC 	: natural := 675; -- Starting V Location of the RGB LED LD16 Column
constant FRM_RGB_2_V_LOC 	: natural := FRM_RGB_1_V_LOC + SZ_RGB_HEIGHT + 15; -- Starting V Location of the RGB LED LD17 Column
constant FRM_RGB_R_H_LOC 	: natural := 1050; -- Starting H Location of the RGB LED RED Column
constant FRM_RGB_G_H_LOC 	: natural := 1125; -- Starting H Location of the RGB LED GREEN Column
constant FRM_RGB_B_H_LOC 	: natural := 1200; -- Starting H Location of the RGB LED BLUE Column

-- LD16 R, G, B Columns Top and Bottom limits
constant RGB1_COL_TOP	 		: natural := FRM_RGB_1_V_LOC - 1;
constant RGB1_COL_BOTTOM		: natural := FRM_RGB_1_V_LOC + SZ_RGB_HEIGHT + 1; 
-- LD17 R, G, B Columns Top and Bottom limits
constant RGB2_COL_TOP	 		: natural := FRM_RGB_2_V_LOC - 1;
constant RGB2_COL_BOTTOM		: natural := FRM_RGB_2_V_LOC + SZ_RGB_HEIGHT + 1; 
-- R columns Left and Right Location limits
constant RGB_R_COL_LEFT  		: natural := FRM_RGB_R_H_LOC - 1;
constant RGB_R_COL_RIGHT 		: natural := FRM_RGB_R_H_LOC + SZ_RGB_WIDTH + 1;
-- G columns Left and Right Location limits
constant RGB_G_COL_LEFT  		: natural := FRM_RGB_G_H_LOC - 1;
constant RGB_G_COL_RIGHT 		: natural := FRM_RGB_G_H_LOC + SZ_RGB_WIDTH + 1;
-- B columns Left and Right Location limits
constant RGB_B_COL_LEFT  		: natural := FRM_RGB_B_H_LOC - 1;
constant RGB_B_COL_RIGHT 		: natural := FRM_RGB_B_H_LOC + SZ_RGB_WIDTH + 1;

--------------------------------------------------------------------------------------------

-- Constants for setting size and location for LBOX - box for displaying the RGB LEDs color

--------------------------------------------------------------------------------------------
constant SZ_LBOX_WIDTH 		: natural := 278; -- LBOX width
constant SZ_LBOX_HEIGHT		: natural := 375; -- LBOX Height

constant FRM_LBOX_H_LOC		: natural := 985; -- LBOX starting horizontal location
constant FRM_LBOX_V_LOC		: natural := 615; -- LBOX starting vertical location
-- LBOX frame limits
constant LBOX_LEFT			: natural := FRM_LBOX_H_LOC - 1;
constant LBOX_RIGHT			: natural := FRM_LBOX_H_LOC + SZ_LBOX_WIDTH + 1;
constant LBOX_TOP				: natural := FRM_LBOX_V_LOC - 1;
constant LBOX_BOTTOM			: natural := FRM_LBOX_V_LOC + SZ_LBOX_HEIGHT + 1;

-----------------------------------------------------------------------------

-- Constants for setting size and location for the Microphone signal display

-----------------------------------------------------------------------------
constant SZ_MIC_WIDTH  		: natural := 915; -- Width of the Microphone frame
constant SZ_MIC_HEIGHT 		: natural := 375; -- Height of the Microphone frame

constant FRM_MIC_H_LOC 		: natural := 25; -- Microphone frame starting horizontal location
constant FRM_MIC_V_LOC 		: natural := 615; -- Microphone frame starting vertical location
-- Microphone display frame limits
constant MIC_LEFT				: natural := FRM_MIC_H_LOC - 1;
constant MIC_RIGHT			: natural := FRM_MIC_H_LOC + SZ_MIC_WIDTH + 1;
constant MIC_TOP				: natural := FRM_MIC_V_LOC - 1;
constant MIC_BOTTOM			: natural := FRM_MIC_V_LOC + SZ_MIC_HEIGHT + 1;

-------------------------------------------------------------------------

-- Constants for setting size and location for the Accelerometer display

--------------------------------------------------------------------------
-- Accelerometer X and Y data is scaled to 0-511 pixels, such as 0: -1g, 255: 0g, 511: +1g
constant SZ_ACL_XY_WIDTH   : natural := 511; -- Width of the Accelerometer frame X-Y Region
constant SZ_ACL_MAG_WIDTH  : natural := 45; -- Width of the Accelerometer frame Magnitude Region
constant SZ_ACL_WIDTH  		: natural := SZ_ACL_XY_WIDTH + SZ_ACL_MAG_WIDTH; -- Width of the entire Accelerometer frame
constant SZ_ACL_HEIGHT 		: natural := 511; -- Height of the Accelerometer frame

constant FRM_ACL_H_LOC 		: natural := 385; -- Accelerometer frame X-Y region starting horizontal location
constant FRM_ACL_MAG_LOC 	: natural := FRM_ACL_H_LOC + SZ_ACL_MAG_WIDTH; -- Accelerometer frame Magnitude Region starting horizontal location
constant FRM_ACL_V_LOC 		: natural := 80; -- Accelerometer frame starting vertical location
-- Accelerometer Display frame limits
constant ACL_LEFT				: natural := FRM_ACL_H_LOC - 1;
constant ACL_RIGHT			: natural := FRM_ACL_H_LOC + SZ_ACL_WIDTH + 1;
constant ACL_TOP				: natural := FRM_ACL_V_LOC - 1;
constant ACL_BOTTOM			: natural := FRM_ACL_V_LOC + SZ_ACL_HEIGHT + 1;


-------------------------------------------------------------------------

-- Signal Declarations

-------------------------------------------------------------------------


-------------------------------------------------------------------------

-- VGA Controller specific signals: Counters, Sync, R, G, B

-------------------------------------------------------------------------
-- Pixel clock, in this case 108 MHz
signal pxl_clk : std_logic;
-- The active signal is used to signal the active region of the screen (when not blank)
signal active  : std_logic;

-- Horizontal and Vertical counters
signal h_cntr_reg : std_logic_vector(11 downto 0) := (others =>'0');
signal v_cntr_reg : std_logic_vector(11 downto 0) := (others =>'0');

-- Pipe Horizontal and Vertical Counters
signal h_cntr_reg_dly   : std_logic_vector(11 downto 0) := (others => '0');
signal v_cntr_reg_dly   : std_logic_vector(11 downto 0) := (others => '0');

-- Horizontal and Vertical Sync
signal h_sync_reg : std_logic := not(H_POL);
signal v_sync_reg : std_logic := not(V_POL);
-- Pipe Horizontal and Vertical Sync
signal h_sync_reg_dly : std_logic := not(H_POL);
signal v_sync_reg_dly : std_logic :=  not(V_POL);

-- VGA R, G and B signals coming from the main multiplexers
signal vga_red_cmb   : std_logic_vector(3 downto 0);
signal vga_green_cmb : std_logic_vector(3 downto 0);
signal vga_blue_cmb  : std_logic_vector(3 downto 0);
--The main VGA R, G and B signals, validated by active
signal vga_red    : std_logic_vector(3 downto 0);
signal vga_green  : std_logic_vector(3 downto 0);
signal vga_blue   : std_logic_vector(3 downto 0);
-- Register VGA R, G and B signals
signal vga_red_reg   : std_logic_vector(3 downto 0) := (others =>'0');
signal vga_green_reg : std_logic_vector(3 downto 0) := (others =>'0');
signal vga_blue_reg  : std_logic_vector(3 downto 0) := (others =>'0');

-------------------------------------------------------------------------

-- Signals for registering the inputs

-------------------------------------------------------------------------
signal RGB_LED_RED_REG     : std_logic_vector (4 downto 0);
signal RGB_LED_BLUE_REG    : std_logic_vector (4 downto 0);
signal RGB_LED_GREEN_REG   : std_logic_vector (4 downto 0);

signal XADC_TEMP_VALUE_I_REG     : std_logic_vector (11 downto 0);
signal ADT7420_TEMP_VALUE_I_REG  : std_logic_vector (12 downto 0);
signal ADXL362_TEMP_VALUE_I_REG  : std_logic_vector (11 downto 0);

signal ACCEL_RADIUS_REG : STD_LOGIC_VECTOR (11 downto 0);
signal LEVEL_THRESH_REG : STD_LOGIC_VECTOR (11 downto 0);
signal ACL_X_IN_REG     : STD_LOGIC_VECTOR (8 downto 0);
signal ACL_Y_IN_REG     : STD_LOGIC_VECTOR (8 downto 0);
signal ACL_MAG_IN_REG   : STD_LOGIC_VECTOR (11 downto 0);

signal MIC_M_DATA_I_REG : STD_LOGIC;

signal MOUSE_X_POS_REG  : std_logic_vector (11 downto 0);
signal MOUSE_Y_POS_REG  : std_logic_vector (11 downto 0);
signal MOUSE_LEFT_BUTTON_REG : std_logic;

-----------------------------------------------------------
-- Signals for generating the background (moving colorbar)
-----------------------------------------------------------
signal cntDyn				: integer range 0 to 2**28-1; -- counter for generating the colorbar
signal intHcnt				: integer range 0 to H_MAX - 1;
signal intVcnt				: integer range 0 to V_MAX - 1;
-- Colorbar red, greeen and blue signals
signal bg_red 				: std_logic_vector(3 downto 0);
signal bg_blue 			: std_logic_vector(3 downto 0);
signal bg_green 			: std_logic_vector(3 downto 0);
-- Pipe the colorbar red, green and blue signals
signal bg_red_dly			: std_logic_vector(3 downto 0) := (others => '0');
signal bg_green_dly		: std_logic_vector(3 downto 0) := (others => '0');
signal bg_blue_dly		: std_logic_vector(3 downto 0) := (others => '0');


-------------------------------------------------------------------------

-- Interconnection signals for the displaying components

-------------------------------------------------------------------------

-- Digilent and Analog Devices logo display signals
signal logo_red   : std_logic_vector(3 downto 0);
signal logo_blue 	: std_logic_vector(3 downto 0);
signal logo_green : std_logic_vector(3 downto 0);

-- FPGA Temperature Display Signals
signal xadc_temp_red    : std_logic_vector (3 downto 0);
signal xadc_temp_green  : std_logic_vector (3 downto 0);
signal xadc_temp_blue   : std_logic_vector (3 downto 0);

-- ADT740 Temperature Sensor Display Signals
signal adt7420_temp_red    : std_logic_vector (3 downto 0);
signal adt7420_temp_green  : std_logic_vector (3 downto 0);
signal adt7420_temp_blue   : std_logic_vector (3 downto 0);

-- ADXL362 Accelerometer Temperature Sensor Display Signals
signal adxl362_temp_red    : std_logic_vector (3 downto 0);
signal adxl362_temp_green  : std_logic_vector (3 downto 0);
signal adxl362_temp_blue   : std_logic_vector (3 downto 0);

-- TBOX (frame holding the temperature columns) display signals
signal tbox_red			: std_logic_vector(3 downto 0);
signal tbox_blue			: std_logic_vector(3 downto 0);
signal tbox_green			: std_logic_vector(3 downto 0);

-- Microphone data display signals
signal mic_red 			: std_logic_vector(3 downto 0);
signal mic_blue 			: std_logic_vector(3 downto 0);
signal mic_green 			: std_logic_vector(3 downto 0);

-- RGB LED Red, Green and Blue Display signals for the three columns
signal rgb_r_red_col	   : std_logic_vector(3 downto 0); -- Red signal for the Red column
signal rgb_g_red_col    : std_logic_vector(3 downto 0); -- Green signal for the Red column
signal rgb_b_red_col    : std_logic_vector(3 downto 0); -- Blue signal for the Red column

signal rgb_r_green_col	: std_logic_vector(3 downto 0); -- Red signal for the Green column
signal rgb_g_green_col 	: std_logic_vector(3 downto 0); -- Green signal for the Green column
signal rgb_b_green_col 	: std_logic_vector(3 downto 0); -- Blue signal for the Green column

signal rgb_r_blue_col	: std_logic_vector(3 downto 0); -- Red signal for the Blue column
signal rgb_g_blue_col 	: std_logic_vector(3 downto 0); -- Green signal for the Blue column
signal rgb_b_blue_col 	: std_logic_vector(3 downto 0); -- Blue signal for the Blue column

--Lbox - frame holding the RGB LED columns display signals
signal lbox_red    : std_logic_vector (3 downto 0);
signal lbox_green  : std_logic_vector (3 downto 0);
signal lbox_blue   : std_logic_vector (3 downto 0);

-- Accelerometer display dignals
signal acl_red    : std_logic_vector(3 downto 0);
signal acl_blue   : std_logic_vector(3 downto 0);
signal acl_green	: std_logic_vector(3 downto 0);

-- Mouse cursor display signals
signal mouse_cursor_red    : std_logic_vector (3 downto 0) := (others => '0');
signal mouse_cursor_blue   : std_logic_vector (3 downto 0) := (others => '0');
signal mouse_cursor_green  : std_logic_vector (3 downto 0) := (others => '0');
-- Mouse cursor enable display signals
signal enable_mouse_display:  std_logic;

-- Overlay display signal
signal overlay_en : std_logic;

---------------------------------------------------------------------------------

-- Pipe all of the interconnection signals coming from the displaying components

---------------------------------------------------------------------------------

-- Registered Digilent and Analog Devices logo display signals
signal logo_red_dly			: std_logic_vector(3 downto 0);
signal logo_blue_dly 		: std_logic_vector(3 downto 0);
signal logo_green_dly 		: std_logic_vector(3 downto 0);

-- Registered FPGA Temperature Display Signals
signal xadc_temp_red_dly    : std_logic_vector (3 downto 0);
signal xadc_temp_green_dly  : std_logic_vector (3 downto 0);
signal xadc_temp_blue_dly   : std_logic_vector (3 downto 0);

-- Registered ADT740 Temperature Sensor Display Signals
signal adt7420_temp_red_dly    : std_logic_vector (3 downto 0);
signal adt7420_temp_green_dly  : std_logic_vector (3 downto 0);
signal adt7420_temp_blue_dly   : std_logic_vector (3 downto 0);

-- Registered ADXL362 Accelerometer Temperature Sensor Display Signals
signal adxl362_temp_red_dly    : std_logic_vector (3 downto 0);
signal adxl362_temp_green_dly  : std_logic_vector (3 downto 0);
signal adxl362_temp_blue_dly   : std_logic_vector (3 downto 0);

-- TBOX (frame holding the temperature columns) color is white,
-- therefore TBOX signals will not be registered again

-- Registered Microphone data display signals
signal mic_red_dly 			: std_logic_vector(3 downto 0);
signal mic_blue_dly 			: std_logic_vector(3 downto 0);
signal mic_green_dly 		: std_logic_vector(3 downto 0);

-- Registered RGB LED Red, Green and Blue Display signals for the three columns
signal rgb_r_red_col_dly		: std_logic_vector(3 downto 0); -- Red signal for the Red column
signal rgb_g_red_col_dly 		: std_logic_vector(3 downto 0); -- Green signal for the Red column
signal rgb_b_red_col_dly 		: std_logic_vector(3 downto 0); -- Blue signal for the Red column

signal rgb_r_green_col_dly		: std_logic_vector(3 downto 0); -- Red signal for the Green column
signal rgb_g_green_col_dly 	: std_logic_vector(3 downto 0); -- Green signal for the Green column
signal rgb_b_green_col_dly 	: std_logic_vector(3 downto 0); -- Blue signal for the Green column

signal rgb_r_blue_col_dly		: std_logic_vector(3 downto 0); -- Red signal for the Blue column
signal rgb_g_blue_col_dly 		: std_logic_vector(3 downto 0); -- Green signal for the Blue column
signal rgb_b_blue_col_dly 		: std_logic_vector(3 downto 0); -- Blue signal for the Blue column

-- Lbox (frame holding the RGB LED columns) signals will be in fact 
-- the incoming RGB LED signals, therefore will not be registered again

-- Registered Accelerometer display dignals
signal acl_red_dly 			: std_logic_vector(3 downto 0);
signal acl_blue_dly 			: std_logic_vector(3 downto 0);
signal acl_green_dly			: std_logic_vector(3 downto 0);

-- Registered Mouse cursor display signals
signal mouse_cursor_red_dly   : std_logic_vector (3 downto 0) := (others => '0');
signal mouse_cursor_blue_dly  : std_logic_vector (3 downto 0) := (others => '0');
signal mouse_cursor_green_dly : std_logic_vector (3 downto 0) := (others => '0');
-- Registered Mouse cursor enable display signals
signal enable_mouse_display_dly  :  std_logic;

-- Registered Overlay display signal
signal overlay_en_dly : std_logic;

begin
  
------------------------------------

-- Generate the 108 MHz pixel clock 

------------------------------------
   Inst_PxlClkGen: PxlClkGen
   port map
    (-- Clock in ports
     CLK_IN1   => CLK_I,
     -- Clock out ports
     CLK_OUT1  => pxl_clk,
     -- Status and control signals
     LOCKED   => open
    );

---------------------------------------------------------------

-- Generate Horizontal, Vertical counters and the Sync signals

---------------------------------------------------------------
  -- Horizontal counter
  process (pxl_clk)
  begin
    if (rising_edge(pxl_clk)) then
      if (h_cntr_reg = (H_MAX - 1)) then
        h_cntr_reg <= (others =>'0');
      else
        h_cntr_reg <= h_cntr_reg + 1;
      end if;
    end if;
  end process;
  -- Vertical counter
  process (pxl_clk)
  begin
    if (rising_edge(pxl_clk)) then
      if ((h_cntr_reg = (H_MAX - 1)) and (v_cntr_reg = (V_MAX - 1))) then
        v_cntr_reg <= (others =>'0');
      elsif (h_cntr_reg = (H_MAX - 1)) then
        v_cntr_reg <= v_cntr_reg + 1;
      end if;
    end if;
  end process;
  -- Horizontal sync
  process (pxl_clk)
  begin
    if (rising_edge(pxl_clk)) then
      if (h_cntr_reg >= (H_FP + FRAME_WIDTH - 1)) and (h_cntr_reg < (H_FP + FRAME_WIDTH + H_PW - 1)) then
        h_sync_reg <= H_POL;
      else
        h_sync_reg <= not(H_POL);
      end if;
    end if;
  end process;
  -- Vertical sync
  process (pxl_clk)
  begin
    if (rising_edge(pxl_clk)) then
      if (v_cntr_reg >= (V_FP + FRAME_HEIGHT - 1)) and (v_cntr_reg < (V_FP + FRAME_HEIGHT + V_PW - 1)) then
        v_sync_reg <= V_POL;
      else
        v_sync_reg <= not(V_POL);
      end if;
    end if;
  end process;
  
--------------------

-- The active 

--------------------  
  -- active signal
  active <= '1' when h_cntr_reg_dly < FRAME_WIDTH and v_cntr_reg_dly < FRAME_HEIGHT
            else '0';

--------------------

-- Register Inputs

--------------------
register_inputs: process (pxl_clk, v_sync_reg)
  begin
    if (rising_edge(pxl_clk)) then
      if v_sync_reg = V_POL then -- All of the signals, except the incoming microphone data 
                                 -- have lover frequencies than the vertical refresh rate,
                                 -- therefore will be registered in the blanking area
         RGB_LED_RED_REG   <= RGB_LED_RED (4 downto 0); -- The RGB LEDs are turned on at a lower than maximum intensity,
         RGB_LED_GREEN_REG <= RGB_LED_GREEN (4 downto 0); -- therefore the five least significant bits are used only
         RGB_LED_BLUE_REG  <= RGB_LED_BLUE (4 downto 0);
         
         XADC_TEMP_VALUE_I_REG      <= XADC_TEMP_VALUE_I;
         ADT7420_TEMP_VALUE_I_REG   <= ADT7420_TEMP_VALUE_I;
         ADXL362_TEMP_VALUE_I_REG   <= ADXL362_TEMP_VALUE_I;

         ACCEL_RADIUS_REG <= ACCEL_RADIUS;
         LEVEL_THRESH_REG <= LEVEL_THRESH;
			ACL_X_IN_REG <= ACL_X_IN;
         ACL_Y_IN_REG <= ACL_Y_IN;
         ACL_MAG_IN_REG <= ACL_MAG_IN;
     
         
         MOUSE_X_POS_REG <= MOUSE_X_POS;
         MOUSE_Y_POS_REG <= MOUSE_Y_POS;
         MOUSE_LEFT_BUTTON_REG <= MOUSE_LEFT_BUTTON_REG;
      end if;   
      -- Incoming Microphone data rate is faster than VSYNC, therefore is registered on the pixel clock
      MIC_M_DATA_I_REG <= MIC_M_DATA_I;
    end if;
end process register_inputs;

--------------------------

-- Logo display instance

--------------------------
 	Inst_LogoDisplay: LogoDisplay 
	GENERIC MAP(
		X_START	=> FRM_LOGO_H_LOC,
		Y_START	=> FRM_LOGO_V_LOC
	)
	PORT MAP(
		CLK_I => pxl_clk,
		H_COUNT_I => h_cntr_reg,
		V_COUNT_I => v_cntr_reg,
		RED_O    => logo_red,
		BLUE_O   => logo_blue,
		GREEN_O  => logo_green
	);
   
 --------------------------------

-- Temperature display instances

---------------------------------
 ----------------------------------------------------
-- FPGA Temperature - from the XADC temperature data
-----------------------------------------------------
   Inst_XadcTempDisplay: TempDisplay
   GENERIC MAP(
           X_TMP_COL_WIDTH    => SZ_TEMP_WIDTH, -- width of the TEMP column
           Y_TMP_COL_HEIGHT   => SZ_TEMP_HEIGHT,-- height of the TEMP column
           X_TMP_H_LOC        => FRM_XADC_TEMP_H_LOC, -- X Location of the FPGA TEMP Column
           Y_TMP_V_LOC        => FRM_XADC_TEMP_V_LOC, -- Y Location of the FPGA TEMP Column
           INPUT_DATA_WIDTH   => 12,
           TMP_TYPE           => "XADC"
           )
    PORT MAP ( 
           CLK_I        => pxl_clk,
           TEMP_IN      => XADC_TEMP_VALUE_I_REG,
           H_COUNT_I    => h_cntr_reg,
           V_COUNT_I    => v_cntr_reg,
           -- Temperature Red, Green and Blue signals
           TEMP_R_OUT   => xadc_temp_red,
           TEMP_G_OUT   => xadc_temp_green,
           TEMP_B_OUT   => xadc_temp_blue
          );
          
-------------------------------------------------
-- ADT740 onboard temperature sensor temperature
-------------------------------------------------
   Inst_Adt7420TempDisplay: TempDisplay
   GENERIC MAP (
           X_TMP_COL_WIDTH    => SZ_TEMP_WIDTH, -- width of the TEMP column
           Y_TMP_COL_HEIGHT   => SZ_TEMP_HEIGHT,-- height of the TEMP column
           X_TMP_H_LOC        => FRM_ADT7420_TEMP_H_LOC, -- X Location of the Temp Sensor Column
           Y_TMP_V_LOC        => FRM_ADT7420_TEMP_V_LOC, -- Y Location of the Temp Sensor Column
           INPUT_DATA_WIDTH   => 13,
           TMP_TYPE           => "TEMP_ACC"
           )
    PORT MAP (
           CLK_I        => pxl_clk,
           TEMP_IN      => ADT7420_TEMP_VALUE_I_REG,
           H_COUNT_I    => h_cntr_reg,
           V_COUNT_I    => v_cntr_reg,
           -- Temperature Red, Green and Blue signals
           TEMP_R_OUT   => adt7420_temp_red,
           TEMP_G_OUT   => adt7420_temp_green,
           TEMP_B_OUT   => adt7420_temp_blue
          );
----------------------------------------------------
-- ADXL362 onboard accelerometer temperature sensor
----------------------------------------------------
    Inst_Adxl362TempDisplay: TempDisplay
   GENERIC MAP(
           X_TMP_COL_WIDTH    => SZ_TEMP_WIDTH, -- width of the TEMP column
           Y_TMP_COL_HEIGHT   => SZ_TEMP_HEIGHT,-- height of the TEMP column
           X_TMP_H_LOC        => FRM_ADXL362_TEMP_H_LOC, -- X Location of the ACC Temp Column
           Y_TMP_V_LOC        => FRM_ADXL362_TEMP_V_LOC, -- Y Location of the Acc Temp Column
           INPUT_DATA_WIDTH   => 12,
           TMP_TYPE           => "TEMP_ACC"
           )
    PORT MAP (
           CLK_I        => pxl_clk,
           TEMP_IN      => ADXL362_TEMP_VALUE_I_REG,
           H_COUNT_I    => h_cntr_reg,
           V_COUNT_I    => v_cntr_reg,
           -- Temperature Red, Green and Blue signals
           TEMP_R_OUT   => adxl362_temp_red,
           TEMP_G_OUT   => adxl362_temp_green,
           TEMP_B_OUT   => adxl362_temp_blue
          ); 

-----------------------------

-- RGB LED display instance

-----------------------------
   Inst_RGBLedDisplay: RgbLedDisplay
   GENERIC MAP(
           X_RGB_COL_WIDTH    => SZ_RGB_WIDTH, -- width of one RGB column
           Y_RGB_COL_HEIGHT   => SZ_RGB_HEIGHT,-- height of one RGB column
           X_RGB_R_LOC        => FRM_RGB_R_H_LOC, -- X Location of the RGB LED RED Column
           X_RGB_G_LOC        => FRM_RGB_G_H_LOC, -- X Location of the RGB LED GREEN Column
           X_RGB_B_LOC        => FRM_RGB_B_H_LOC, -- X Location of the RGB LED BLUE Column
           Y_RGB_1_LOC        => FRM_RGB_1_V_LOC, -- Y Location of the RGB LED LD16 Column
           Y_RGB_2_LOC        => FRM_RGB_2_V_LOC -- Y Location of the RGB LED LD17 Column
           )
    PORT MAP(
           pxl_clk        => pxl_clk,
           RGB_LED_RED    => RGB_LED_RED_REG,
           RGB_LED_GREEN  => RGB_LED_GREEN_REG,
           RGB_LED_BLUE   => RGB_LED_BLUE_REG,
           H_COUNT_I      => h_cntr_reg,
           V_COUNT_I      => v_cntr_reg,
           -- RGB LED RED signal Data for the three columns
           RGB_LED_R_RED_COL     => rgb_r_red_col,
           RGB_LED_R_GREEN_COL   => rgb_r_green_col,
           RGB_LED_R_BLUE_COL    => rgb_r_blue_col,
           -- RGB LED GREEN signal Data for the three columns
           RGB_LED_G_RED_COL     => rgb_g_red_col,
           RGB_LED_G_GREEN_COL   => rgb_g_green_col,
           RGB_LED_G_BLUE_COL    => rgb_g_blue_col,
           -- RGB LED BLUE signal Data for the three columns
           RGB_LED_B_RED_COL     => rgb_b_red_col,
           RGB_LED_B_GREEN_COL   => rgb_b_green_col,
           RGB_LED_B_BLUE_COL    => rgb_b_blue_col
          ); 
          
--------------------------------------

-- Microphone signal display instance

--------------------------------------
	Inst_MicDisplay: MicDisplay 
	GENERIC MAP(
		X_WIDTH 	         => SZ_MIC_WIDTH,
      Y_HEIGHT 		   => SZ_MIC_HEIGHT,
      X_START 			   => FRM_MIC_H_LOC,
      Y_START 			   => FRM_MIC_V_LOC,
      PXLCLK_FREQ_HZ    => 108000000,
      H_MAX             => H_MAX,
      SAMPLE_RATE_DIV   => 4096,
      BG_COLOR 		   => x"FFF",
      ACTIVE_COLOR	   => x"008"
	)
	PORT MAP(
		CLK_I             => pxl_clk,
      SYSCLK            => CLK_I,
		MIC_M_DATA_I      => MIC_M_DATA_I_REG,
      MIC_M_CLK_RISING  => MIC_M_CLK_RISING,
		H_COUNT_I         => h_cntr_reg,
		V_COUNT_I         => v_cntr_reg,
		RED_O             => mic_red,
		GREEN_O           => mic_green,
		BLUE_O            => mic_blue
      );

              
----------------------------------

-- Accelerometer display instance

----------------------------------
   Inst_AccelDisplay: AccelDisplay 
   GENERIC MAP
   (
      X_XY_WIDTH		=> SZ_ACL_XY_WIDTH, -- Width of the Accelerometer frame X-Y region
      X_MAG_WIDTH    => SZ_ACL_MAG_WIDTH, -- Width of the Accelerometer frame Magnitude region
      Y_HEIGHT 		=> SZ_ACL_HEIGHT, -- Height of the Accelerometer frame
      X_START 			=> FRM_ACL_H_LOC, -- Accelerometer frame X-Y region starting horizontal location
      Y_START 			=> FRM_ACL_V_LOC, -- Accelerometer frame starting vertical location
      BG_COLOR 		=> x"FFF", -- White
      ACTIVE_COLOR 	=> x"0F0", -- Green
      WARNING_COLOR 	=> x"F00" -- Red
    )
    PORT MAP
    (
      CLK_I => pxl_clk,
      ACCEL_X_I => ACL_X_IN_REG,
      ACCEL_Y_I => ACL_Y_IN_REG,
      ACCEL_MAG_I => ACL_MAG_IN_REG(8 DOWNTO 0), -- only 9 bits are taken into account, data is scaled between 0-500
      H_COUNT_I => h_cntr_reg,
      V_COUNT_I => v_cntr_reg,
      ACCEL_RADIUS => ACCEL_RADIUS_REG,
      LEVEL_THRESH => LEVEL_THRESH_REG,
      RED_O => acl_red,
      BLUE_O => acl_blue,
      GREEN_O => acl_green
	 );
    

----------------------------------

-- Mouse Cursor display instance

----------------------------------
   Inst_MouseDisplay: MouseDisplay
   PORT MAP 
   (
      pixel_clk   => pxl_clk,
      xpos        => MOUSE_X_POS_REG, 
      ypos        => MOUSE_Y_POS_REG,
      hcount      => h_cntr_reg,
      vcount      => v_cntr_reg,
      enable_mouse_display_out  => enable_mouse_display,
      red_out     => mouse_cursor_red,
      green_out   => mouse_cursor_green,
      blue_out    => mouse_cursor_blue
   );

----------------------------------

-- Overlay display instance

----------------------------------
    	Inst_OverlayCtrl: OverlayCtl 
      PORT MAP
      (
		CLK_I       => pxl_clk,
		VSYNC_I     => v_sync_reg,
		ACTIVE_I    => active,
		OVERLAY_O   => overlay_en
      );
  
  
---------------------------------------

-- Generate moving colorbar background

---------------------------------------

	process(pxl_clk)
	begin
		if(rising_edge(pxl_clk)) then
			cntdyn <= cntdyn + 1;
		end if;
	end process;
   
  	intHcnt <= conv_integer(h_cntr_reg);
	intVcnt <= conv_integer(v_cntr_reg);
	
	bg_red <= conv_std_logic_vector((-intvcnt - inthcnt - cntDyn/2**20),8)(7 downto 4);
	bg_green <= conv_std_logic_vector((inthcnt - cntDyn/2**20),8)(7 downto 4);
	bg_blue <= conv_std_logic_vector((intvcnt - cntDyn/2**20),8)(7 downto 4);
   
---------------------------------------------------------------------------
-- Generate LBOX (the frame that holds the RGB LED column) signals
-- LBOX signals are, in fact, the MSB of the incoming RBG LED data signals
---------------------------------------------------------------------------
   lbox_red    <= RGB_LED_RED_REG(4 downto 1);
   lbox_green  <= RGB_LED_GREEN_REG(4 downto 1);
   lbox_blue   <= RGB_LED_BLUE_REG(4 downto 1);

----------------------------------------------------------------------
-- TBOX (the frame that holds the temperature columns) color is white
----------------------------------------------------------------------
   tbox_red    <= X"F";
   tbox_blue   <= X"F";
   tbox_green  <= X"F";


---------------------------------------------------------------------------------------------------

-- Register Outputs coming from the displaying components and the horizontal and vertical counters

---------------------------------------------------------------------------------------------------
  process (pxl_clk)
  begin
    if (rising_edge(pxl_clk)) then
   
      logo_red_dly		<= logo_red;
		logo_green_dly	   <= logo_green;
		logo_blue_dly		<= logo_blue;

      xadc_temp_red_dly    <= xadc_temp_red;
      xadc_temp_green_dly  <= xadc_temp_green;
      xadc_temp_blue_dly   <= xadc_temp_blue;

      adt7420_temp_red_dly    <= adt7420_temp_red;
      adt7420_temp_green_dly  <= adt7420_temp_green;
      adt7420_temp_blue_dly   <= adt7420_temp_blue;

      adxl362_temp_red_dly    <= adxl362_temp_red;
      adxl362_temp_green_dly  <= adxl362_temp_green;
      adxl362_temp_blue_dly   <= adxl362_temp_blue;

      rgb_r_red_col_dly       <= rgb_r_red_col;
      rgb_g_red_col_dly 		<= rgb_g_red_col;
      rgb_b_red_col_dly 		<= rgb_b_red_col;

      rgb_r_green_col_dly		<= rgb_r_green_col;
      rgb_g_green_col_dly 		<= rgb_g_green_col;
      rgb_b_green_col_dly 		<= rgb_b_green_col;

      rgb_r_blue_col_dly		<= rgb_r_blue_col;
      rgb_g_blue_col_dly 		<= rgb_g_blue_col;
      rgb_b_blue_col_dly 		<= rgb_b_blue_col;
      
	   mic_red_dly			<= mic_red;
		mic_green_dly		<= mic_green;
		mic_blue_dly		<= mic_blue;

      acl_red_dly			<= acl_red;
		acl_green_dly		<= acl_green;
		acl_blue_dly		<= acl_blue;
      
      bg_red_dly			<= bg_red;
		bg_green_dly		<= bg_green;
		bg_blue_dly			<= bg_blue;

      mouse_cursor_red_dly    <= mouse_cursor_red;
      mouse_cursor_blue_dly   <= mouse_cursor_blue;
      mouse_cursor_green_dly  <= mouse_cursor_green;

      enable_mouse_display_dly   <= enable_mouse_display;

      overlay_en_dly <= overlay_en;
      
      h_cntr_reg_dly <= h_cntr_reg;
		v_cntr_reg_dly <= v_cntr_reg;
      
      
    end if;
  end process;


-------------------------------------------------------------

-- Main Multiplexers for the VGA Red, Green and Blue signals

-------------------------------------------------------------
----------
-- Red
----------

  vga_red <=   -- Mouse_cursor_display is on the top of others
               mouse_cursor_red_dly when enable_mouse_display_dly = '1'
               else
               -- Overlay display is black 
               x"0" when overlay_en_dly = '1'
               else
               -- logo display
               logo_red_dly when h_cntr_reg_dly > LOGO_LEFT and h_cntr_reg_dly < LOGO_RIGHT 
                             and v_cntr_reg_dly < LOGO_BOTTOM and v_cntr_reg_dly > LOGO_TOP               
               else
               -- Temperature display
               xadc_temp_red_dly when h_cntr_reg_dly > XADC_TEMP_LEFT and h_cntr_reg_dly < XADC_TEMP_RIGHT 
                                  and v_cntr_reg_dly > XADC_TEMP_TOP and v_cntr_reg_dly < XADC_TEMP_BOTTOM
               else
               adt7420_temp_red_dly when h_cntr_reg_dly > ADT7420_TEMP_LEFT and h_cntr_reg_dly < ADT7420_TEMP_RIGHT 
                                     and v_cntr_reg_dly > ADT7420_TEMP_TOP and v_cntr_reg_dly < ADT7420_TEMP_BOTTOM
               else
               adxl362_temp_red_dly when h_cntr_reg_dly > ADXL362_TEMP_LEFT and h_cntr_reg_dly < ADXL362_TEMP_RIGHT 
                                     and v_cntr_reg_dly > ADXL362_TEMP_TOP and v_cntr_reg_dly < ADXL362_TEMP_BOTTOM
               else
               -- TBOX display
     			   tbox_red when h_cntr_reg_dly > TBOX_LEFT and h_cntr_reg_dly < TBOX_RIGHT 
                         and v_cntr_reg_dly < TBOX_BOTTOM and v_cntr_reg_dly > TBOX_TOP
               else
               --  RGB Led display
               rgb_r_red_col_dly when (h_cntr_reg_dly > RGB_R_COL_LEFT and h_cntr_reg_dly < RGB_R_COL_RIGHT) 
                                  and 
                                     (
                                      (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                      or
                                      (v_cntr_reg_dly > RGB2_COL_TOP and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                     )
               else
               rgb_r_green_col_dly when (h_cntr_reg_dly > RGB_G_COL_LEFT and h_cntr_reg_dly < RGB_G_COL_RIGHT) 
                                    and 
                                      (
                                      (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                    or
                                      (v_cntr_reg_dly > RGB2_COL_TOP - 1 and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                      )
               else
               rgb_r_blue_col_dly when (h_cntr_reg_dly > RGB_B_COL_LEFT and h_cntr_reg_dly < RGB_B_COL_RIGHT) 
                                    and 
                                       (
                                        (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                        or
                                        (v_cntr_reg_dly > RGB2_COL_TOP and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                       )
               else
               -- LBOX display
               lbox_red when h_cntr_reg_dly > LBOX_LEFT and h_cntr_reg_dly < LBOX_RIGHT 
                         and v_cntr_reg_dly < LBOX_BOTTOM and v_cntr_reg_dly > LBOX_TOP
               else
               -- Microphone data display
               mic_red_dly when h_cntr_reg_dly > MIC_LEFT and h_cntr_reg_dly < MIC_RIGHT 
                            and v_cntr_reg_dly > MIC_TOP and v_cntr_reg_dly < MIC_BOTTOM
               else
               -- Accelerometer display   
               acl_red_dly when h_cntr_reg_dly > ACL_LEFT and h_cntr_reg_dly < ACL_RIGHT 
                            and v_cntr_reg_dly > ACL_TOP and v_cntr_reg_dly < ACL_BOTTOM
               else
               -- Colorbar will be on the backround
               bg_red_dly;
                
-----------
-- Green
-----------

  vga_green <= -- Mouse_cursor_display is on the top of others
               mouse_cursor_green_dly when enable_mouse_display_dly = '1'
               else
               -- Overlay display is black 
               x"0" when overlay_en_dly = '1'
               else
               -- logo display
               logo_green_dly when h_cntr_reg_dly > LOGO_LEFT and h_cntr_reg_dly < LOGO_RIGHT 
                               and v_cntr_reg_dly < LOGO_BOTTOM and v_cntr_reg_dly > LOGO_TOP
               else
               -- Temperature display
               xadc_temp_green_dly when h_cntr_reg_dly > XADC_TEMP_LEFT and h_cntr_reg_dly < XADC_TEMP_RIGHT 
                                    and v_cntr_reg_dly > XADC_TEMP_TOP and v_cntr_reg_dly < XADC_TEMP_BOTTOM
               else
               adt7420_temp_green_dly when h_cntr_reg_dly > ADT7420_TEMP_LEFT and h_cntr_reg_dly < ADT7420_TEMP_RIGHT 
                                       and v_cntr_reg_dly > ADT7420_TEMP_TOP and v_cntr_reg_dly < ADT7420_TEMP_BOTTOM
               else
               adxl362_temp_green_dly when h_cntr_reg_dly > ADXL362_TEMP_LEFT and h_cntr_reg_dly < ADXL362_TEMP_RIGHT 
                                       and v_cntr_reg_dly > ADXL362_TEMP_TOP and v_cntr_reg_dly < ADXL362_TEMP_BOTTOM
               else
               -- TBOX display
     			   tbox_green when h_cntr_reg_dly > TBOX_LEFT and h_cntr_reg_dly < TBOX_RIGHT 
                           and v_cntr_reg_dly < TBOX_BOTTOM and v_cntr_reg_dly > TBOX_TOP
               else
               --  RGB Led display
               rgb_g_red_col_dly when (h_cntr_reg_dly > RGB_R_COL_LEFT and h_cntr_reg_dly < RGB_R_COL_RIGHT) 
                                  and 
                                     (
                                      (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                      or
                                      (v_cntr_reg_dly > RGB2_COL_TOP and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                     )
               else
               rgb_g_green_col_dly when (h_cntr_reg_dly > RGB_G_COL_LEFT and h_cntr_reg_dly < RGB_G_COL_RIGHT) 
                                    and 
                                      (
                                      (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                    or
                                      (v_cntr_reg_dly > RGB2_COL_TOP - 1 and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                      )
               else
               rgb_g_blue_col_dly when (h_cntr_reg_dly > RGB_B_COL_LEFT and h_cntr_reg_dly < RGB_B_COL_RIGHT) 
                                    and 
                                       (
                                        (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                        or
                                        (v_cntr_reg_dly > RGB2_COL_TOP and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                       )
               else
               -- LBOX display
               lbox_green when h_cntr_reg_dly > LBOX_LEFT and h_cntr_reg_dly < LBOX_RIGHT 
                           and v_cntr_reg_dly < LBOX_BOTTOM and v_cntr_reg_dly > LBOX_TOP
               else
               -- Microphone data display
               mic_green_dly when h_cntr_reg_dly > MIC_LEFT and h_cntr_reg_dly < MIC_RIGHT 
                              and v_cntr_reg_dly > MIC_TOP and v_cntr_reg_dly < MIC_BOTTOM
               else
               -- Accelerometer display
               acl_green_dly when h_cntr_reg_dly > ACL_LEFT and h_cntr_reg_dly < ACL_RIGHT 
                              and v_cntr_reg_dly > ACL_TOP and v_cntr_reg_dly < ACL_BOTTOM
               else
               -- Colorbar will be on the backround
               bg_green_dly;

-----------
-- Blue
-----------

  vga_blue <=  -- Mouse_cursor_display is on the top of others
               mouse_cursor_blue_dly when enable_mouse_display_dly = '1'
               else
               -- Overlay display is black 
               x"0" when overlay_en_dly = '1'
               else
               -- logo display
               logo_blue_dly when h_cntr_reg_dly > LOGO_LEFT and h_cntr_reg_dly < LOGO_RIGHT 
                              and v_cntr_reg_dly < LOGO_BOTTOM and v_cntr_reg_dly > LOGO_TOP
               else
               -- Temperature display
               xadc_temp_blue_dly when h_cntr_reg_dly > XADC_TEMP_LEFT and h_cntr_reg_dly < XADC_TEMP_RIGHT 
                                   and v_cntr_reg_dly > XADC_TEMP_TOP and v_cntr_reg_dly < XADC_TEMP_BOTTOM
               else
               adt7420_temp_blue_dly when h_cntr_reg_dly > ADT7420_TEMP_LEFT and h_cntr_reg_dly < ADT7420_TEMP_RIGHT 
                                      and v_cntr_reg_dly > ADT7420_TEMP_TOP and v_cntr_reg_dly < ADT7420_TEMP_BOTTOM
               else
               adxl362_temp_blue_dly when h_cntr_reg_dly > ADXL362_TEMP_LEFT and h_cntr_reg_dly < ADXL362_TEMP_RIGHT 
                                      and v_cntr_reg_dly > ADXL362_TEMP_TOP and v_cntr_reg_dly < ADXL362_TEMP_BOTTOM
               else
               -- TBOX display
     			   tbox_blue when h_cntr_reg_dly > TBOX_LEFT and h_cntr_reg_dly < TBOX_RIGHT 
                          and v_cntr_reg_dly < TBOX_BOTTOM and v_cntr_reg_dly > TBOX_TOP
               else
               --  RGB Led display
               rgb_b_red_col_dly when (h_cntr_reg_dly > RGB_R_COL_LEFT and h_cntr_reg_dly < RGB_R_COL_RIGHT) 
                                  and 
                                     (
                                      (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                      or
                                      (v_cntr_reg_dly > RGB2_COL_TOP and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                     )
               else
               rgb_b_green_col_dly when (h_cntr_reg_dly > RGB_G_COL_LEFT and h_cntr_reg_dly < RGB_G_COL_RIGHT) 
                                    and 
                                      (
                                      (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                    or
                                      (v_cntr_reg_dly > RGB2_COL_TOP - 1 and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                      )
               else
               rgb_b_blue_col_dly when (h_cntr_reg_dly > RGB_B_COL_LEFT and h_cntr_reg_dly < RGB_B_COL_RIGHT) 
                                    and 
                                       (
                                        (v_cntr_reg_dly > RGB1_COL_TOP and v_cntr_reg_dly < RGB1_COL_BOTTOM)
                                        or
                                        (v_cntr_reg_dly > RGB2_COL_TOP and v_cntr_reg_dly < RGB2_COL_BOTTOM)
                                       )
               else
               -- LBOX display
               lbox_blue when h_cntr_reg_dly > LBOX_LEFT and h_cntr_reg_dly < LBOX_RIGHT 
                          and v_cntr_reg_dly < LBOX_BOTTOM and v_cntr_reg_dly > LBOX_TOP
               else
               -- Microphone data display
               mic_blue_dly when h_cntr_reg_dly > MIC_LEFT and h_cntr_reg_dly < MIC_RIGHT 
                             and v_cntr_reg_dly > MIC_TOP and v_cntr_reg_dly < MIC_BOTTOM
               else
               -- Accelerometer display
               acl_blue_dly when h_cntr_reg_dly > ACL_LEFT and h_cntr_reg_dly < ACL_RIGHT 
                             and v_cntr_reg_dly > ACL_TOP and v_cntr_reg_dly < ACL_BOTTOM
               else
               -- Colorbar will be on the backround
               bg_blue_dly;
                

------------------------------------------------------------
-- Turn Off VGA RBG Signals if outside of the active screen
-- Make a 4-bit AND logic with the R, G and B signals
------------------------------------------------------------
 vga_red_cmb <= (active & active & active & active) and vga_red;
 vga_green_cmb <= (active & active & active & active) and vga_green;
 vga_blue_cmb <= (active & active & active & active) and vga_blue;
 

 -- Register Outputs
  process (pxl_clk)
  begin
    if (rising_edge(pxl_clk)) then

      v_sync_reg_dly <= v_sync_reg;
      h_sync_reg_dly <= h_sync_reg;
      vga_red_reg    <= vga_red_cmb;
      vga_green_reg  <= vga_green_cmb;
      vga_blue_reg   <= vga_blue_cmb;      
    end if;
  end process;

  -- Assign outputs
  VGA_HS_O     <= h_sync_reg_dly;
  VGA_VS_O     <= v_sync_reg_dly;
  VGA_RED_O    <= vga_red_reg;
  VGA_GREEN_O  <= vga_green_reg;
  VGA_BLUE_O   <= vga_blue_reg;

end Behavioral;
