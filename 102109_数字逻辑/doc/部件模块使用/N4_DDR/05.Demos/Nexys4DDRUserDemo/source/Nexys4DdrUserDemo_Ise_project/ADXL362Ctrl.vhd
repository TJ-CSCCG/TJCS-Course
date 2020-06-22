----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Albert Fazakas
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    16:48:39 02/20/2014 
-- Design Name: 
-- Module Name:    ADXL362Ctrl - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--       This module represents the controller for the Nexys4 onboard ADXL362 Accelerometer device.
--    The module uses the SPI Interface component to communicate with the ADXL362.
--    At initialization time, the module resets the ADXL362, then configures its internal registers.
--       After configuring its internal registers, the acceleration will be read on the three axes followed 
--    by the temperature data: A set of 8 data bytes are read: XDATA_L, XDATA_H, YDATA_L, YDATA_H, ZDATA_L,
--    ZDATA_H, TEMP_L and TEMP_H, see the ADXL362 datasheet for details.
--       Reading is done continuously and an average is made from a number of reads. The number of reads 
--    for which average is made should be a power of two and is determined by the NUM_READS_AVG parameter,
--    by default 16. The UPDATE_FREQUENCY_HZ parameter sets a counter to a period of 1/UPDATE_FREQUENCY_HZ.
--    The state machine will wait for this period of time before starting a number of NUM_READS_AVG times 
--    data read procedure.
--       Before reading a set of data, the state machine reads and checks the status register to see when
--    new data is available at the ADXL362. Therefore the real sample time depends on the ADXL362 sample 
--    frequency, set in the Filter Control Register, address 0x2C, in this project set by default to 200Hz.
--
--       The module consists of three state machines, named by the signals holding the states:
--       - SPI Send/Receive Control State Machine: StC_Spi_SendRec. This state machine creates a handshake transaction
--    with the SPI Interface component, using the Start and Done signals to:
--             - Send the number of bytes specified by the Cnt_Bytes_Sent counter (3 when configuring an ADXL362 internal
--          register, 2 when sending a Read Command). The bytes to be sent through the SPI interface are taken from the 
--          command register, Cmd_Reg.
--             - Receive the number of bytes specified by the Cnt_Bytes_Rec counter (8 when reading acceleration and 
--          temperature data, 1 when reading the status register), when reading is required (SPI_RnW = 1).
--          The acceleration and temperature data is stored in the data register, Data_Reg
--
--       - SPI Transaction State machine, StC_Spi_Trans. This state machine controls the previously described SPI 
--    Send/Receive Control State Machine, using handshake with the the StartSpiSendRec (write) and SPI_SendRec_Done (read)
--    signals:
--            - Prepares and loads the command register, Cmd_Reg with the appropiate command string (configure a specific 
--          register, read data or read status)
--            - Loads Cnt_Bytes_Sent and Cnt_Bytes_Rec with the number of bytes to be sent and/or received
--            - Activates StartSpiSendRec to start the SPI Send/Receive Control State Machine and wait for its answer 
--          by reading the SPI_SendRec_Done signal
--
--          Note that between each SPI transaction (Register write or Register read) the SS signal has to be deactivated
--          for at least 10nS before a new command is issued
--
--       - ADXL 362 Control State Machine, StC_Adxl_Ctrl. This state machine controls the previously described SPI 
--    Transaction State machine, also by using handshake with the StartSpiTr (write) and SPI_Trans_Done (read) signals:
--            1. First, Cmd_Reg will be loaded with the reset command from the Cmd_Reg_Data ROM and the state machine starts 
--          the SPI Transaction State Machine to send the reset command to the ADXL362 accelerometer
--            2. The state machine waits for a period of time and then sends the remaining configuration register data from 
--          Cmd_Reg_Data to the ADXL362 accelerometer
--            3. After configuring ADXL362, the state machine waits for a period of time equal to 1/UPDATE_FREQUENCY_HZ 
--          before starts reading
--            4. After the period of time elapsed the state machine reads the status register and, when there is new data available, reads the 
--          reads the X, Y and Z acceleration data, followed by temperature data.
--               A number of reads equal to NUM_READS_AVG is performed, i.e. Step 4 is repeated NUM_READS_AVG times.
--            5. The data read is averaged in the ACCEL_X_SUM, ACCEL_Y_SUM, ACCEL_Z_SUM and ACCEL_TMP_SUM registers. The
--          NUM_READS_AVG is power of 2 in order to make averaging easier by removing the least significant bits. After 
--          a number of reads equal to NUM_READS_AVG is done, the state machine updates the output data, ACCEL_X, ACCEL_Y,
--          ACCEL_Z and ACCEL_TMP, stored in 12-bit two's complement format and signals to the output by activating for one
--          clock period the Data_Ready signal.
--               After that the state machine proceeds to Step 3, in an infinite loop. The state machine restarts from Step 1 only
--          when the FPGA is reconfigured or the Reset signal is activated.
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
use IEEE.NUMERIC_STD.ALL;
use IEEE.std_logic_signed.all;
use IEEE.math_real.all;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity ADXL362Ctrl is
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
end ADXL362Ctrl;

architecture Behavioral of ADXL362Ctrl is

-- SPI Interface component declaration
component SPI_If is
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
end component;

--**************************************
-- Constant Definitions
--**************************************

-- To create the update frequency counter
constant UPDATE_DIV_RATE : integer := (SYSCLK_FREQUENCY_HZ / UPDATE_FREQUENCY_HZ);
constant SYS_CLK_PERIOD_PS : integer := ((1000000000 / SYSCLK_FREQUENCY_HZ) * 1000);

--ADXL 362 Read and Write Command
constant READ_CMD : STD_LOGIC_VECTOR (7 downto 0) := X"0B";
constant WRITE_CMD : STD_LOGIC_VECTOR (7 downto 0) := X"0A";
-- Data read will be always performed starting from Address X"0E",
-- representing XACC_H. A total number of 8 bytes will be read.
constant READ_STARTING_ADDR : STD_LOGIC_VECTOR (7 downto 0):= X"0E";
-- Status Register Read will be used to check when new data is available (Bit 0 is 1)
constant STATUS_REG_ADDR : STD_LOGIC_VECTOR (7 downto 0):= X"0B";

