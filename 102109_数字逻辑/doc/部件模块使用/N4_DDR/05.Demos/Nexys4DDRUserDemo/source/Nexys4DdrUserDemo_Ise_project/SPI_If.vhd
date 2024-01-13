----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Albert Fazakas
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    17:55:33 02/17/2014 
-- Design Name: 
-- Module Name:    SPI_If - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--       This module represents an SPI controller. The controller transfers 8 bits of data, MSB first.
--    Data is read on the positive edge of SCLK and also stable on the positive edge of SCLK.
--    When there is no data transfer, SCLK = 0, therefore CPOL = 0, CPHA = 0.
--       Data transfer starts by activating the "Start" signal, and, when data transfer is done, the 
--    "Done" signal is activated for one SYSCLK period.
--
--       The module is also capable to transfer multiple bytes, if the "HOLD_SS" signal is active.
--    In this case SS is not raised between byte transfers and a new transfer can begin 
--    if the "Start" signal is activated. This feature is used in conjunction with the ADXL 362 accelerometer
--    or any device which needs multiple-byte transfers for sending commands and/or reading data
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

entity SPI_If is
generic 
(
   SYSCLK_FREQUENCY_HZ : integer:= 100000000;
   SCLK_FREQUENCY_HZ : integer:= 1000000
);
port
(
 SYSCLK     : in STD_LOGIC; -- System Clock
 RESET      : in STD_LOGIC;
 Din        : in STD_LOGIC_VECTOR (7 downto 0); -- Data to be transmitted
 Dout       : out STD_LOGIC_VECTOR (7 downto 0); -- Data received;
 Start      : in STD_LOGIC; -- used to start the transmission
 Done       : out STD_LOGIC; -- Signaling that transmission ended
 HOLD_SS   : in STD_LOGIC; -- Signal that forces SS low in the case of multiple byte 
                           -- transmit/receive mode
 --SPI Interface Signals
 SCLK       : out STD_LOGIC;
 MOSI       : out STD_LOGIC;
 MISO       : in STD_LOGIC;
 SS         : out STD_LOGIC
);
end SPI_If;

architecture Behavioral of SPI_If is

-- Determine the division rate in order to create the 2X SCLK tick
constant DIV_RATE : integer := ((SYSCLK_FREQUENCY_HZ / ( 2 * SCLK_FREQUENCY_HZ)) - 1);

-- To generate the 2X SCLK tick, then SCLK
signal SCLK_2X_DIV      : integer range 0 to DIV_RATE := 0;
signal SCLK_2X_TICK     : STD_LOGIC := '0';
-- Internal SCLK signal
signal SCLK_INT         : STD_LOGIC := '0';

-- Enable Output Signals
signal EN_SCLK    : STD_LOGIC := '0';
signal EN_SS      : STD_LOGIC := '0';

-- Control Signals 
signal EN_SHIFT      : STD_LOGIC := '0';  -- Enable shifting the MOSI_REG and MISO_REG registers
signal EN_LOAD_DOUT  : STD_LOGIC :='0';   -- Enable loading the Dout register
                                          -- from the shift register for MISO 
signal EN_LOAD_DIN   : STD_LOGIC := '0'; -- Load the MOSI shift register 

signal SHIFT_TICK_IN     : STD_LOGIC; -- the moment at which the shifting is made, 
signal SHIFT_TICK_OUT    : STD_LOGIC; -- i.e. at the SCLK frequency

-- State machine internal condition signals
signal Start_Shifting : STD_LOGIC := '0';
signal Shifting_Done  : STD_LOGIC := '0';

--Counter for number of bits sent/received
signal CntBits: integer range 0 to 7 := 0;
signal Reset_Counters: STD_LOGIC; -- to reset all the counters, when in the Idle State

-- Shift In and Shift Out Registers
signal MOSI_REG   : STD_LOGIC_VECTOR (7 downto 0) := X"00";
signal MISO_REG   : STD_LOGIC_VECTOR (7 downto 0) := X"00";

-- Pipe Done signal to ensure that data is stable at the output
signal DONE_1  : STD_LOGIC := '0';

-- Define Control Signals and States. From MSB: 7:EN_LOAD_DIN, 6:EN_SHIFT, 5:Reset_Counters, 
--                                          4:EN_SCLK, 3:EN_SS, 2:EN_LOAD_DOUT, 1:STC(1), 0:STC(0)
constant stIdle    : STD_LOGIC_VECTOR (7 downto 0) := "10100000";
constant stPrepare : STD_LOGIC_VECTOR (7 downto 0) := "00001001";
constant stShift   : STD_LOGIC_VECTOR (7 downto 0) := "01011011";
constant stDone    : STD_LOGIC_VECTOR (7 downto 0) := "00001110";

--State Machine Signal Definitions
signal StC, StN   :  STD_LOGIC_VECTOR (7 downto 0) := stIdle;

--Force User Encoding for the State Machine
attribute FSM_ENCODING : string;
attribute FSM_ENCODING of StC: signal is "USER";

begin

-- Assign the control signals first
EN_LOAD_DIN    <= StC(7);
EN_SHIFT       <= StC(6);
Reset_Counters <= StC(5);
EN_SCLK        <= StC(4);
EN_SS          <= StC(3);
EN_LOAD_DOUT   <= StC(2);

-- Assign the outputs
SS <= '0' when RESET = '0' and (HOLD_SS = '1' or EN_SS = '1') else '1';
MOSI  <= MOSI_REG(7);
SCLK <= SCLK_INT when EN_SCLK = '1' else '0';

