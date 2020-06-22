----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Albert Fazakas
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    15:00:45 03/04/2014 
-- Design Name: 
-- Module Name:    AccelerometerCtl - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--       This is the main module for the the Nexys4 onboard ADXL362 accelerometer.
--    The module consists of two components, AXDX362Ctrl and AccelArithmetics. The first one
--    configures the ADXL362 accelerometer and continuously reads X, Y, Z acceleration data and
--    temperature data in 12-bit two's complement format. 
--       The data read is sent to the AccelArithmetics module that formats X and Y acceleration 
--    data to be displayed on the VGA screen in a 512 X 512 pixel area. Therefore the X and Y 
--    acceleration data will be scaled and limited to -1g: 0, 0g: 255, 1g: 511.
--       The AccelArithmetics module also determines the acceleration  magnitude using the
--    SQRT (X^2 + Y^2 + Z^2) formula. The magnitude value is also displayed on the VGA screen.
--    To perform SQRT calculation a Logicore component is used.
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

entity AccelerometerCtl is
generic 
(
   SYSCLK_FREQUENCY_HZ : integer := 100000000;
   SCLK_FREQUENCY_HZ   : integer := 1000000;
   NUM_READS_AVG       : integer := 16;
   UPDATE_FREQUENCY_HZ : integer := 100
);
port
(
 SYSCLK     : in STD_LOGIC; -- System Clock
 RESET      : in STD_LOGIC;

 -- Spi interface Signals
 SCLK       : out STD_LOGIC;
 MOSI       : out STD_LOGIC;
 MISO       : in STD_LOGIC;
 SS         : out STD_LOGIC;

-- Accelerometer data signals
 ACCEL_X_OUT    : out STD_LOGIC_VECTOR (8 downto 0);
 ACCEL_Y_OUT    : out STD_LOGIC_VECTOR (8 downto 0);
 ACCEL_MAG_OUT  : out STD_LOGIC_VECTOR (11 downto 0);
 ACCEL_TMP_OUT  : out STD_LOGIC_VECTOR (11 downto 0)

);
end AccelerometerCtl;

architecture Behavioral of AccelerometerCtl is

component ADXL362Ctrl
generic 
(
   SYSCLK_FREQUENCY_HZ : integer := 100000000;
   SCLK_FREQUENCY_HZ   : integer := 1000000;
   NUM_READS_AVG       : integer := 16;
   UPDATE_FREQUENCY_HZ : integer := 1000
);
port
(
 SYSCLK     : in STD_LOGIC; -- System Clock
 RESET      : in STD_LOGIC;
 
 -- Accelerometer data signals
 ACCEL_X    : out STD_LOGIC_VECTOR (11 downto 0);
 ACCEL_Y    : out STD_LOGIC_VECTOR (11 downto 0);
 ACCEL_Z    : out STD_LOGIC_VECTOR (11 downto 0);
 ACCEL_TMP  : out STD_LOGIC_VECTOR (11 downto 0);
 Data_Ready : out STD_LOGIC;
 
 --SPI Interface Signals
 SCLK       : out STD_LOGIC;
 MOSI       : out STD_LOGIC;
 MISO       : in STD_LOGIC;
 SS         : out STD_LOGIC

);
end component;

component AccelArithmetics
generic
(
   SYSCLK_FREQUENCY_HZ : integer := 100000000;
   ACC_X_Y_MAX         : STD_LOGIC_VECTOR (9 downto 0) := "01" & X"FF"; -- 511 pixels, corresponding to +1g
   ACC_X_Y_MIN         : STD_LOGIC_VECTOR (9 downto 0) := (others => '0') -- corresponding to -1g
);
port
(
 SYSCLK     : in STD_LOGIC; -- System Clock
 RESET      : in STD_LOGIC;
 
 -- Accelerometer data input signals
 ACCEL_X_IN    : in STD_LOGIC_VECTOR (11 downto 0);
 ACCEL_Y_IN    : in STD_LOGIC_VECTOR (11 downto 0);
 ACCEL_Z_IN    : in STD_LOGIC_VECTOR (11 downto 0);
 Data_Ready    : in STD_LOGIC;

 -- Accelerometer data output signals to be sent to the VGA controller
 
 ACCEL_X_OUT    : out STD_LOGIC_VECTOR (8 downto 0);
 ACCEL_Y_OUT    : out STD_LOGIC_VECTOR (8 downto 0);
 ACCEL_MAG_OUT  : out STD_LOGIC_VECTOR (11 downto 0)
);
end component;