-- Number of bytes to write when configuring registers
constant NUMBYTES_CMD_CONFIG_REG : integer := 3;
-- Number of bytes to write when reading registers
constant NUMBYTES_CMD_READ : integer := 2;

-- Number of bytes to read when reading data from ADXL362
constant NUMBYTES_READ_DATA : integer := 8;
-- Number of bytes to read when reading status register from ADXL362
constant NUMBYTES_READ_STATUS : integer := 1;

-- number of command vectors to send, one command vector 
-- represents ADXL362 register address followed by command byte, 
-- i.e. one command vector will mean two bytes
constant NUM_COMMAND_VEC : integer := 4;

-- Number of reads to be performed for which average is calculated, default is 16
constant NUM_READS : natural := NUM_READS_AVG;
-- Number of extra bits when creating the average of the reads
constant NUM_READS_BITS : natural := natural(ceil(log(real(NUM_READS), 2.0)));

-- after each SPI transaction, SS needs to be inactive for at least 10ns before a new command is issued
constant SS_INACTIVE_PERIOD_NS : integer := 10000;
constant SS_INACTIVE_CLOCKS   : integer := (SS_INACTIVE_PERIOD_NS/(SYS_CLK_PERIOD_PS/1000));

-- To specify encoding of the state machines
attribute FSM_ENCODING : string;

--SPI Interface Control Signals
signal Start      :  STD_LOGIC; -- Signal controlling the SPI interface, controlled by the SPI Send/Receive State Machine
signal Done       :  STD_LOGIC; -- Signaling that transmission ended, coming from the SPI interface
signal HOLD_SS    :  STD_LOGIC; -- Signal that forces SS low in the case of multiple byte
                           -- transmit/receive mode, controlled by the SPI Transaction State Machine
                           
-- Create the initialization vector, i.e. 
-- the register data to be written to initialize ADXL362
type rom_type is array (0 to ((2* NUM_COMMAND_VEC)-1)) of STD_LOGIC_VECTOR (7 downto 0);

constant Cmd_Reg_Data : rom_type := ( X"1F", X"52", -- Soft Reset Register Address and Reset Command
                                      X"1F", X"00", -- Soft Reset Register Address, clear Command
                                      X"2D", X"02", -- Power Control Register, Enable Measure Command
                                      X"2C", X"14"  -- Filter Control Register, 2g range, 1/4 Bandwidth, 200HZ Output Data Rate                                      
                                     );

--address the reg_data ROM
signal Cmd_Reg_Data_Addr: integer range 0 to (NUM_COMMAND_VEC - 1) := 0;
-- Enable Incrementing Cmd_Reg_Data_Addr, controlled by the ADXL Control State machine
signal EN_Advance_Cmd_Reg_Addr:  STD_LOGIC := '0';
-- will enable incrementing by 2 Cmd_Reg_Data_Addr
signal Advance_Cmd_Reg_Addr:  STD_LOGIC := '0';
-- signal that shows that all of the addresses were read
signal Cmd_Reg_Addr_Done : STD_LOGIC := '0';

-- SPI Transfer Send Data signals. Writing Commands will be always a 3-byte transfer,
-- therefore commands will be temporarry stored in a 3X8 shift register.
type command_reg_type is array (0 to 2) of STD_LOGIC_VECTOR (7 downto 0);
signal Cmd_Reg: command_reg_type := (X"00", X"00", WRITE_CMD);

-- command_reg control signals
signal Load_Cmd_Reg : STD_LOGIC := '0';  -- Controlled by the SPI Transaction State Machine, 
                                         -- the command register is load with the appropiate command
signal Shift_Cmd_Reg : STD_LOGIC := '0'; -- Controlled by the SPI Send/Receive State Machine,
                                         -- advance to the next command when a byte was sent
                                            
-- to count the bytes to be sent
-- Cnt_Bytes_Sent will decrement at the same time when
-- cmd_reg is shifted, therefore its control signal is the same as
-- for Cmd_Reg: EN_Shift_Cmd_Reg
signal Cnt_Bytes_Sent : integer range 0 to 3 :=0; 
-- Load Cnt_Bytes_Sent with the number of bytes to be sent
-- according to the command to be sent
signal Load_Cnt_Bytes_Sent : STD_LOGIC := '0'; -- controlled by the SPI Transaction State Machine

signal Reset_Cnt_Bytes : STD_LOGIC := '0'; -- Controlled by the Main State Machine
                                           -- will reset both the sent and received byte counter

-- SPI Transfer Receive Data signals. Reading data will be a 8-byte transfer:
-- XACC_H, XACC_L, YACC_H, YACC_L, ZACC_H, ZACC_L, TEMP_H, TEMP_L
-- therefore an 8X8 shift register is created.
type data_reg_type is array (0 to (NUMBYTES_READ_DATA - 1)) of STD_LOGIC_VECTOR (7 downto 0);
signal Data_Reg: data_reg_type := (others => X"00");

-- data_reg control signals:
signal EN_Shift_Data_Reg : STD_LOGIC := '0'; -- Controlled by the SPI Send/Receive State Machine
                                             -- Shift when a new byte was received
-- Data Reg will be shifted when a new byte comes, i.e. Shift_Data_Reg <= EN_Shift_Data_Reg AND Done;
signal Shift_Data_Reg : STD_LOGIC := '0';                                             

-- to count the bytes to be received
-- Cnt_Bytes_Rec will decrement at the same time when Data_Reg is shifted, 
-- therefore its control signal is the same as for Data_Reg: Shift_Data_Reg
signal Cnt_Bytes_Rec : integer range 0 to NUMBYTES_READ_DATA - 1 := 0;
-- Load Cnt_Bytes_Rec with the number of bytes to be received, controlled by the SPI Transaction State Machine
signal Load_Cnt_Bytes_Rec : STD_LOGIC := '0'; 

