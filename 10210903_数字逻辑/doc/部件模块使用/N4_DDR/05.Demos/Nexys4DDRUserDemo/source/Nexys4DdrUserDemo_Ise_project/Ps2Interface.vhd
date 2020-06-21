------------------------------------------------------------------------
-- ps2interface.vhd
------------------------------------------------------------------------
-- Author : Ulrich Zoltán
--          Copyright 2006 Digilent, Inc.
------------------------------------------------------------------------
-- This file contains the implementation of a generic bidirectional
-- ps/2 interface.
------------------------------------------------------------------------
--  Behavioral description
------------------------------------------------------------------------
-- Please read the following article on the web for understanding how
-- the ps/2 protocol works.
-- http://www.computer-engineering.org/ps2protocol/

-- This module implements a generic bidirectional ps/2 interface. It can
-- be used with any ps/2 compatible device. It offers its clients a
-- convenient way to exchange data with the device. The interface
-- transparently wraps the byte to be sent into a ps/2 frame, generates
-- parity for byte and sends the frame one bit at a time to the device.
-- Similarly, when receiving data from the ps2 device, the interface
-- receives the frame, checks for parity, and extract the usefull data
-- and forwards it to the client. If an error occurs during receiving
-- or sending a byte, the client is informed by settings the err output
-- line high. This way, the client can resend the data or can issue
-- a resend command to the device.

-- The physical ps/2 interface uses 4 lines
-- For the 6-pin connector pins are assigned as follows: 
-- 1 - Data
-- 2 - Not Implemented
-- 3 - Ground
-- 4 - Vcc (+5V)
-- 5 - Clock
-- 6 - Not Implemented

-- The clock line carries the device generated clock which has a 
-- frequency in range 10 - 16.7 kHz (30 to 50us). When line is idle
-- it is placed in high impedance. The clock is only generated when
-- device is sending or receiving data.
-- The Data and Clock lines are both open-collector with pullup
-- resistors to Vcc. An "open-collector" interface has two possible
-- states: low('0') or high impedance('Z').

-- When device wants to send a byte, it pulls the clock line low and the
-- host(i.e. this interfaces) recognizes that the device is sending data
-- When the host wants to send data, it maeks a request to send. This
-- is done by holding the clock line low for at least 100us, then with
-- the clock line low, the data line is brought low. Next the clock line
-- is released (placed in high impedance). The devices begins generating
-- clock signal on clock line.
-- When receiving data, bits are read from the data line (ps2_data) on
-- the falling edge of the clock (ps2_clk). When sending data, the
-- device reads the bits from the data line on the rising edge of the
-- clock.
-- A frame for sending a byte is comprised of 11 bits as shown bellow:
-- bits     10     9    8    7    6    5    4    3    2    1      0
--        -------------------------------------------------------------
--        | STOP| PAR | D7 | D6 | D5 | D4 | D3 | D2 | D1 | D0 | START |
--        -------------------------------------------------------------
-- STOP  - stop  bit, always '1'
-- PAR   - parity bit, odd parity for the 8 data bits.
--       - select in such way that the number of bits of '1' in the data
--       - bits together with parity bit is odd.
-- D0-7  - data bits.
-- START - start bit, always '0'
--
-- Frame is sent bit by bit starting with the least significant bit
-- (starting bit) and is received the same way. This is done, when
-- receiving, by shifting the frame register to the left when a bit
-- is available and placing the bit on data line on the most significant
-- bit. This way the first bit sent will reach the least significant bit
-- of the frame when all the bits have been received. When sending data
-- the least significant bit of the frame is placed on the data line
-- and the frame is shifted to the right when another bit needs to be
-- sent. During the request to send, when releasing the clock line,
-- the device reads the data line and interprets the data on it as the
-- first bit of the frame. Data line is low at that time, at this is the
-- way the start bit('0') is sent. Because of this, when sending, only
-- 10 shifts of the frame will be made.
-- While the interface is sending or receiving data, the busy output
-- signal goes high. When interface is idle, busy is low.
-- After sending all the bits in the frame, the device must acknowledge
-- the data sent. This is done by the host releasing and data line
-- (clock line is already released) after the last bit is sent. The
-- devices brings the data line and the clock line low, in this order,
-- to acknowledge the data. If data line is high when clock line goes
-- low after last bit, the device did not acknowledge the data and
-- err output is set.
-- A FSM is used to manage the transitions the set all the command
-- signals. States that begin with "rx_" are used to receive data
-- from device and states begining with "tx_" are used to send data
-- to the device.
-- For the parity bit, a ROM holds the parity bit for all possible
-- data (256 possible values, since 8 bits of data). The ROM has
-- dimensions 256x1bit. For obtaining the parity bit of a value,
-- the bit at the data value address is read. Ex: to find the parity
-- bit of 174, the bit at address 174 is read.
-- For generating the necessary delay, counters are used. For example,
-- to generate the 100us delay a 14 bit counter is used that has the
-- upper limit for counting 10000. The interface is designed to run
-- at 100MHz. Thus, 10000x10ns = 100us.