--Assign the outputs: Register Dout
Load_Output: process (SYSCLK, EN_LOAD_DOUT, MISO_REG)
begin
   if RISING_EDGE (SYSCLK) then
      if EN_LOAD_DOUT = '1' then
         Dout <= MISO_REG;
      end if;
   end if;
end process Load_Output;

-- Set the Done signal, delayed with one clock period
-- after data is assigned
Set_Done: process (SYSCLK, EN_LOAD_DOUT, DONE_1)
begin
   if RISING_EDGE (SYSCLK) then
      DONE_1 <= EN_LOAD_DOUT;
      Done   <= DONE_1;
   end if;
end process Set_Done;

-- Divider to generate the 2X SCLK tick
Div_2X_SCLK: process (SYSCLK, Reset_Counters, SCLK_2X_DIV)
begin
   if RISING_EDGE (SYSCLK) then
      if Reset_Counters = '1' 
         or SCLK_2X_DIV = DIV_RATE then
            SCLK_2X_DIV <= 0;
      else
            SCLK_2X_DIV <= SCLK_2X_DIV + 1;
      end if;
   end if;
end process Div_2X_SCLK;

-- SCLK_2X_TICK will be active at both the rising and falling edges
-- of SCLK_INT
SCLK_2X_TICK <= '1' when SCLK_2X_DIV = DIV_RATE else '0';

--Generate SCLK_INT;
Gen_SCLK_INT: process (SYSCLK, Reset_Counters, SCLK_2X_TICK, SCLK_INT)
begin
   if RISING_EDGE (SYSCLK) then
      if Reset_Counters = '1' then
         SCLK_INT <= '0';
      elsif SCLK_2X_TICK = '1' then
         SCLK_INT <= not SCLK_INT;
      end if;
   end if;
end process Gen_SCLK_INT;

-- SHIFT_TICK_IN will be active at the rising edge of SCLK
-- At that moment MOSI will be read
SHIFT_TICK_IN  <= '1' when EN_SHIFT = '1' and SCLK_INT = '0' and SCLK_2X_TICK = '1' else '0';

-- SHIFT_TICK_OUT will be active at the falling edge of SCLK
-- At that moment the next bit will be shifted out
SHIFT_TICK_OUT <= '1' when EN_SHIFT = '1' and SCLK_INT = '1' and SCLK_2X_TICK = '1' else '0';

-- Create the shift in register, MSB First, so shift left
SHIFT_IN: process (SYSCLK, SHIFT_TICK_IN, MISO_REG)
begin
   if RISING_EDGE (SYSCLK) then
      if SHIFT_TICK_IN = '1' then
         MISO_REG (7 downto 0) <= MISO_REG (6 downto 0) & MISO;
      end if;
   end if;
end process SHIFT_IN;

-- Create the shift out register, MSB out first, so shift left
-- MOSI_REG is constantly loaded with Din when in idle state;
SHIFT_OUT: process (SYSCLK, EN_LOAD_DIN, Din, SHIFT_TICK_OUT, MOSI_REG)
begin
   if RISING_EDGE (SYSCLK) then
      if EN_LOAD_DIN = '1' then 
         MOSI_REG <= Din;
      elsif SHIFT_TICK_OUT = '1' then
         MOSI_REG (7 downto 0) <= MOSI_REG (6 downto 0) & '0';
      end if;
   end if;
end process SHIFT_OUT;

-- CntBits will be incremented at the falling edge of SCLK
-- i.e. when SHIFT_TICK_OUT is active
Count_Bits: process (SYSCLK, Reset_Counters, SHIFT_TICK_OUT, CntBits)
begin
   if RISING_EDGE (SYSCLK) then
      if Reset_Counters = '1' then
         CntBits <= 0;
      elsif SHIFT_TICK_OUT = '1' then
         if CntBits = 7 then 
            CntBits <= 0;
         else
            CntBits <= CntBits + 1;
         end if;
      end if;
   end if;
end process Count_Bits; 

-- Assign the State machine internal condition signals

-- Start Shifting in the stPrepare state, after 
-- either a falling edge of SCLK_INT comes in a single byte transfer mode
-- or immediately after a Start command received, in multiple byte transfer mode
Start_Shifting <= '1' when StC = stPrepare and (HOLD_SS = '1' or (SCLK_INT = '1' and SCLK_2X_TICK = '1'))
                      else '0';

-- Shifting ends when 8 bits has been transferred, 
-- at the falling edge of SCLK_INT
Shifting_Done <= '1' when StC = stShift and CntBits = 7 and SCLK_INT = '1' and SCLK_2X_TICK = '1' else '0';

-- State machine register process
Reg_Statem: process (SYSCLK, RESET, StN)
begin
   if RISING_EDGE (SYSCLK) then
      if RESET = '1' then
         StC <= stIdle;
      else
         StC <= StN;
      end if;
   end if;
end process Reg_Statem;

-- State machine transition process
Cmb_Statem: process (StC, Start, Start_Shifting, Shifting_Done)
begin
   StN <= StC; -- default: stay in the current state if no other
               -- condition for transition occurs
   case StC is
   when stIdle => if Start = '1' then StN <= stPrepare; end if;
   when stPrepare => if Start_Shifting = '1' then StN <= stShift; end if;
   when stShift => if Shifting_Done = '1' then StN <= stDone; end if;
   when stDone => StN <= stIdle;
   when others => StN <= stIdle;
   end case;
end process Cmb_Statem;


end Behavioral;