-- SPI Data to Send and Data Received registers
-- Data to send register will be loaded together with shifting 
-- a new byte in Cmd_Reg, i.e. its control signal is Shift_Cmd_Reg
-- Data Received Register will be read when shifting Data_Reg
signal D_Send   : STD_LOGIC_VECTOR (7 downto 0) := X"00";
signal D_Rec   : STD_LOGIC_VECTOR (7 downto 0) := X"00";

-- SPI Send/Receive State Machine internal condition signals
signal StartSpiSendRec  : STD_LOGIC := '0'; -- Start SPI transfer, controlled by the SPI Transaction State Machine
signal SPI_RnW  : STD_LOGIC := '0'; -- Write 3 bytes or write 2 bytes followed by read 1 byte or 8 bytes
signal SPI_WR_Done   : STD_LOGIC := '0'; -- Active when the write transfer is done, i.e. 2 or 3 bytes were written
signal SPI_RD_Done   : STD_LOGIC := '0'; -- Active when the read transfer is done, i.e. 8 bytes were read

-- SPI Send/Receive State Machine status signals, used by the SPI Transaction State Machine
signal SPI_SendRec_Done  : STD_LOGIC := '0';

-- Define Control Signals, Status signals and States for the SPI Send/Receive State Machine
-- From MSB: 6:Shift_Cmd_Reg, 5:EN_Shift_Data_Reg, 4:SPI_SendRec_Done, 3:Start, 2:STC(2), 1:STC(1), 0:STC(0)
                                                          -- bit  6543210
constant stSpiSendRecIdle     : STD_LOGIC_VECTOR (6 downto 0) := "0000000"; -- Idle state, wait for StartSpiSendRec
constant stSpiPrepareCmd      : STD_LOGIC_VECTOR (6 downto 0) := "1000001"; -- Load D_Send with the next byte and shift the command register
constant stSpiSendStartW      : STD_LOGIC_VECTOR (6 downto 0) := "0001011"; -- Send the Start command to the SPI interface
constant stSpiWaitOnDoneW     : STD_LOGIC_VECTOR (6 downto 0) := "0000111"; -- Wait until Done comes
constant stSpiSendStartR      : STD_LOGIC_VECTOR (6 downto 0) := "0001110"; -- Send Start command again to the SPI interface if read was requested
constant stSpiWaitOnDoneR     : STD_LOGIC_VECTOR (6 downto 0) := "0100100"; -- Wait until Done comes
constant stSpiSendRecDone     : STD_LOGIC_VECTOR (6 downto 0) := "0010101"; -- Done state, return to Idle

--State Machine Signal Definitions
signal StC_Spi_SendRec, StN_Spi_SendRec   :  STD_LOGIC_VECTOR (6 downto 0) := stSpiSendRecIdle;
--Force User Encoding for the State Machine
attribute FSM_ENCODING of StC_Spi_SendRec: signal is "USER";

-- Self-blocking counter for SS_INACTIVE_CLOCKS periods while SS is inactive
signal Cnt_SS_Inactive : integer range 0 to (SS_INACTIVE_CLOCKS -1) := 0;
-- controlled by the SPI Transaction State Machine
signal Reset_Cnt_SS_Inactive : STD_LOGIC := '0';
-- Signaling that SS_INACTIVE_PERIOD passed
signal Cnt_SS_Inactive_done : STD_LOGIC := '0';

-- Signaling that SPI Transaction is done
signal SPI_Trans_Done   : STD_LOGIC := '0';

-- SPI Transaction State Machine internal Condition Signals, controlled 
-- by the ADXL 362 Control State Machine
signal StartSpiTr  : STD_LOGIC := '0'; -- Start SPI transaction

-- Define Control Signals, Status signals and States for the SPI Transaction State Machine
-- From MSB: 9:Load_Cmd_Reg, 8:Load_Cnt_Bytes_Sent, 7:Load_Cnt_Bytes_Rec, 6:StartSpiSendRec, 
-- 5:HOLD_SS, 4:Reset_Cnt_SS_Inactive, 3:SPI_Trans_Done, 2:STC(2), 1:STC(1), 0:STC(0)
                                                          -- bit  9876543210
constant stSpiTransIdle       : STD_LOGIC_VECTOR (9 downto 0) := "0000000000"; -- Idle state, wait for StartSpiTr
constant stSpiPrepAndSendCmd  : STD_LOGIC_VECTOR (9 downto 0) := "1111100001"; -- Load Cmd_Reg with the command string and activate StartSpiSendRec
constant stSpiWaitonDoneSR    : STD_LOGIC_VECTOR (9 downto 0) := "0000110011"; -- Wait until SPI_SendRec_Done becomes active
constant stSpiWaitForSsInact  : STD_LOGIC_VECTOR (9 downto 0) := "0000000010"; -- Wait for SS_INACTIVE_PERIOD
constant stSpiTransDone       : STD_LOGIC_VECTOR (9 downto 0) := "0000001110"; -- Done state, return to Idle

--State Machine Signal Definitions
signal StC_Spi_Trans, StN_Spi_Trans : STD_LOGIC_VECTOR (9 downto 0) := stSpiTransIdle;
--Force User Encoding for the State Machine
attribute FSM_ENCODING of StC_Spi_Trans: signal is "USER";

-- Data from the ADXL 362 will be sampled at a period defined by UPDATE_DIV_RATE
-- Divider used to generate the Sample_Rate_Tick, used also for timing
signal Sample_Rate_Div        : integer range 0 to (UPDATE_DIV_RATE - 1) := 0;
signal Reset_Sample_Rate_Div  : STD_LOGIC := '0';
signal Sample_Rate_Tick       : STD_LOGIC := '0';

-- A number of 16 reads will be performed from the ADXL 362, 
-- and their average will be sent as data
signal Cnt_Num_Reads : integer range 0 to (NUM_READS - 1) := 0;
signal CE_Cnt_Num_Reads : STD_LOGIC := '0'; -- enable counting, controlled by the ADXL 362 Control State Machine
signal Reset_Cnt_Num_Reads : STD_LOGIC := '0';
--Signaling that a number of 16 reads were done
signal Cnt_Num_Reads_Done : STD_LOGIC := '0';