-----------------------------------------------------------------------
-- If using the interface at different frequency than 100MHz, adjusting
-- the delay counters is necessary!!!
-----------------------------------------------------------------------

-- Clock line(ps2_clk) and data line(ps2_data) are passed through a
-- debouncer for the transitions of the clock and data to be clean.
-- Also, ps2_clk_s and ps2_data_s hold the debounced and synchronized
-- value of the clock and data line to the system clock(clk).
------------------------------------------------------------------------
--  Port definitions
------------------------------------------------------------------------
-- ps2_clk        - inout pin, clock line of the ps/2 interface
-- ps2_data       - inout pin, data line of the ps/2 interface
-- clk            - input pin, system clock signal
-- rst            - input pin, system reset signal
-- tx_data        - input pin, 8 bits, from client
--                - data to be sent to the device
-- write_data     - input pin, from client
--                - should be active for one clock period when then
--                - client wants to send data to the device and
--                - data to be sent is valid on tx_data
-- rx_data        - output pin, 8 bits, to client
--                - data received from device
-- read           - output pin, to client
--                - active for one clock period when new data is
--                - available from device
-- busy           - output pin, to client
--                - active while sending or receiving data.
-- err            - output pin, to client
--                - active for one clock period when an error occurred
--                - during sending or receiving.
------------------------------------------------------------------------
-- Revision History:
-- 09/18/2006(UlrichZ): created
------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- simulation library
library UNISIM;
use UNISIM.VComponents.all;

-- the ps2interface entity declaration
-- read above for behavioral description and port definitions.
entity Ps2Interface is
port(
   ps2_clk  : inout std_logic;
   ps2_data : inout std_logic;

   clk      : in std_logic;
   rst      : in std_logic;

   tx_data  : in std_logic_vector(7 downto 0);
   write_data : in std_logic;
   
   rx_data  : out std_logic_vector(7 downto 0);
   read_data  : out std_logic;
   busy     : out std_logic;
   err      : out std_logic

);

-- forces the extraction of distributed ram for
-- the parity rom memory.
-- please remove if block ram is preffered.
attribute rom_extract : string;
attribute rom_extract of Ps2Interface: entity is "yes";
attribute rom_style : string;
attribute rom_style of Ps2Interface: entity is "distributed";

end Ps2Interface;

architecture Behavioral of Ps2Interface is

------------------------------------------------------------------------
-- CONSTANTS
------------------------------------------------------------------------

-- Values are valid for a 100MHz clk. Please adjust for other
-- frequencies if necessary!

-- upper limit for 100us delay counter.
-- 10000 * 10ns = 100us
constant DELAY_100US : std_logic_vector(13 downto 0):= "10011100010000";
                                                 -- 10000 clock periods
-- upper limit for 20us delay counter.
-- 2000 * 10ns = 20us
constant DELAY_20US  : std_logic_vector(10 downto 0) := "11111010000";
                                                  -- 2000 clock periods