-- Self-blocking reset counter constants
constant ACC_RESET_PERIOD_US : integer := 10;
constant ACC_RESET_IDLE_CLOCKS   : integer := ((ACC_RESET_PERIOD_US*1000)/(1000000000/SYSCLK_FREQUENCY_HZ));

signal  ACCEL_X    : STD_LOGIC_VECTOR (11 downto 0);
signal  ACCEL_Y    : STD_LOGIC_VECTOR (11 downto 0);
signal  ACCEL_Z    : STD_LOGIC_VECTOR (11 downto 0);

signal Data_Ready : STD_LOGIC;

-- Self-blocking reset counter
signal cnt_acc_reset : integer range 0 to (ACC_RESET_IDLE_CLOCKS - 1):= 0;
signal RESET_INT: std_logic;


begin


-- Create the self-blocking reset counter
COUNT_RESET: process(SYSCLK, cnt_acc_reset, RESET)
begin
   if SYSCLK'EVENT and SYSCLK = '1' then
      if (RESET = '1') then
         cnt_acc_reset <= 0;
         RESET_INT <= '1';
      elsif cnt_acc_reset = (ACC_RESET_IDLE_CLOCKS - 1) then
         cnt_acc_reset <= (ACC_RESET_IDLE_CLOCKS - 1);
         RESET_INT <= '0';
      else
         cnt_acc_reset <= cnt_acc_reset + 1;
         RESET_INT <= '1';
      end if;
   end if;
end process COUNT_RESET;


ADXL_Control: ADXL362Ctrl
generic map
(
   SYSCLK_FREQUENCY_HZ  => SYSCLK_FREQUENCY_HZ,
   SCLK_FREQUENCY_HZ    => SCLK_FREQUENCY_HZ,
   NUM_READS_AVG        => NUM_READS_AVG,   
   UPDATE_FREQUENCY_HZ  => UPDATE_FREQUENCY_HZ
)
port map
(
 SYSCLK     => SYSCLK, 
 RESET      => RESET_INT, 
 
 -- Accelerometer data signals
 ACCEL_X    => ACCEL_X,
 ACCEL_Y    => ACCEL_Y, 
 ACCEL_Z    => ACCEL_Z,
 ACCEL_TMP  => ACCEL_TMP_OUT, 
 Data_Ready => Data_Ready, 
 
 --SPI Interface Signals
 SCLK       => SCLK, 
 MOSI       => MOSI,
 MISO       => MISO, 
 SS         => SS
);

Accel_Calculation: AccelArithmetics
GENERIC MAP
(
   SYSCLK_FREQUENCY_HZ  => 100000000,
   ACC_X_Y_MAX          => "01" & X"FF", -- 511 pixels, corresponding to +1g
   ACC_X_Y_MIN          => (others => '0') -- corresponding to -1g
)
PORT MAP
(
 SYSCLK  => SYSCLK, 
 RESET   => RESET_INT,
 
 -- Accelerometer data input signals
 ACCEL_X_IN => ACCEL_X,
 ACCEL_Y_IN => ACCEL_Y,
 ACCEL_Z_IN => ACCEL_Z,
 Data_Ready => Data_Ready,

 -- Accelerometer data output signals to be sent to the VGA display
 ACCEL_X_OUT => ACCEL_X_OUT,
 ACCEL_Y_OUT => ACCEL_Y_OUT,
 ACCEL_MAG_OUT => ACCEL_MAG_OUT
);

end Behavioral;