-- Summing of incoming data will be stored in these signals
-- These will be used as accumulators, on two's complement
signal ACCEL_X_SUM   : STD_LOGIC_VECTOR ((11 + (NUM_READS_BITS)) downto 0) := (others => '0');
signal ACCEL_Y_SUM   : STD_LOGIC_VECTOR ((11 + (NUM_READS_BITS)) downto 0) := (others => '0');
signal ACCEL_Z_SUM   : STD_LOGIC_VECTOR ((11 + (NUM_READS_BITS)) downto 0) := (others => '0');
signal ACCEL_TMP_SUM : STD_LOGIC_VECTOR ((11 + (NUM_READS_BITS)) downto 0) := (others => '0');

-- Enables Summing the incomming data
signal Enable_Sum : STD_LOGIC := '0';
-- Pipe Data_Ready to have stable data at the output when activates
signal Data_Ready_1 : STD_LOGIC := '0';

-- ADXL 362 Control State Machine - The Main State Machine Internal Condition Signal
signal Adxl_Data_Ready : STD_LOGIC := '0'; -- showing that data is ready, read from the ADXL 362 Status Register
signal Adxl_Conf_Err : STD_LOGIC := '0'; -- showing that a configuration error ocurred, read from the ADXL 362 Status Register

-- Define Control Signals, Status signals and States for the ADXL 362 Control State Machine
-- From MSB: 11:Reset_Cnt_Bytes, 10:EN_Advance_Cmd_Reg_Addr, 9:StartSpiTr, 8:Reset_Cnt_Num_Reads, 
-- 7:CE_Cnt_Num_Reads, 6:Reset_Sample_Rate_Div, 5:Enable_Sum, 4:Data_Ready_1, 3:STC(3), 2:STC(2), 1:STC(1), 0:STC(0)
                                                               --       11
                                                               -- bit   109876543210
constant stAdxlCtrlIdle            : STD_LOGIC_VECTOR (11 downto 0) := "100100000000"; -- Idle state, wait for 10 clock periods before start
constant stAdxlSendResetCmd        : STD_LOGIC_VECTOR (11 downto 0) := "011001000001"; -- Send the Reset Command for ADXL
constant stAdxlWaitResetDone       : STD_LOGIC_VECTOR (11 downto 0) := "010000000011"; -- Wait for some time until ADXL initializes. 
                                                                                               -- The sample rate divider is used for timing
constant stAdxlConf_Remaining      : STD_LOGIC_VECTOR (11 downto 0) := "011001000010"; -- Clear the Reset Register, 
                                                                                               -- then configure the remaining registers
constant stAdxlWaitSampleRateTick  : STD_LOGIC_VECTOR (11 downto 0) := "000100000110"; -- wait until the sample time passes
constant stAdxlRead_Status         : STD_LOGIC_VECTOR (11 downto 0) := "011001000111"; -- Read the status register from ADXL 362
constant stAdxlRead_Data           : STD_LOGIC_VECTOR (11 downto 0) := "011000000101"; -- Read the data from ADXL 362
constant stAdxlFormatandSum        : STD_LOGIC_VECTOR (11 downto 0) := "000010101101"; -- Store and sum the received data 
                                                                                               -- If 16 reads were done, go to the Done state
constant stAdxlRead_Done           : STD_LOGIC_VECTOR (11 downto 0) := "000001011111"; -- Done state, return to stAdxlWaitSampleRateTick

--State Machine Signal Definitions
signal StC_Adxl_Ctrl, StN_Adxl_Ctrl : STD_LOGIC_VECTOR (11 downto 0) := stAdxlCtrlIdle;
--Force User Encoding for the State Machine
attribute FSM_ENCODING of StC_Adxl_Ctrl: signal is "USER";


begin

--Instantiate the SPI interface first
SPI_Interface: SPI_If
generic map 
(
   SYSCLK_FREQUENCY_HZ => SYSCLK_FREQUENCY_HZ,
   SCLK_FREQUENCY_HZ => SCLK_FREQUENCY_HZ
)
port map
(
 SYSCLK => SYSCLK,  
 RESET  => RESET,
 Din    => D_Send,
 Dout   => D_Rec,
 Start  => Start,
 Done   => Done,
 HOLD_SS => HOLD_SS,
 --SPI Interface Signals
 SCLK  => SCLK,
 MOSI  => MOSI,
 MISO  => MISO,
 SS    => SS
);

-- Assign the control and status signals of the SPI Send/Receive State Machine
Shift_Cmd_Reg     <= StC_Spi_SendRec(6); -- Shift Cmd_Reg when New data is loading into D_Send
EN_Shift_Data_Reg <= StC_Spi_SendRec(5); -- Enable shifting the data register from D_Rec, shifting is performed when a new byte comes, i.e Done becomes active
SPI_SendRec_Done  <= StC_Spi_SendRec(4); -- Transfer of the number of bytes is done
Start             <= StC_Spi_SendRec(3); -- Send the Start command to the SPI interface 
                                          --in the stSpiSendStartW (writing) or stSpiSendStartR (Reading) states
-- Load D_Send with the new data to be transmitted
Load_D_Send: process (SYSCLK, RESET, Cmd_Reg, Shift_Cmd_Reg)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if RESET = '1' then
         D_Send <= X"00";
      elsif Shift_Cmd_Reg = '1' then
         D_Send <= Cmd_Reg (2);
      end if;
   end if;
end process Load_D_Send;


-- Assign the control and status signals of the SPI Transaction State Machine
Load_Cmd_Reg          <= StC_Spi_Trans(9); -- Load the Command Register when preparing the command to be sent to ADXL
Load_Cnt_Bytes_Sent   <= StC_Spi_Trans(8); -- Also load the counter of bytes to be sent
Load_Cnt_Bytes_Rec    <= StC_Spi_Trans(7); -- And, in the case of reception, the counter of bytes to be received
StartSpiSendRec       <= StC_Spi_Trans(6); -- Also send the start command to the SPI Send/Receive State Machine
-- Note that the signals above are active at the same time, i.e identical. They will be optimized by the synthesizer