-- upper limit for 63clk delay counter.
constant DELAY_63CLK : std_logic_vector(6 downto 0)  := "1111111";
                                                    -- 63 clock periods
-- delay from debouncing ps2_clk and ps2_data signals
constant DEBOUNCE_DELAY : std_logic_vector(3 downto 0)  := "1111";

-- number of bits in a frame
constant NUMBITS: std_logic_vector(3 downto 0) := "1011"; -- 11

-- parity bit position in frame
constant PARITY_BIT: positive := 9;

-- (odd) parity bit ROM
-- Used instead of logic because this way speed is far greater
-- 256x1bit rom
-- If the odd parity bit for a 8 bits number, x, is needed
-- the bit at address x is the parity bit.
type ROM is array(0 to 255) of std_logic;
constant parityrom : ROM := (
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1',
'1','0','0','1','0','1','1','0',
'1','0','0','1','0','1','1','0',
'0','1','1','0','1','0','0','1'
);

------------------------------------------------------------------------
-- SIGNALS
------------------------------------------------------------------------

-- 14 bits counter
-- max value DELAY_100US
-- used to wait 100us
signal delay_100us_count: std_logic_vector(13 downto 0) :=
                                          (others => '0');

-- 11 bits counter
-- max value DELAY_20US
-- used to wait 20us
signal delay_20us_count: std_logic_vector(10 downto 0) :=
                                         (others => '0');
-- 11 bits counter
-- max value DELAY_63CLK
-- used to wait 63 clock periods
signal delay_63clk_count: std_logic_vector(6 downto 0) :=
                                          (others => '0');

-- done signal for the couters above
-- when a counter reaches max value,the corresponding done signal is set
signal delay_100us_done, delay_20us_done, delay_63clk_done: std_logic;

-- enable signal for 100us delay counter
signal delay_100us_counter_enable: std_logic := '0';
-- enable signal for 20us delay counter
signal delay_20us_counter_enable : std_logic := '0';
-- enable signal for 63clk delay counter
signal delay_63clk_counter_enable: std_logic := '0';

-- synchronzed input for ps2_clk and ps2_data
signal ps2_clk_s,ps2_data_s: std_logic := '1';

-- control the output of ps2_clk and ps2_data
-- if 1 then corresponding signal (ps2_clk or ps2_data) is
-- put in high impedance ('Z').
signal ps2_clk_h,ps2_data_h: std_logic := '1';

-- states of the FSM for controlling the communcation with the device
-- states that begin with "rx_" are used when receiving data
-- states that begin with "tx_" are used when transmiting data
type fsm_state is
(
   idle,rx_clk_h,rx_clk_l,rx_down_edge,rx_error_parity,rx_data_ready,
   tx_force_clk_l,tx_bring_data_down,tx_release_clk,
   tx_first_wait_down_edge,tx_clk_l,tx_wait_up_edge,tx_clk_h,
   tx_wait_up_edge_before_ack,tx_wait_ack,tx_received_ack,
   tx_error_no_ack
);

-- the signal that holds the current state of the FSM
-- implicitly state is idle.
signal state: fsm_state := idle;

-- register that holds the frame received or the one to be sent.
-- Its contents are shifted in from the bus one bit at a time
-- from left to right when receiving data and are shifted on the
-- bus (ps2_data) one bit at a time to the right when sending data
signal frame: std_logic_vector(10 downto 0) := (others => '0');

-- how many bits have been sent or received.
signal bit_count: std_logic_vector(3 downto 0) := (others => '0');

-- when active the bit counter is reset.
signal reset_bit_count: std_logic := '0';

-- when active the contents of the frame is shifted to the right
-- and the most significant bit of frame is loaded with ps2_data.
signal shift_frame: std_logic := '0';

-- parity of the byte that was received from the device.
-- must match the parity bit received, else error occurred.
signal rx_parity: std_logic := '0';
-- parity bit that is sent with the frame, representing the
-- odd parity of the byte currently being sent
signal tx_parity: std_logic := '0';

-- when active, frame is loaded with the start bit, data on
-- tx_data, parity bit (tx_parity) and stop bit
-- this frame will be sent to the device.
signal load_tx_data: std_logic := '0';

-- when active bits 8 downto 1 from frame are loaded into
-- rx_data register. This is the byte received from the device.
signal load_rx_data: std_logic := '0';

-- intermediary signals used to debounce the inputs ps2_clk and ps2_data
signal ps2_clk_clean,ps2_data_clean: std_logic := '1';
-- debounce counter for the ps2_clk input and the ps2_data input.
signal clk_count,data_count: std_logic_vector(3 downto 0);
-- last value on ps2_clk and ps2_data.
signal clk_inter,data_inter: std_logic := '1';

begin

   ---------------------------------------------------------------------
   -- FLAGS and PS2 CLOCK AND DATA LINES
   ---------------------------------------------------------------------

   -- clean ps2_clk signal (debounce)
   -- note that this introduces a delay in ps2_clk of
   -- DEBOUNCE_DELAY clocks
   process(clk)
   begin
      if(rising_edge(clk)) then
         -- if the current bit on ps2_clk is different
         -- from the last value, then reset counter
         -- and retain value
         if(ps2_clk /= clk_inter) then
            clk_inter <= ps2_clk;
            clk_count <= (others => '0');
         -- if counter reached upper limit, then
         -- the signal is clean
         elsif(clk_count = DEBOUNCE_DELAY) then
            ps2_clk_clean <= clk_inter;
         -- ps2_clk did not change, but counter did not
         -- reach limit. Increment counter
         else
            clk_count <= clk_count + 1;
         end if;
      end if;
   end process;

   -- clean ps2_data signal (debounce)
   -- note that this introduces a delay in ps2_data of
   -- DEBOUNCE_DELAY clocks
   process(clk)
   begin
      if(rising_edge(clk)) then
         -- if the current bit on ps2_data is different
         -- from the last value, then reset counter
         -- and retain value
         if(ps2_data /= data_inter) then
            data_inter <= ps2_data;
            data_count <= (others => '0');
         -- if counter reached upper limit, then
         -- the signal is clean
         elsif(data_count = DEBOUNCE_DELAY) then
            ps2_data_clean <= data_inter;
         -- ps2_data did not change, but counter did not
         -- reach limit. Increment counter
         else
            data_count <= data_count + 1;
         end if;
      end if;
   end process;
   
   -- Synchronize ps2 entries
   ps2_clk_s <= ps2_clk_clean when rising_edge(clk);
   ps2_data_s <= ps2_data_clean when rising_edge(clk);

   -- Assign parity from frame bits 8 downto 1, this is the parity
   -- that should be received inside the frame on PARITY_BIT position
   rx_parity <= parityrom(conv_integer(frame(8 downto 1)))
                when rising_edge(clk);
   -- The parity for the data to be sent
   tx_parity <= parityrom(conv_integer(tx_data)) when rising_edge(clk);

   -- Force ps2_clk to '0' if ps2_clk_h = '0', else release the line
   -- ('Z' = +5Vcc because of pull-ups)
   ps2_clk <= 'Z' when ps2_clk_h = '1' else '0';

   -- Force ps2_data to '0' if ps2_data_h = '0', else release the line
   -- ('Z' = +5Vcc because of pull-ups)
   ps2_data <= 'Z' when ps2_data_h = '1' else '0';

   -- Control busy flag. Interface is not busy while in idle state.
   busy <= '0' when state = idle else '1';

   -- reset the bit counter when in idle state.
   reset_bit_count <= '1' when state = idle else '0';

   -- Control shifting of the frame
   -- When receiving from device, data is read
   -- on the falling edge of ps2_clk
   -- When sending to device, data is read by device
   -- on the rising edge of ps2_clk
   shift_frame <= '1' when state = rx_down_edge or
                           state = tx_clk_l else
                  '0';

   ---------------------------------------------------------------------
   -- FINITE STATE MACHINE
   ---------------------------------------------------------------------

   -- For the current state establish next state
   -- and give necessary commands
   manage_fsm: process(clk,rst,state,ps2_clk_s,ps2_data_s,write_data,tx_data,
                       bit_count,rx_parity,frame,delay_100us_done,
                       delay_20us_done,delay_63clk_done)
   begin
      -- if reset occurs, go to idle state.
      if(rst = '1') then
         state <= idle;
      elsif(rising_edge(clk)) then
            
         -- default values for these signals
         -- ensures signals are reset to default value
         -- when coditions for their activation are no
         -- longer applied (transition to other state,
         -- where signal should not be active)
         -- Idle value for ps2_clk and ps2_data is 'Z'
         ps2_clk_h <= '1';
         ps2_data_h <= '1';
         load_tx_data <= '0';
         load_rx_data <= '0';
         read_data <= '0';
         err <= '0';

         case state is
   
            -- wait for the device to begin a transmission
            -- by pulling the clock line low and go to state
            -- rx_down_edge or, if write is high, the
            -- client of this interface wants to send a byte
            -- to the device and a transition is made to state
            -- tx_force_clk_l
            when idle =>
               if(ps2_clk_s = '0') then
                  state <= rx_down_edge;
               elsif(write_data = '1') then
                  state <= tx_force_clk_l;               
               else
                  state <= idle;
               end if;
   
            -- ps2_clk is high, check if all the bits have been read
            -- if, last bit read, check parity, and if parity ok
            -- load received data into rx_data.
            -- else if more bits left, then wait for the ps2_clk to
            -- go low
            when rx_clk_h =>
               if(bit_count = NUMBITS) then
                  if(not (rx_parity = frame(PARITY_BIT))) then
                     state <= rx_error_parity;
                  else
                     load_rx_data <= '1';
                     state <= rx_data_ready;
                  end if;
               elsif(ps2_clk_s = '0') then
                  state <= rx_down_edge;
               else
                  state <= rx_clk_h;
               end if;
   
            -- data must be read into frame in this state
            -- the ps2_clk just transitioned from high to low
            when rx_down_edge =>
               state <= rx_clk_l;
   
            -- ps2_clk line is low, wait for it to go high
            when rx_clk_l =>
               if(ps2_clk_s = '1') then
                  state <= rx_clk_h;
               else
                  state <= rx_clk_l;
               end if;
   
            -- parity bit received is invalid
            -- signal error and go back to idle.
            when rx_error_parity =>
               err <= '1';
               state <= idle;
   
            -- parity bit received was good
            -- set read signal for the client to know
            -- a new byte was received and is available on rx_data
            when rx_data_ready =>
               read_data <= '1';
               state <= idle;
   
            -- the client wishes to transmit a byte to the device
            -- this is done by holding ps2_clk down for at least 100us
            -- bringing down ps2_data, wait 20us and then releasing
            -- the ps2_clk.
            -- This constitutes a request to send command.
            -- In this state, the ps2_clk line is held down and
            -- the counter for waiting 100us is eanbled.
            -- when the counter reached upper limit, transition
            -- to tx_bring_data_down
            when tx_force_clk_l =>
               load_tx_data <= '1';
               ps2_clk_h <= '0';
               if(delay_100us_done = '1') then
                  state <= tx_bring_data_down;
               else
                  state <= tx_force_clk_l;
               end if;
   
            -- with the ps2_clk line low bring ps2_data low
            -- wait for 20us and then go to tx_release_clk
            when tx_bring_data_down =>
               -- keep clock line low
               ps2_clk_h <= '0';
               -- set data line low
               -- when clock is released in the next state
               -- the device will read bit 0 on data line
               -- and this bit represents the start bit.
               ps2_data_h <= '0';   -- start bit = '0'
               if(delay_20us_done = '1') then
                  state <= tx_release_clk;
               else
                  state <= tx_bring_data_down;
               end if;
   
            -- release the ps2_clk line
            -- keep holding data line low
            when tx_release_clk =>
               ps2_clk_h <= '1';
               -- must maintain data low,
               -- otherwise will be released by default value
               ps2_data_h <= '0';
               state <= tx_first_wait_down_edge;
   
            -- state is necessary because the clock signal
            -- is not released instantaneously and, because of debounce,
            -- delay is even greater.
            -- Wait 63 clock periods for the clock line to release
            -- then if clock is low then go to tx_clk_l
            -- else wait until ps2_clk goes low.
            when tx_first_wait_down_edge =>
               ps2_data_h <= '0';
               if(delay_63clk_done = '1') then
                  if(ps2_clk_s = '0') then
                     state <= tx_clk_l;
                  else
                     state <= tx_first_wait_down_edge;
                  end if;
               else
                  state <= tx_first_wait_down_edge;
               end if;
   
            -- place the least significant bit from frame
            -- on the data line
            -- During this state the frame is shifted one
            -- bit to the right
            when tx_clk_l =>
               ps2_data_h <= frame(0);
               state <= tx_wait_up_edge;

            -- wait for the clock to go high
            -- this is the edge on which the device reads the data
            -- on ps2_data.
            -- keep holding ps2_data on frame(0) because else
            -- will be released by default value.
            -- Check if sent the last bit and if so, release data line
            -- and go to state that wait for acknowledge
            when tx_wait_up_edge =>
               ps2_data_h <= frame(0);
               -- NUMBITS - 1 because first (start bit = 0) bit was read
               -- when the clock line was released in the request to
               -- send command (see tx_bring_data_down state).
               if(bit_count = NUMBITS-1) then
                  ps2_data_h <= '1';
                  state <= tx_wait_up_edge_before_ack;
               -- if more bits to send, wait for the up edge
               -- of ps2_clk
               elsif(ps2_clk_s = '1') then
                  state <= tx_clk_h;
               else
                  state <= tx_wait_up_edge;
               end if;
   
            -- ps2_clk is released, wait for down edge
            -- and go to tx_clk_l when arrived
            when tx_clk_h =>
               ps2_data_h <= frame(0);
               if(ps2_clk_s = '0') then
                  state <= tx_clk_l;
               else
                  state <= tx_clk_h;
               end if;
   
            -- release ps2_data and wait for rising edge of ps2_clk
            -- once this occurs, transition to tx_wait_ack
            when tx_wait_up_edge_before_ack =>
               ps2_data_h <= '1';
               if(ps2_clk_s = '1') then
                  state <= tx_wait_ack;
               else
                  state <= tx_wait_up_edge_before_ack;
               end if;
            
            -- wait for the falling edge of the clock line
            -- if data line is low when this occurs, the
            -- ack is received
            -- else if data line is high, the device did not
            -- acknowledge the transimission
            when tx_wait_ack =>
               if(ps2_clk_s = '0') then
                  if(ps2_data_s = '0') then
                     -- acknowledge received
                     state <= tx_received_ack;
                  else
                     -- acknowledge not received
                     state <= tx_error_no_ack;
                  end if;
               else
                  state <= tx_wait_ack;
               end if;
   
            -- wait for ps2_clk to be released together with ps2_data
            -- (bus to be idle) and go back to idle state
            when tx_received_ack =>
               if(ps2_clk_s = '1' and ps2_data_s = '1') then
                  state  <= idle;
               else
                  state <= tx_received_ack;
               end if;
   
            -- wait for ps2_clk to be released together with ps2_data
            -- (bus to be idle) and go back to idle state
            -- signal error for not receiving ack
            when tx_error_no_ack =>
               if(ps2_clk_s = '1' and ps2_data_s = '1') then
                  err <= '1';
                  state  <= idle;
               else
                  state <= tx_error_no_ack;
               end if;
   
            -- if invalid transition occurred, signal error and
            -- go back to idle state
            when others =>
               err <= '1';
               state  <= idle;
   
         end case;
      end if;
   end process manage_fsm;

   ---------------------------------------------------------------------
   -- DELAY COUNTERS
   ---------------------------------------------------------------------

   -- Enable the 100us counter only when state is tx_force_clk_l
   delay_100us_counter_enable <= '1' when state = tx_force_clk_l else '0';

   -- Counter for a 100us delay
   -- after done counting, done signal remains active until
   -- enable counter is reset.
   delay_100us_counter: process(clk)
   begin
      if(rising_edge(clk)) then
         if(delay_100us_counter_enable = '1') then
            if(delay_100us_count = (DELAY_100US)) then
               delay_100us_count <= delay_100us_count;
               delay_100us_done <= '1';
            else
               delay_100us_count <= delay_100us_count + 1;
               delay_100us_done <= '0';
            end if;
         else
            delay_100us_count <= (others => '0');
            delay_100us_done <= '0';
         end if;
      end if;
   end process delay_100us_counter;

   -- Enable the 20us counter only when state is tx_bring_data_down
   delay_20us_counter_enable <= '1' when state = tx_bring_data_down else '0'; 

   -- Counter for a 20us delay
   -- after done counting, done signal remains active until
   -- enable counter is reset.
   delay_20us_counter: process(clk)
   begin
      if(rising_edge(clk)) then
         if(delay_20us_counter_enable = '1') then
            if(delay_20us_count = (DELAY_20US)) then
               delay_20us_count <= delay_20us_count;
               delay_20us_done <= '1';
            else
               delay_20us_count <= delay_20us_count + 1;
               delay_20us_done <= '0';
            end if;
         else
            delay_20us_count <= (others => '0');
            delay_20us_done <= '0';
         end if;
      end if;
   end process delay_20us_counter;

   -- Enable the 63clk counter only when state is tx_first_wait_down_edge
   delay_63clk_counter_enable <= '1' when state = tx_first_wait_down_edge else '0';

   -- Counter for a 63 clock periods delay
   -- after done counting, done signal remains active until
   -- enable counter is reset.
   delay_63clk_counter: process(clk)
   begin
      if(rising_edge(clk)) then
         if(delay_63clk_counter_enable = '1') then
            if(delay_63clk_count = (DELAY_63CLK)) then
               delay_63clk_count <= delay_63clk_count;
               delay_63clk_done <= '1';
            else
               delay_63clk_count <= delay_63clk_count + 1;
               delay_63clk_done <= '0';
            end if;
         else
            delay_63clk_count <= (others => '0');
            delay_63clk_done <= '0';
         end if;
      end if;
   end process delay_63clk_counter;

   ---------------------------------------------------------------------
   -- BIT COUNTER AND FRAME SHIFTING LOGIC
   ---------------------------------------------------------------------

   -- counts the number of bits shifted into the frame
   -- or out of the frame.
   bit_counter: process(clk)
   begin
      if(rising_edge(clk)) then
         if(reset_bit_count = '1') then
            bit_count <= (others => '0');
         elsif(shift_frame = '1') then
            bit_count <= bit_count + 1;
         end if;
      end if;
   end process bit_counter;

   -- shifts frame with one bit to right when shift_frame is acitve
   -- and loads data into frame from tx_data then load_tx_data is high
   load_tx_data_into_frame: process(clk)
   begin
      if(rising_edge(clk)) then
         if(load_tx_data = '1') then
            frame(8 downto 1) <= tx_data;       -- byte to send
            frame(0) <= '0';                    -- start bit
            frame(10) <= '1';                   -- stop bit
            frame(9) <= tx_parity;              -- parity bit
         elsif(shift_frame = '1') then
            -- shift right 1 bit
            frame(9 downto 0) <= frame(10 downto 1);
            -- shift in from the ps2_data line
            frame(10) <= ps2_data_s;
         end if;
      end if;
   end process load_tx_data_into_frame;

   -- Loads data from frame into rx_data output when data is ready
   do_load_rx_data: process(clk)
   begin
      if(rising_edge(clk)) then
         if(load_rx_data = '1') then
            rx_data <= frame(8 downto 1);
         end if;
      end if;
   end process do_load_rx_data;

end Behavioral;