HOLD_SS               <= StC_Spi_Trans(5); -- Each SPI send/receive will be multiple byte transfer, so activate HOLD_SS
Reset_Cnt_SS_Inactive <= StC_Spi_Trans(4); -- Keep the Cnt_SS_Inactive counter reset, until the transfer is done
                                          -- in the next state the state machine will raise SS for a period of SS_INACTIVE_PERIOD_NS
SPI_Trans_Done        <= StC_Spi_Trans(3); -- Signals that the SPI transfer is done



-- Assign the control and status signals of the ADXL 362 Control State Machine
Reset_Cnt_Bytes         <= StC_Adxl_Ctrl(11); -- Reset the counters for bytes to send and receive. These counters are reset
                                              -- at initializing, then reloaded at each new transaction
EN_Advance_Cmd_Reg_Addr <= StC_Adxl_Ctrl(10); -- Advance the address of the command vectors, to load a new command
StartSpiTr              <= StC_Adxl_Ctrl(9);  -- Send the Start command to the SPI Transaction State Machine
Reset_Cnt_Num_Reads     <= StC_Adxl_Ctrl(8);  -- Reset the counter, once at initialization time, then before starting data read,
                                              -- i.e in the stAdxlRead_Status state
CE_Cnt_Num_Reads        <= StC_Adxl_Ctrl(7);  -- Increment Cnt_Num_Reads after a new set of data come i.e. in the stAdxlFormatandSum state
Reset_Sample_Rate_Div   <= StC_Adxl_Ctrl(6);  -- Reset the Sample_Rate_Div counter before entering in the sample period wait state
                                              -- i.e. in the stAdxlConf_Remaining and the stAdxlRead_Done
Enable_Sum              <= StC_Adxl_Ctrl(5);  -- After new data set come, enable summing
Data_Ready_1            <= StC_Adxl_Ctrl(4);  -- To signal external components that new data set is available, coming from 16 reads


-- Load and shift Cmd_Reg according to the active commands
Load_Shift_Cmd_Reg: process (SYSCLK, Cmd_Reg, Cmd_Reg_Data_Addr, StC_Adxl_Ctrl, Load_Cmd_Reg, Shift_Cmd_Reg)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if Load_Cmd_Reg = '1' then -- Load with data
         if       (StC_Adxl_Ctrl = stAdxlSendResetCmd) 
               or (StC_Adxl_Ctrl = stAdxlConf_Remaining) then -- In this case load with command vectors

                  Cmd_Reg(2) <= WRITE_CMD;
                  Cmd_Reg(1) <= Cmd_Reg_Data (2 * Cmd_Reg_Data_Addr);
                  Cmd_Reg(0) <= Cmd_Reg_Data ((2 * Cmd_Reg_Data_Addr) + 1);

         elsif    (StC_Adxl_Ctrl = stAdxlRead_Status) then

                  Cmd_Reg(2) <= READ_CMD;
                  Cmd_Reg(1) <= STATUS_REG_ADDR;
                  Cmd_Reg(0) <= X"00";
         
         elsif (StC_Adxl_Ctrl = stAdxlRead_Data) then -- In this case load with command vectors

                  Cmd_Reg(2) <= READ_CMD;
                  Cmd_Reg(1) <= READ_STARTING_ADDR;
                  Cmd_Reg(0) <= X"00";
         end if;
         
      elsif Shift_Cmd_Reg = '1' then -- shift to load D_send with the new command byte

                  Cmd_Reg(2) <= Cmd_Reg(1);
                  Cmd_Reg(1) <= Cmd_Reg(0);
                  Cmd_Reg(0) <= X"00";
      end if;
   end if;
end process Load_Shift_Cmd_Reg;

-- Create the address counter for the Cmd_Reg_Data command vectors
-- Increment by two the Cmd_Reg_Data_Addr after a SPI Register Write transaction is done
Advance_Cmd_Reg_Addr <= EN_Advance_Cmd_Reg_Addr AND SPI_Trans_Done;

Count_Addr: process (SYSCLK, RESET, Cmd_Reg_Data_Addr, StC_Adxl_Ctrl, Advance_Cmd_Reg_Addr)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if RESET = '1'  or StC_Adxl_Ctrl = stAdxlCtrlIdle then
         Cmd_Reg_Data_Addr <= 0;
      elsif Advance_Cmd_Reg_Addr = '1' then
         if Cmd_Reg_Data_Addr =  (NUM_COMMAND_VEC - 1) then -- Avoid to address Cmd_Reg_Data out of range
            Cmd_Reg_Data_Addr <= 0;
         else
            Cmd_Reg_Data_Addr <= Cmd_Reg_Data_Addr + 1;
         end if;
      end if;
   end if;
end process Count_Addr;

-- Signal when all of the addresses were read 
Cmd_Reg_Addr_Done <= '1' when Cmd_Reg_Data_Addr = (NUM_COMMAND_VEC - 1) else '0';


-- Shift Data_Reg when a new byte comes
Shift_Data_Reg <= EN_Shift_Data_Reg AND Done;
-- Read incoming data
Read_Data: process (SYSCLK, Shift_Data_Reg, D_Rec, Data_Reg) -- When reading the status register, one byte is read, therefore
variable i: integer range 0 to 6 := 0;                       -- the status register data will be on Data_Reg(0)
begin                                                        -- When reading incoming data, exactly 8 reads are performed,
   if SYSCLK'EVENT AND SYSCLK = '1' then                     -- therefore no initialization is required for Data_Reg
      if Shift_Data_Reg = '1' then                     
         for i in 0 to 6 loop
            Data_Reg(i+1) <= Data_Reg(i);
         end loop;
            Data_Reg(0) <= D_Rec;
      end if;
   end if;
end process Read_Data;

-- Count the bytes to be send and to be received
Count_Bytes_Send: process (SYSCLK, Reset_Cnt_Bytes, Load_Cnt_Bytes_Sent, Shift_Cmd_Reg, Cnt_Bytes_Sent)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if Reset_Cnt_Bytes = '1' then
          Cnt_Bytes_Sent <= 0;
          
      elsif Load_Cnt_Bytes_Sent = '1' then

        if   (StC_Adxl_Ctrl = stAdxlSendResetCmd) 
          or (StC_Adxl_Ctrl = stAdxlConf_Remaining) then -- In this case send 3 command bytes
                  -- Decrementing and shifting Cmd_Reg will be done BEFORE sending data 
                  -- through the serial interface, therefore load NUMBYTES_CMD_CONFIG_REG or NUMBYTES_CMD_READ.
                  -- The condition to end SPI send operation is Cnt_Bytes_Sent = 0 AND Done = '1'
                  Cnt_Bytes_Sent <= NUMBYTES_CMD_CONFIG_REG; 

        elsif  (StC_Adxl_Ctrl = stAdxlRead_Status) 
            or (StC_Adxl_Ctrl = stAdxlRead_Data) then -- In the case of read command, send 2 command bytes

                  Cnt_Bytes_Sent <= NUMBYTES_CMD_READ;
        else
                  Cnt_Bytes_Sent <= 0;
        end if;

      elsif Shift_Cmd_Reg = '1' then -- When shifting Cmd_Reg, decrement the counter
           if Cnt_Bytes_Sent = 0 then
               Cnt_Bytes_Sent <= 0; -- Stay at 0, reload at the next SPI transaction
           else
               Cnt_Bytes_Sent <= Cnt_Bytes_Sent - 1;
           end if;
      end if;
   end if;
end process Count_Bytes_Send;


Count_Bytes_Rec: process (SYSCLK, Reset_Cnt_Bytes, Load_Cnt_Bytes_Rec, Shift_Data_Reg, Cnt_Bytes_Rec)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if Reset_Cnt_Bytes = '1' then
          Cnt_Bytes_Rec <= 0;
 
     elsif Load_Cnt_Bytes_Rec = '1' then
        if   (StC_Adxl_Ctrl = stAdxlRead_Status)  then -- In this case we have to read 1 byte
                 -- Decrementing and shifting Data_Reg will be done AFTER sending data 
                 -- through the serial interface, therefore load NUMBYTES_READ_STATUS - 1 or NUMBYTES_READ_DATA - 1. 
                 -- The condition to end SPI receive operation is Cnt_Bytes_Rec = 0 AND Done = '1'
                  Cnt_Bytes_Rec <= NUMBYTES_READ_STATUS - 1; 

        elsif (StC_Adxl_Ctrl = stAdxlRead_Data) then -- In the case we have to read 8 bytes
                  Cnt_Bytes_Rec <= NUMBYTES_READ_DATA -1;
        else
                  Cnt_Bytes_Rec <= 0;
        end if;
        
      elsif Shift_Data_Reg = '1' then -- When shifting Data_Reg, decrement the counter
           if Cnt_Bytes_Rec = 0 then
               Cnt_Bytes_Rec <= 0; -- Stay at 0, reload at the next SPI transaction
           else
               Cnt_Bytes_Rec <= Cnt_Bytes_Rec - 1;
           end if;
      end if;
   end if;
end process Count_Bytes_Rec;

-- Create the Sample_Rate_Div counter and the Sample_Rate_Tick signal
Count_Sample_Rate_Div: process (SYSCLK, Reset_Sample_Rate_Div, Sample_Rate_Div)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if Reset_Sample_Rate_Div = '1' then
         Sample_Rate_Div <= 0;
      elsif Sample_Rate_Div = (UPDATE_DIV_RATE - 1) then   
         Sample_Rate_Div <= 0;
      else
         Sample_Rate_Div <= Sample_Rate_Div + 1;
      end if;
   end if;
end process Count_Sample_Rate_Div;

Sample_Rate_Tick  <= '1' when Sample_Rate_Div = (UPDATE_DIV_RATE - 1) else '0';


-- Create the Cnt_Num_Reads counter, self-blocking
Count_Num_Reads: process (SYSCLK, Reset_Cnt_Num_Reads, CE_Cnt_Num_Reads, Cnt_Num_Reads)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if Reset_Cnt_Num_Reads = '1' then
         Cnt_Num_Reads <= 0;
      elsif CE_Cnt_Num_Reads = '1' then
            if Cnt_Num_Reads = (NUM_READS - 1) then   
               Cnt_Num_Reads <= (NUM_READS - 1);
            else
               Cnt_Num_Reads <= Cnt_Num_Reads + 1;
            end if;
      end if;
   end if;
end process Count_Num_Reads;

Cnt_Num_Reads_Done <= '1' when Cnt_Num_Reads = (NUM_READS - 1) else '0';

-- Create the Cnt_SS_Inactive counter, also self_blocking
Count_SS_Inactive: process (SYSCLK, RESET, Reset_Cnt_SS_Inactive, Cnt_SS_Inactive)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
      if RESET = '1' or Reset_Cnt_SS_Inactive = '1' then
         Cnt_SS_Inactive <= 0;
      elsif Cnt_SS_Inactive = (SS_INACTIVE_CLOCKS - 1) then   
         Cnt_SS_Inactive <= (SS_INACTIVE_CLOCKS - 1);
      else
         Cnt_SS_Inactive <= Cnt_SS_Inactive + 1;
      end if;
   end if;
end process Count_SS_Inactive;

Cnt_SS_Inactive_done <= '1' when Cnt_SS_Inactive = (SS_INACTIVE_CLOCKS - 1) else '0';


-- SPI Send/Receive State Machine internal condition signals
-- SPI_RnW will be controlled according to the states of the Adxl 362 Control state machine
Set_SPI_RnW: process (SYSCLK, StC_Adxl_Ctrl)
begin
   if SYSCLK'EVENT AND SYSCLK = '1' then
         if    (StC_Adxl_Ctrl = stAdxlRead_Status) 
            or (StC_Adxl_Ctrl = stAdxlRead_Data) then
                  SPI_RnW <= '1';
         else
                  SPI_RnW <= '0';
         end if;
   end if;
end process Set_SPI_RnW;


-- SPI Send/Receive State machine internal condition signals
SPI_WR_Done <= '1' when Cnt_Bytes_Sent = 0 AND Done = '1' else '0';
SPI_RD_Done <= '1' when Cnt_Bytes_Rec = 0 AND Done = '1' else '0';

-- Spi Send/Receive State Machine register process
Register_StC_Spi_SendRec: process (SYSCLK, RESET, StN_Spi_SendRec)
begin
      if SYSCLK'EVENT AND SYSCLK = '1' then
         if RESET = '1' then
            StC_Spi_SendRec <= stSpiSendRecIdle;
         else
            StC_Spi_SendRec <= StN_Spi_SendRec;
         end if;
      end if;
end process Register_StC_Spi_SendRec;

-- Spi Send/Receive State Machine transitions process
Cmb_StC_Spi_SendRec: process (StC_Spi_SendRec, StartSpiSendRec, StartSpiSendRec, 
                              SPI_WR_Done, SPI_RD_Done, SPI_RnW, Done)
begin
   StN_Spi_SendRec <= StC_Spi_SendRec; -- Default: Stay in the current state
   case (StC_Spi_SendRec) is
      when stSpiSendRecIdle => if (StartSpiSendRec = '1') then StN_Spi_SendRec <= stSpiPrepareCmd; end if;
      when stSpiPrepareCmd  => StN_Spi_SendRec <= stSpiSendStartW;
      when stSpiSendStartW  => StN_Spi_SendRec <= stSpiWaitOnDoneW;
      when stSpiWaitOnDoneW => if (SPI_RnW = '1') then -- in the case of a read command proceed to reading data, if writing is done
                                   if (SPI_WR_Done = '1') then StN_Spi_SendRec <= stSpiSendStartR;
                                   elsif (Done = '1') then StN_Spi_SendRec <= stSpiPrepareCmd; -- Return to send the next command byte
                                   end if;
                               else
                                    if (SPI_WR_Done = '1') then StN_Spi_SendRec <= stSpiSendRecDone; -- Sending command bytes finished
                                    elsif (Done = '1') then StN_Spi_SendRec <= stSpiPrepareCmd; -- Return to send the next command byte
                                    end if;
                               end if;
      when stSpiSendStartR  => StN_Spi_SendRec <= stSpiWaitOnDoneR; --Send Start command to the SPI interface and wait until reads a byte
      when stSpiWaitOnDoneR => if (SPI_RD_Done = '1') then StN_Spi_SendRec <= stSpiSendRecDone; -- If all of the bytes were read
                               elsif (Done = '1') then StN_Spi_SendRec <= stSpiSendStartR; -- Return and send another Start command to read
                               end if;                                                     -- the next byte
      when stSpiSendRecDone => StN_Spi_SendRec <= stSpiSendRecIdle;
      when others => StN_Spi_SendRec <= stSpiSendRecIdle;
   end case;
end process Cmb_StC_Spi_SendRec;


-- SPI Transaction State Machine register process
Register_StC_Spi_Trans: process (SYSCLK, RESET, StN_Spi_Trans)
begin
      if SYSCLK'EVENT AND SYSCLK = '1' then
         if RESET = '1' then
            StC_Spi_Trans <= stSpiTransIdle;
         else
            StC_Spi_Trans <= StN_Spi_Trans;
         end if;
      end if;
end process Register_StC_Spi_Trans;


-- SPI Transaction State Machine transitions process
Cmb_StC_Spi_Trans: process (StC_Spi_Trans, StartSpiTr, Cnt_SS_Inactive_done, SPI_SendRec_Done)
begin
   StN_Spi_Trans <= StC_Spi_Trans; -- Default: Stay in the current state
   case (StC_Spi_Trans) is
      when stSpiTransIdle      => if (StartSpiTr = '1') then StN_Spi_Trans <= stSpiPrepAndSendCmd; end if; -- Start SPI Transaction
      when stSpiPrepAndSendCmd => StN_Spi_Trans <= stSpiWaitonDoneSR; -- Load Cmd_Reg, the Cnt_Bytes_Sent and Cnt_Bytes_Rec counters and send 
                                                                      -- the Start command to the Spi Send/Receive state machine
      when stSpiWaitonDoneSR   => if (SPI_SendRec_Done = '1') then StN_Spi_Trans <= stSpiWaitForSsInact; end if; -- SPI Send/Receive done,
                                                                                                           -- Discativate SS for SS_INACTIVE_CLOCKS
      when stSpiWaitForSsInact => if (Cnt_SS_Inactive_done = '1') then StN_Spi_Trans <= stSpiTransDone; end if; -- SS_INACTIVE_CLOCKS passed, go to
                                                                                                          -- the Done state
      when stSpiTransDone      => StN_Spi_Trans <= stSpiTransIdle;
      when others => StN_Spi_Trans <= stSpiTransIdle;
   end case;
end process Cmb_StC_Spi_Trans;

-- The Status Register read will be on Data_Reg(0), bit 0 shows if new data is ready
-- Adxl_Data_Ready and Adxl_Conf_Err will be tested in the ADXL 362 Control State Machine, in the stAdxlRead_Status state
Adxl_Data_Ready <= Data_Reg(0)(0);
Adxl_Conf_Err <= Data_Reg(0)(7);

-- ADXL 362 Control State Machine register process
Register_StC_Adxl_Ctrl: process (SYSCLK, RESET, StN_Adxl_Ctrl)
begin
      if SYSCLK'EVENT AND SYSCLK = '1' then
         if RESET = '1' then
            StC_Adxl_Ctrl <= stAdxlCtrlIdle;
         else
            StC_Adxl_Ctrl <= StN_Adxl_Ctrl;
         end if;
      end if;
end process Register_StC_Adxl_Ctrl;

-- ADXL 362 Control State Machine Transitions process
Cmb_StC_Adxl_Ctrl: process (StC_Adxl_Ctrl, Cnt_SS_Inactive_done, SPI_Trans_Done, Sample_Rate_Tick,
                            Cmd_Reg_Addr_Done, Adxl_Data_Ready, Adxl_Conf_Err, Cnt_Num_Reads_Done)
begin
   StN_Adxl_Ctrl <= StC_Adxl_Ctrl; -- Default: Stay in the current state
   case (StC_Adxl_Ctrl) is
      when stAdxlCtrlIdle           => if (Sample_Rate_Tick = '1') then StN_Adxl_Ctrl <= stAdxlSendResetCmd; end if; -- wait for some clock periods 
                                                                                                                         -- before start
      when stAdxlSendResetCmd       => if (SPI_Trans_Done = '1') then StN_Adxl_Ctrl <= stAdxlWaitResetDone; end if; -- Send the Reset command to the ADXL 362
      when stAdxlWaitResetDone      => if (Sample_Rate_Tick = '1') then StN_Adxl_Ctrl <= stAdxlConf_Remaining; end if; -- wait for about 1mS for the
                                                                                                                       -- ADXL 362 to initialize
      when stAdxlConf_Remaining     => if ( Cmd_Reg_Addr_Done = '1' AND SPI_Trans_Done = '1') then -- all of the configuration register data were written
                                             StN_Adxl_Ctrl <= stAdxlWaitSampleRateTick;      -- into the ADXL 362
                                       end if;
      when stAdxlWaitSampleRateTick => if (Sample_Rate_Tick = '1') then StN_Adxl_Ctrl <= stAdxlRead_Status; end if; -- Read and check the 
                                                                                                                     --status register
      when stAdxlRead_Status        => if SPI_Trans_Done = '1' then 
                                                if Adxl_Conf_Err = '1' then StN_Adxl_Ctrl <= stAdxlCtrlIdle; -- if error ocurred in configuration, go to the ilde state 
                                                                                                             -- and send reset command again
                                                elsif Adxl_Data_Ready = '1' then StN_Adxl_Ctrl <= stAdxlRead_Data; -- If data is available, start data read
                                                end if;
                                       end if;
      when stAdxlRead_Data          => if (SPI_Trans_Done = '1') then StN_Adxl_Ctrl <= stAdxlFormatandSum; end if; -- If a set of data is read,
                                                                                                                   -- add it to the accumulators
      when stAdxlFormatandSum       => if (Cnt_Num_Reads_Done = '1') then 
                                            StN_Adxl_Ctrl <= stAdxlRead_Done; -- done 16 reads, go to the Done state 
                                       else
                                            StN_Adxl_Ctrl <= stAdxlRead_Status; -- Proceed to the next read
                                       end if;
      when stAdxlRead_Done          => StN_Adxl_Ctrl <= stAdxlWaitSampleRateTick; -- Wait for the next Sample_Rate_Tick, in an infinite loop
      when others => StN_Adxl_Ctrl <= stAdxlCtrlIdle;
   end case;
end process Cmb_StC_Adxl_Ctrl;

-- Create the accumulators
-- Data_Reg is shifted from 0 to 7, it means that after reading a set of data, Data_Reg will contain 
-- Data_Reg(7) = XDATA_L,
-- Data_Reg(6) = XDATA_H,
-- Data_Reg(5) = YDATA_L,
-- Data_Reg(4) = YDATA_H,
-- Data_Reg(3) = ZDATA_L,
-- Data_Reg(2) = ZDATA_H,
-- Data_Reg(1) = TEMP_L,
-- Data_Reg(0) = TEMP_H
Sum_Data: process (SYSCLK, RESET, Data_Ready_1, Enable_Sum, Data_Reg, 
                   ACCEL_X_SUM, ACCEL_Y_SUM, ACCEL_Z_SUM, ACCEL_TMP_SUM)
begin
    if SYSCLK'EVENT AND SYSCLK = '1' then
         if (RESET = '1' OR Data_Ready_1 = '1') then
            ACCEL_X_SUM <= (others => '0');
            ACCEL_Y_SUM <= (others => '0');
            ACCEL_Z_SUM <= (others => '0');
            ACCEL_TMP_SUM <= (others => '0');
         elsif Enable_Sum = '1' then
            ACCEL_X_SUM <= ACCEL_X_SUM + (Data_Reg(6)((3 + (NUM_READS_BITS)) downto 0) & Data_Reg(7));
            ACCEL_Y_SUM <= ACCEL_Y_SUM + (Data_Reg(4)((3 + (NUM_READS_BITS)) downto 0) & Data_Reg(5));
            ACCEL_Z_SUM <= ACCEL_Z_SUM + (Data_Reg(2)((3 + (NUM_READS_BITS)) downto 0) & Data_Reg(3));
            ACCEL_TMP_SUM <= ACCEL_TMP_SUM + (Data_Reg(0)((3 + (NUM_READS_BITS)) downto 0) & Data_Reg(1));
         end if;
    end if;
end process Sum_Data;
                  
-- Register the output data
Register_Output_Data: process (SYSCLK, RESET, Data_Ready_1, ACCEL_X_SUM, 
                               ACCEL_Y_SUM, ACCEL_Z_SUM, ACCEL_TMP_SUM)
begin
    if SYSCLK'EVENT AND SYSCLK = '1' then
         if RESET = '1' then
            ACCEL_X <= (others => '0');
            ACCEL_Y <= (others => '0');
            ACCEL_Z <= (others => '0');
            ACCEL_TMP <= (others => '0');
         elsif Data_Ready_1 = '1' then -- Divide by NUM_READS to create the average and set the output data
            ACCEL_X <= ACCEL_X_SUM ((11 + (NUM_READS_BITS)) downto (NUM_READS_BITS)); -- 12 bits
            ACCEL_Y <= ACCEL_Y_SUM ((11 + (NUM_READS_BITS)) downto (NUM_READS_BITS));
            ACCEL_Z <= ACCEL_Z_SUM ((11 + (NUM_READS_BITS)) downto (NUM_READS_BITS));
            ACCEL_TMP <= ACCEL_TMP_SUM ((11 + (NUM_READS_BITS)) downto (NUM_READS_BITS));
         end if;
    end if;
end process Register_Output_Data;

-- Pipe Data_Ready from Data_Ready_1 
-- to have stable output data when Data_Ready becomes active
Pipe_Data_Ready: process (SYSCLK, RESET, Data_Ready_1)
begin
    if SYSCLK'EVENT AND SYSCLK = '1' then
         if RESET = '1' then
            Data_Ready <= '0';
         else
            Data_Ready <= Data_Ready_1;
         end if;
    end if;
end process Pipe_Data_Ready;
 
end Behavioral;

