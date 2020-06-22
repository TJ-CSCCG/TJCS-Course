------------------------------------------------------------------------
-- mouse_controller.vhd
------------------------------------------------------------------------
-- Author : Ulrich Zoltán
--          Copyright 2006 Digilent, Inc.
------------------------------------------------------------------------
-- This file contains a controller for a ps/2 compatible mouse device.
-- This controller uses the ps2interface module.
------------------------------------------------------------------------
--  Behavioral description
------------------------------------------------------------------------
-- Please read the following article on the web for understanding how
-- to interface a ps/2 mouse:
-- http://www.computer-engineering.org/ps2mouse/

-- This controller is implemented as described in the above article.
-- The mouse controller receives bytes from the ps2interface which, in
-- turn, receives them from the mouse device. Data is received on the
-- rx_data input port, and is validated by the read signal. read is
-- active for one clock period when new byte available on rx_data. Data
-- is sent to the ps2interface on the tx_data output port and validated
-- by the write output signal. 'write' should be active for one clock
-- period when tx_data contains the command or data to be sent to the
-- mouse. ps2interface wraps the byte in a 11 bits packet that is sent
-- through the ps/2 port using the ps/2 protocol. Similarly, when the
-- mouse sends data, the ps2interface receives 11 bits for every byte,
-- extracts the byte from the ps/2 frame, puts it on rx_data and
-- activates read for one clock period. If an error occurs when sending
-- or receiving a frame from the mouse, the err input goes high for one
-- clock period. When this occurs, the controller enters reset state.

-- When in reset state, the controller resets the mouse and begins an
-- initialization procedure that consists of tring to put mouse in
-- scroll mode (enables wheel if the mouse has one), setting the
-- resolution of the mouse, the sample rate and finally enables
-- reporting. Implicitly the mouse, after a reset or imediately after a
-- reset, does not send data packets on its own. When reset(or power-up)
-- the mouse enters reset state, where it performs a test, called the
-- bat test (basic assurance test), when this test is done, it sends
-- the result: AAh for test ok, FCh for error. After this it sends its
-- ID which is 00h. When this is done, the mouse enters stream mode,
-- but with reporting disabled (movement data packets are not sent).
-- To enable reporting the enable data reporting command (F4h) must be
-- sent to the mouse. After this command is sent, the mouse will send
-- movement data packets when the mouse is moved or the status of the
-- button changes.

-- After sending a command or a byte following a command, the mouse
-- must respond with ack (FAh). For managing the intialization
-- procedure and receiving the movement data packets, a FSM is used.
-- When the fpga is powered up or the logic is reset using the global
-- reset, the FSM enters reset state. From this state, the FSM will
-- transition to a series of states used to initialize the mouse. When
-- initialization is complete, the FSM remains in state read_byte_1,
-- waiting for a movement data packet to be sent. This is the idle
-- state if the FSM. When a byte is received in this state, this is
-- the first byte of the 3 bytes sent in a movement data packet (4 bytes
-- if mouse in scrolling mode). After reading the last byte from the
-- packet, the FSM enters mark_new_event state and sets new_event high.
-- After that FSM enterss read_byte_1 state, resets new_event and waits
-- for a new packet.
-- After a packet is received, new_event is set high for one clock
-- period to "inform" the clients of this controller a new packet was
-- received and processed.

-- During the initialization procedure, the controller tries to put the
-- mouse in scroll mode (activates wheel, if mouse has one). This is
-- done by successively setting the sample rate to 200, then to 100, and
-- lastly to 80. After this is done, the mouse ID is requested by 
-- sending get device ID command (F2h). If the received ID is 00h than
-- the mouse does not have a wheel. If the received ID is 03h than the
-- mouse is in scroll mode, and when sending movement data packets
-- (after enabling data reporting) it will include z movement data.
-- If the mouse is in normal, non-scroll mode, the movement data packet
-- consists of 3 successive bytes. This is their format:
--
--
--
-- bits      7     6     5     4     3     2     1     0
--        -------------------------------------------------
-- byte 1 | YOVF| XOVF|YSIGN|XSIGN|  1  | MBTN| RBTN| LBTN|
--        -------------------------------------------------
--        -------------------------------------------------
-- byte 2 |                  X MOVEMENT                   |
--        -------------------------------------------------
--        -------------------------------------------------
-- byte 3 |                  Y MOVEMENT                   |
--        -------------------------------------------------
-- OVF = overflow
-- BTN = button
-- M = middle
-- R = right
-- L = left
--
-- When scroll mode is enabled, the mouse send 4 byte movement packets.
-- bits      7     6     5     4     3     2     1     0
--        -------------------------------------------------
-- byte 1 | YOVF| XOVF|YSIGN|XSIGN|  1  | MBTN| RBTN| LBTN|
--        -------------------------------------------------
--        -------------------------------------------------
-- byte 2 |                  X MOVEMENT                   |
--        -------------------------------------------------
--        -------------------------------------------------
-- byte 3 |                  Y MOVEMENT                   |
--        -------------------------------------------------
--        -------------------------------------------------
-- byte 4 |                  Z MOVEMENT                   |
--        -------------------------------------------------
-- x and y movement counters are represented on 8 bits, 2's complement
-- encoding. The first bit (sign bit) of the counters are the xsign and
-- ysign bit from the first packet, the rest of the bits are the second
-- byte for the x movement and the third byte for y movement. For the
-- z movement the range is -8 -> +7 and only the 4 least significant
-- bits from z movement are valid, the rest are sign extensions.
-- The x and y movements are in range: -256 -> +255

-- The mouse uses as axes origin the lower-left corner. For the purpose
-- of displaying a mouse cursor on the screen, the controller inverts
-- the y axis to move the axes origin in the upper-left corner. This
-- is done by negating the y movement value (following the 2s complement
-- encoding). The movement data received from the mouse are delta
-- movements, the data represents the movement of the mouse relative
-- to the last position. The controller keeps track of the position of
-- the mouse relative to the upper-left corner. This is done by keeping
-- the mouse position in two registers x_pos and y_pos and adding the
-- delta movements to their value. The addition uses saturation. That
-- means the value of the mouse position will not exceed certain bounds
-- and will not rollover the a margin. For example, if the mouse is at
-- the left margin and is moved left, the x position remains at the left
-- margin(0). The lower bound is always 0 for both x and y movement.
-- The upper margin can be set using input pins: value, setmax_x,
-- setmax_y. To set the upper bound of the x movement counter, the new
-- value is placed on the value input pins and setmax_x is activated
-- for at least one clock period. Similarly for y movement counter, but
-- setmax_y is activated instead. Notice that value has 10 bits, and so
-- the maximum value for a bound is 1023.

-- The position of the mouse (x_pos and y_pos) can be set at any time,
-- by placing the x or y position on the value input pins and activating
-- the setx, or sety respectively, for at least one clock period. This
-- is useful for setting an original position of the mouse different
-- from (0,0).
------------------------------------------------------------------------
--  Port definitions
------------------------------------------------------------------------
-- clk            - global clock signal (100MHz)
-- rst            - global reset signal
-- xpos           - output pin, 10 bits
--                - the x position of the mouse relative to the upper
--                - left corner
-- ypos           - output pin, 10 bits
--                - the y position of the mouse relative to the upper
--                - left corner
-- zpos           - output pin, 4 bits
--                - last delta movement on z axis
-- left           - output pin, high if the left mouse button is pressed
-- middle         - output pin, high if the middle mouse button is
--                - pressed
-- right          - output pin, high if the right mouse button is
--                - pressed
-- new_event      - output pin, active one clock period after receiving
--                - and processing one movement data packet.
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

-- the mouse_controller entity declaration
-- read above for behavioral description and port definitions.
entity MouseCtl is
generic
(
   SYSCLK_FREQUENCY_HZ : integer := 100000000;
   CHECK_PERIOD_MS     : integer := 500; -- Period in miliseconds to check if the mouse is present
   TIMEOUT_PERIOD_MS   : integer := 100 -- Timeout period in miliseconds when the mouse presence is checked
);
port(
   clk         : in std_logic;
   rst         : in std_logic;
   xpos        : out std_logic_vector(11 downto 0);
   ypos        : out std_logic_vector(11 downto 0);
   zpos        : out std_logic_vector(3 downto 0);
   left        : out std_logic;
   middle      : out std_logic;
   right       : out std_logic;
   new_event   : out std_logic;
   value       : in std_logic_vector(11 downto 0);
   setx        : in std_logic;
   sety        : in std_logic;
   setmax_x    : in std_logic;
   setmax_y    : in std_logic;
   
   ps2_clk     : inout std_logic;
   ps2_data    : inout std_logic
   
);
end MouseCtl;

architecture Behavioral of MouseCtl is

------------------------------------------------------------------------
-- Ps2 Interface component declaration
------------------------------------------------------------------------
COMPONENT Ps2Interface
PORT(
   ps2_clk        : inout std_logic;
   ps2_data       : inout std_logic;
   clk            : in std_logic;
   rst            : in std_logic;
   tx_data        : in std_logic_vector(7 downto 0);
   write_data     : in std_logic;
   rx_data        : out std_logic_vector(7 downto 0);
   read_data      : out std_logic;
   busy           : out std_logic;
   err            : out std_logic
);
END COMPONENT;

------------------------------------------------------------------------
-- CONSTANTS
------------------------------------------------------------------------
-- constants defining commands to send or received from the mouse
constant FA: std_logic_vector(7 downto 0) := "11111010"; -- 0xFA(ACK)
constant FF: std_logic_vector(7 downto 0) := "11111111"; -- 0xFF(RESET)
constant AA: std_logic_vector(7 downto 0) := "10101010"; -- 0xAA(BAT_OK)
constant OO: std_logic_vector(7 downto 0) := "00000000"; -- 0x00(ID)
                         -- (atention: name is 2 letters O not zero)

-- command to read id
constant READ_ID          : std_logic_vector(7 downto 0) := x"F2";
-- command to enable mouse reporting
-- after this command is sent, the mouse begins sending data packets
constant ENABLE_REPORTING : std_logic_vector(7 downto 0) := x"F4";
-- command to set the mouse resolution
constant SET_RESOLUTION   : std_logic_vector(7 downto 0) := x"E8";
-- the value of the resolution to send after sending SET_RESOLUTION
constant RESOLUTION       : std_logic_vector(7 downto 0) := x"03";
                                                  -- (8 counts/mm)
-- command to set the mouse sample rate
constant SET_SAMPLE_RATE  : std_logic_vector(7 downto 0) := x"F3";

-- the value of the sample rate to send after sending SET_SAMPLE_RATE
constant SAMPLE_RATE      : std_logic_vector(7 downto 0) := x"28";
                                                  -- (40 samples/s)

-- default maximum value for the horizontal mouse position
constant DEFAULT_MAX_X : std_logic_vector(11 downto 0) := x"4FF";
                                                      -- 1279
-- default maximum value for the vertical mouse position
constant DEFAULT_MAX_Y : std_logic_vector(11 downto 0) := x"3FF";
                                                      -- 1023

-- Mouse check tick constants
constant CHECK_PERIOD_CLOCKS   : integer := ((CHECK_PERIOD_MS*1000000)/(1000000000/SYSCLK_FREQUENCY_HZ));
constant TIMEOUT_PERIOD_CLOCKS : integer := ((TIMEOUT_PERIOD_MS*1000000)/(1000000000/SYSCLK_FREQUENCY_HZ));

------------------------------------------------------------------------
-- SIGNALS
------------------------------------------------------------------------

-- after doing the enable scroll mouse procedure, if the ID returned by
-- the mouse is 03 (scroll mouse enabled) then this register will be set
-- If '1' then the mouse is in scroll mode, else mouse is in simple
-- mouse mode.
signal haswheel: std_logic := '0';

-- horizontal and veritcal mouse position
-- origin of axes is upper-left corner
-- the origin of axes the mouse uses is the lower-left corner
-- The y-axis is inverted, by making negative the y movement received
-- from the mouse (if it was positive it becomes negative
-- and vice versa)
signal x_pos,y_pos: std_logic_vector(11 downto 0) := (others => '0');

-- active when an overflow occurred on the x and y axis
-- bits 6 and 7 from the first byte received from the mouse
signal x_overflow,y_overflow: std_logic := '0';

-- active when the x,y movement is negative
-- bits 4 and 5 from the first byte received from the mouse
signal x_sign,y_sign: std_logic := '0';

-- 2's complement value for incrementing the x_pos,y_pos
-- y_inc is the negated value from the mouse in the third byte
signal x_inc,y_inc: std_logic_vector(7 downto 0) := (others => '0');

-- active for one clock period, indicates new delta movement received
-- on x,y axis
signal x_new,y_new: std_logic := '0';

-- maximum value for x and y position registers(x_pos,y_pos)
signal x_max: std_logic_vector(11 downto 0) := DEFAULT_MAX_X;
signal y_max: std_logic_vector(11 downto 0) := DEFAULT_MAX_Y;

-- active when left,middle,right mouse button is down
signal left_down,middle_down,right_down: std_logic := '0';

-- the FSM states
-- states that begin with "reset" are part of the reset procedure.
-- states that end in "_wait_ack" are states in which ack is waited
-- as response to sending a byte to the mouse.
-- read behavioral description above for details.
type fsm_state is
(
   reset,reset_wait_ack,reset_wait_bat_completion,reset_wait_id,
   reset_set_sample_rate_200,reset_set_sample_rate_200_wait_ack,
   reset_send_sample_rate_200,reset_send_sample_rate_200_wait_ack,
   reset_set_sample_rate_100,reset_set_sample_rate_100_wait_ack,
   reset_send_sample_rate_100,reset_send_sample_rate_100_wait_ack,
   reset_set_sample_rate_80,reset_set_sample_rate_80_wait_ack,
   reset_send_sample_rate_80,reset_send_sample_rate_80_wait_ack,
   reset_read_id,reset_read_id_wait_ack,reset_read_id_wait_id,
   reset_set_resolution,reset_set_resolution_wait_ack,
   reset_send_resolution,reset_send_resolution_wait_ack,
   reset_set_sample_rate_40,reset_set_sample_rate_40_wait_ack,
   reset_send_sample_rate_40,reset_send_sample_rate_40_wait_ack,
   reset_enable_reporting,reset_enable_reporting_wait_ack,   
   read_byte_1,read_byte_2,read_byte_3,read_byte_4,
   check_read_id,check_read_id_wait_ack,check_read_id_wait_id,
   mark_new_event
);
-- holds current state of the FSM
signal state: fsm_state := reset;

-- PS2 Interface and Mouse Controller interconnection signals
-- read_data      - from ps2interface
--                - active one clock period when new data received
--                - and available on rx_data
-- err            - from ps2interface
--                - active one clock period when error occurred when
--                - receiving or sending data.
-- rx_data        - 8 bits, from ps2interface
--                - the byte received from the mouse.
-- tx_data        - 8 bits, to ps2interface
--                - byte to be sent to the mouse
-- write_data     - to ps2interface
--                - active one clock period when sending a byte to the
--                - ps2interface.
signal read_data  : std_logic;
signal err  : std_logic;
signal rx_data: std_logic_vector (7 downto 0);
signal tx_data: std_logic_vector (7 downto 0);
signal write_data : std_logic;

-- Periodic checking counter, reset and tick signal
-- The periodic checking counter acts as a watchdog, periodically
-- reading the Mouse ID, therefore checking if the mouse is present
-- If there is no answer, after the timeout period passed, then the
-- state machine is reinitialized
signal periodic_check_cnt        : integer range 0 to (CHECK_PERIOD_CLOCKS - 1) := 0;
signal reset_periodic_check_cnt  : STD_LOGIC := '0';
signal periodic_check_tick       : STD_LOGIC := '0';

-- Self-blocking Timeout checking counter, reset and timeout indication signal
signal timeout_cnt        : integer range 0 to (TIMEOUT_PERIOD_CLOCKS - 1) := 0;
signal reset_timeout_cnt  : STD_LOGIC := '0';
signal timeout            : STD_LOGIC := '0';

begin

   Inst_Ps2Interface: Ps2Interface
   PORT MAP
   (
      ps2_clk        => ps2_clk,
      ps2_data       => ps2_data,
      clk            => clk,
      rst            => rst,
      tx_data        => tx_data,
      write_data     => write_data,
      rx_data        => rx_data,
      read_data      => read_data,
      busy           => open,
      err            => err
   );


-- Create the periodic_check_cnt counter
Count_periodic_check: process (clk, periodic_check_cnt, reset_periodic_check_cnt)
begin
   if clk'EVENT AND clk = '1' then
      if reset_periodic_check_cnt = '1' then
         periodic_check_cnt <= 0;
      elsif periodic_check_cnt = (CHECK_PERIOD_CLOCKS - 1) then   
         periodic_check_cnt <= 0;
      else
         periodic_check_cnt <= periodic_check_cnt + 1;
      end if;
   end if;
end process Count_periodic_check;

periodic_check_tick  <= '1' when periodic_check_cnt = (CHECK_PERIOD_CLOCKS - 1) else '0';

-- Create the timeout counter
Count_timeout: process (clk, timeout_cnt, reset_timeout_cnt)
begin
   if clk'EVENT AND clk = '1' then
      if reset_timeout_cnt = '1' then
         timeout_cnt <= 0;
      elsif timeout_cnt = (TIMEOUT_PERIOD_CLOCKS - 1) then   
         timeout_cnt <= (TIMEOUT_PERIOD_CLOCKS - 1);
      else
         timeout_cnt <= timeout_cnt + 1;
      end if;
   end if;
end process Count_timeout;

timeout  <= '1' when timeout_cnt = (TIMEOUT_PERIOD_CLOCKS - 1) else '0';

   -- left output the state of the left_down register   
   left <= left_down when rising_edge(clk);
   -- middle output the state of the middle_down register
   middle <= middle_down when rising_edge(clk);
   -- right output the state of the right_down register
   right <= right_down when rising_edge(clk);

   -- xpos output is the horizontal position of the mouse
   -- it has the range: 0-x_max
   xpos <= x_pos(11 downto 0) when rising_edge(clk);
   -- ypos output is the vertical position of the mouse
   -- it has the range: 0-y_max
   ypos <= y_pos(11 downto 0) when rising_edge(clk);

   -- sets the value of x_pos from another module when setx is active
   -- else, computes the new x_pos from the old position when new x
   -- movement detected by adding the delta movement in x_inc, or by
   -- adding 256 or -256 when overflow occurs.
   set_x: process(clk)
   variable x_inter: std_logic_vector(11 downto 0);
   variable inc: std_logic_vector(11 downto 0);
   begin
      if(rising_edge(clk)) then
         -- if setx active, set new x_pos value
         if(setx = '1') then
            x_pos <= value;
         -- if delta movement received from mouse
         elsif(x_new = '1') then
            -- if negative movement on x axis
            if(x_sign = '1') then
               -- if overflow occurred
               if(x_overflow = '1') then
                  -- inc is -256
                  inc := "111000000000";
               else
                  -- inc is sign extended x_inc
                  inc := "1111" & x_inc;
               end if;
               -- intermediary horizontal position
               x_inter := x_pos + inc;
               -- if first bit of x_inter is 1
               -- then negative overflow occurred and
               -- new x position is 0.
               -- Note: x_pos and x_inter have 11 bits,
               -- and because xpos has only 10, when
               -- first bit becomes 1, this is considered
               -- a negative number when moving left
               if(x_inter(11) = '1') then
                  x_pos <= (others => '0');
               else
                  x_pos <= x_inter;
               end if;
            -- if positive movement on x axis
            else
               -- if overflow occurred
               if(x_overflow = '1') then
                  -- inc is 256
                  inc := "000100000000";
               else
                  -- inc is sign extended x_inc
                  inc := "0000" & x_inc;
               end if;
               -- intermediary horizontal position
               x_inter := x_pos + inc;
               -- if x_inter is greater than x_max
               -- then positive overflow occurred and
               -- new x position is x_max.
               if(x_inter > ('0' & x_max)) then
                  x_pos <= x_max;
               else
                  x_pos <= x_inter;
               end if;
            end if;
         end if;
      end if;
   end process set_x;

   -- sets the value of y_pos from another module when sety is active
   -- else, computes the new y_pos from the old position when new y
   -- movement detected by adding the delta movement in y_inc, or by
   -- adding 256 or -256 when overflow occurs.
   set_y: process(clk)
   variable y_inter: std_logic_vector(11 downto 0);
   variable inc: std_logic_vector(11 downto 0);
   begin
      if(rising_edge(clk)) then
         -- if sety active, set new y_pos value
         if(sety = '1') then
            y_pos <= value;
         -- if delta movement received from mouse
         elsif(y_new = '1') then
            -- if negative movement on y axis
            -- Note: axes origin is upper-left corner
            if(y_sign = '1') then
               -- if overflow occurred
               if(y_overflow = '1') then
                  -- inc is -256
                  inc := "111100000000";
               else
                  -- inc is sign extended y_inc
                  inc := "1111" & y_inc;
               end if;
               -- intermediary vertical position
               y_inter := y_pos + inc;
               -- if first bit of y_inter is 1
               -- then negative overflow occurred and
               -- new y position is 0.
               -- Note: y_pos and y_inter have 11 bits,
               -- and because ypos has only 10, when
               -- first bit becomes 1, this is considered
               -- a negative number when moving upward
               if(y_inter(11) = '1') then
                  y_pos <= (others => '0');
               else
                  y_pos <= y_inter;
               end if;
            -- if positive movement on y axis
            else
               -- if overflow occurred
               if(y_overflow = '1') then
                  -- inc is 256
                  inc := "000100000000";
               else
                  -- inc is sign extended y_inc
                  inc := "0000" & y_inc;
               end if;
               -- intermediary vertical position
               y_inter := y_pos + inc;
               -- if y_inter is greater than y_max
               -- then positive overflow occurred and
               -- new y position is y_max.
               if(y_inter > (y_max)) then
                  y_pos <= y_max;
               else
                  y_pos <= y_inter;
               end if;
            end if;
         end if;
      end if;
   end process set_y;

   -- sets the maximum value of the x movement register, stored in x_max
   -- when setmax_x is active, max value should be on value input pin
   set_max_x: process(clk,rst)
   begin
      if(rising_edge(clk)) then
         if(rst = '1') then
            x_max <= DEFAULT_MAX_X;
         elsif(setmax_x = '1') then
            x_max <= value;
         end if;
      end if;
   end process set_max_x;

   -- sets the maximum value of the y movement register, stored in y_max
   -- when setmax_y is active, max value should be on value input pin
   set_max_y: process(clk,rst)
   begin
      if(rising_edge(clk)) then
         if(rst = '1') then
            y_max <= DEFAULT_MAX_Y;
         elsif(setmax_y = '1') then
            y_max <= value;
         end if;
      end if;
   end process set_max_y;

   -- Synchronous one process fsm to handle the communication
   -- with the mouse.
   -- When reset and at start-up it enters reset state
   -- where it begins the procedure of initializing the mouse.
   -- After initialization is complete, it waits packets from
   -- the mouse.
   -- Read at Behavioral decription for details.
   manage_fsm: process(clk,rst)
   begin
      -- when reset occurs, give signals default values.
      if(rst = '1') then
         state <= reset;
         haswheel <= '0';
         x_overflow <= '0';
         y_overflow <= '0';
         x_sign <= '0';
         y_sign <= '0';
         x_inc <= (others => '0');
         y_inc <= (others => '0');
         x_new <= '0';
         y_new <= '0';
         new_event <= '0';
         left_down <= '0';
         middle_down <= '0';
         right_down <= '0';
         reset_periodic_check_cnt <= '1';
         reset_timeout_cnt <= '1';


      elsif(rising_edge(clk)) then

         -- at every rising edge of the clock, this signals
         -- are reset, thus assuring that they are active
         -- for one clock period only if a state sets then
         -- because the fsm will transition from the state
         -- that set them on the next rising edge of clock.
         write_data <= '0';
         x_new <= '0';
         y_new <= '0';
         
         case state is

            -- if just powered-up, reset occurred or some error in
            -- transmision encountered, then fsm will transition to
            -- this state. Here the RESET command (FF) is sent to the
            -- mouse, and various signals receive their default values
            -- From here the FSM transitions to a series of states that
            -- perform the mouse initialization procedure. All this
            -- state are prefixed by "reset_". After sending a byte
            -- to the mouse, it respondes by sending ack (FA). All
            -- states that wait ack from the mouse are postfixed by
            -- "_wait_ack".
            -- Read at Behavioral decription for details.
            when reset =>
               haswheel <= '0';
               x_overflow <= '0';
               y_overflow <= '0';
               x_sign <= '0';
               y_sign <= '0';
               x_inc <= (others => '0');
               y_inc <= (others => '0');
               x_new <= '0';
               y_new <= '0';
               left_down <= '0';
               middle_down <= '0';
               right_down <= '0';
               tx_data <= FF;
               write_data <= '1';
               reset_periodic_check_cnt <= '1';
               reset_timeout_cnt <= '1';               
               state <= reset_wait_ack;

            -- wait ack for the reset command.
            -- when received transition to reset_wait_bat_completion.
            -- if error occurs go to reset state.
            when reset_wait_ack =>
               if(read_data = '1') then
                  -- if received ack
                  if(rx_data = FA) then
                     state <= reset_wait_bat_completion;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_wait_ack;
               end if;

            -- wait for bat completion test
            -- mouse should send AA if test is successful
            when reset_wait_bat_completion =>
               if(read_data = '1') then
                  if(rx_data = AA) then
                     state <= reset_wait_id;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_wait_bat_completion;
               end if;

            -- the mouse sends its id after performing bat test
            -- the mouse id should be 00
            when reset_wait_id =>
               if(read_data = '1') then
                  if(rx_data = OO) then
                     state <= reset_set_sample_rate_200;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_wait_id;
               end if;

            -- with this state begins the enable wheel mouse
            -- procedure. The procedure consists of setting
            -- the sample rate of the mouse first 200, then 100
            -- then 80. After this is done, the mouse id is
            -- requested and if the mouse id is 03, then
            -- mouse is in wheel mode and will send 4 byte packets
            -- when reporting is enabled.
            -- If the id is 00, the mouse does not have a wheel
            -- and will send 3 byte packets when reporting is enabled.
            -- This state issues the set_sample_rate command to the
            -- mouse.
            when reset_set_sample_rate_200 =>
               tx_data <= SET_SAMPLE_RATE;
               write_data <= '1';
               state <= reset_set_sample_rate_200_wait_ack;

            -- wait ack for set sample rate command
            when reset_set_sample_rate_200_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_send_sample_rate_200;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_set_sample_rate_200_wait_ack;
               end if;

            -- send the desired sample rate (200 = 0xC8)
            when reset_send_sample_rate_200 =>
               tx_data <= "11001000"; -- 0xC8
               write_data <= '1';
               state <= reset_send_sample_rate_200_wait_ack;

            -- wait ack for sending the sample rate
            when reset_send_sample_rate_200_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_set_sample_rate_100;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_send_sample_rate_200_wait_ack;
               end if;

            -- send the sample rate command
            when reset_set_sample_rate_100 =>
               tx_data <= SET_SAMPLE_RATE;
               write_data <= '1';
               state <= reset_set_sample_rate_100_wait_ack;

            -- wait ack for sending the sample rate command
            when reset_set_sample_rate_100_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_send_sample_rate_100;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_set_sample_rate_100_wait_ack;
               end if;

            -- send the desired sample rate (100 = 0x64)
            when reset_send_sample_rate_100 =>
               tx_data <= "01100100"; -- 0x64
               write_data <= '1';
               state <= reset_send_sample_rate_100_wait_ack;

            -- wait ack for sending the sample rate
            when reset_send_sample_rate_100_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_set_sample_rate_80;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_send_sample_rate_100_wait_ack;
               end if;           

            -- send set sample rate command
            when reset_set_sample_rate_80 =>
               tx_data <= SET_SAMPLE_RATE;
               write_data <= '1';
               state <= reset_set_sample_rate_80_wait_ack;

            -- wait ack for sending the sample rate command
            when reset_set_sample_rate_80_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_send_sample_rate_80;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_set_sample_rate_80_wait_ack;
               end if;

            -- send desired sample rate (80 = 0x50)
            when reset_send_sample_rate_80 =>
               tx_data <= "01010000"; -- 0x50
               write_data <= '1';
               state <= reset_send_sample_rate_80_wait_ack;

            -- wait ack for sending the sample rate
            when reset_send_sample_rate_80_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_read_id;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_send_sample_rate_80_wait_ack;
               end if;           

            -- now the procedure for enabling wheel mode is done
            -- the mouse id is read to determine is mouse is in
            -- wheel mode.
            -- Read ID command is sent to the mouse.
            when reset_read_id =>
               tx_data <= READ_ID;
               write_data <= '1';
               state <= reset_read_id_wait_ack;

            -- wait ack for sending the read id command
            when reset_read_id_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_read_id_wait_id;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_read_id_wait_ack;
               end if;

            -- received the mouse id
            -- if the id is 00, then the mouse does not have
            -- a wheel and haswheel is reset
            -- if the id is 03, then the mouse is in scroll mode
            -- and haswheel is set.
            -- if anything else is received or an error occurred
            -- then the FSM transitions to reset state.
            when reset_read_id_wait_id =>
               if(read_data = '1') then
                  if(rx_data = "000000000") then
                     -- the mouse does not have a wheel
                     haswheel <= '0';
                     state <= reset_set_resolution;
                  elsif(rx_data = "00000011") then  -- 0x03
                     -- the mouse is in scroll mode
                     haswheel <= '1';
                     state <= reset_set_resolution;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_read_id_wait_id;
               end if;

            -- send the set resolution command to the mouse
            when reset_set_resolution =>
               tx_data <= SET_RESOLUTION;
               write_data <= '1';
               state <= reset_set_resolution_wait_ack;

            -- wait ack for sending the set resolution command
            when reset_set_resolution_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_send_resolution;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_set_resolution_wait_ack;
               end if;

            -- send the desired resolution (0x03 = 8 counts/mm)
            when reset_send_resolution =>
               tx_data <= RESOLUTION;
               write_data <= '1';
               state <= reset_send_resolution_wait_ack;

            
            -- wait ack for sending the resolution
            when reset_send_resolution_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_set_sample_rate_40;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_send_resolution_wait_ack;
               end if;

            -- send the set sample rate command
            when reset_set_sample_rate_40 =>
               tx_data <= SET_SAMPLE_RATE;
               write_data <= '1';
               state <= reset_set_sample_rate_40_wait_ack;

            -- wait ack for sending the set sample rate command
            when reset_set_sample_rate_40_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_send_sample_rate_40;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_set_sample_rate_40_wait_ack;
               end if;

            -- send the desired sampele rate.
            -- 40 samples per second is sent.
            when reset_send_sample_rate_40 =>
               tx_data <= SAMPLE_RATE;
               write_data <= '1';
               state <= reset_send_sample_rate_40_wait_ack;

            -- wait ack for sending the sample rate
            when reset_send_sample_rate_40_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= reset_enable_reporting;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_send_sample_rate_40_wait_ack;
               end if;

            -- in this state enable reporting command is sent
            -- to the mouse. Before this point, the mouse
            -- does not send packets. Only after issuing this
            -- command, the mouse begins sending data packets,
            -- 3 byte packets if it doesn't have a wheel and
            -- 4 byte packets if it is in scroll mode.
            when reset_enable_reporting =>
               tx_data <= ENABLE_REPORTING;
               write_data <= '1';
               state <= reset_enable_reporting_wait_ack;

            -- wait ack for sending the enable reporting command
            when reset_enable_reporting_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= read_byte_1;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= reset_enable_reporting_wait_ack;
               end if;

            -- this is idle state of the FSM after the
            -- initialization is complete.
            -- Here the first byte of a packet is waited.
            -- The first byte contains the state of the
            -- buttons, the sign of the x and y movement
            -- and overflow information about these movements
            -- First byte looks like this:
            --    7       6        5        4      3   2   1   0
            ------------------------------------------------------
            -- | Y OVF | X OVF | Y SIGN | X SIGN | 1 | M | R | L |
            ------------------------------------------------------
            when read_byte_1 =>
               -- Start periodic check counter
               reset_periodic_check_cnt <= '0';
               -- reset new_event when back in idle state.
               new_event <= '0';
               -- reset last z delta movement
               zpos <= (others => '0');
               if(read_data = '1') then
                  -- mouse button states
                  left_down <= rx_data(0);
                  middle_down <= rx_data(2);
                  right_down <= rx_data(1);
                  -- sign of the movement data
                  x_sign <= rx_data(4);
                  -- y sign is changed to invert the y axis
                  -- because the mouse uses the lower-left corner
                  -- as axes origin and it is placed in the upper-left
                  -- corner by this inversion (suitable for displaying
                  -- a mouse cursor on the screen).
                  -- y movement data from the third packet must be
                  -- also negated.
                  y_sign <= not rx_data(5);

                  -- overflow status of the x and y movement
                  x_overflow <= rx_data(6);
                  y_overflow <= rx_data(7);
                  
                  -- transition to state read_byte_2
                  state <= read_byte_2;
               elsif periodic_check_tick = '1' then -- Check periodically if the mouse is present
                  state <= check_read_id;
               else
                  -- no byte received yet.
                  state <= read_byte_1;
               end if;

            -- wait the second byte of the packet
            -- this byte contains the x movement counter.
            when read_byte_2 =>
               if(read_data = '1') then
                  -- put the delta movement in x_inc
                  x_inc <= rx_data;
                  -- signal the arrival of new x movement data.
                  x_new <= '1';
                  -- go to state read_byte_3.
                  state <= read_byte_3;
               elsif periodic_check_tick = '1' then -- Check periodically if the mouse is present
                  state <= check_read_id;
               elsif(err = '1') then
                  state <= reset;
               else
                  -- byte not received yet.
                  state <= read_byte_2;
               end if;
            
            -- wait the third byte of the data, that
            -- contains the y data movement counter.
            -- negate its value, for the axis to be
            -- inverted.
            -- If mouse is in scroll mode, transition
            -- to read_byte_4, else go to mark_new_event
            when read_byte_3 =>
               if(read_data = '1') then
                  -- when y movement is 0, then ignore
                  if(rx_data /= "00000000") then
                     -- 2's complement positive numbers
                     -- become negative and vice versa
                     y_inc <= (not rx_data) + "00000001";
                     y_new <= '1';        
                  end if;
                  -- if the mouse has a wheel then transition
                  -- to read_byte_4, else go to mark_new_event
                  if(haswheel = '1') then
                     state <= read_byte_4;
                  else
                     state <= mark_new_event;
                  end if;
               elsif periodic_check_tick = '1' then -- Check periodically if the mouse is present
                  state <= check_read_id;
               elsif(err = '1') then
                  state <= reset;
               else
                     state <= read_byte_3;
               end if;

            -- only reached when mouse is in scroll mode
            -- wait for the fourth byte to arrive
            -- fourth byte contains the z movement counter
            -- only least significant 4 bits are relevant
            -- the rest are sign extension.
            when read_byte_4 =>
               if(read_data = '1') then
                  -- zpos is the delta movement on z
                  zpos <= rx_data(3 downto 0);
                  -- packet completly received,
                  -- go to mark_new_event
                  state <= mark_new_event;
               elsif periodic_check_tick = '1' then -- Check periodically if the mouse is present
                  state <= check_read_id;
               elsif(err = '1') then
                  state <= reset;
               else
                  state <= read_byte_4;
               end if;
            -- From timer to time determined by the CHECK_TICK_PERIOD_MS,
            -- Read ID command is sent to the mouse.
            when check_read_id =>
               -- Start the timeout counter
               reset_timeout_cnt <= '0';
              
               tx_data <= READ_ID;
               write_data <= '1';
               state <= check_read_id_wait_ack;

            -- wait ack for sending the read id command
            when check_read_id_wait_ack =>
               if(read_data = '1') then
                  if(rx_data = FA) then
                     state <= check_read_id_wait_id;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               elsif (timeout = '1') then -- Timeout ocurred, so the mouse is not present, go to the reset state
                  state <= reset;
               else
                  state <= check_read_id_wait_ack;
               end if;

            -- received the mouse id
            -- It means that the mouse is present and reading data
            -- can continue
            -- if anything else is received or timeout or an error occurred
            -- then the FSM transitions to reset state.
            when check_read_id_wait_id =>
               if(read_data = '1') then
                  if(rx_data = "000000000") or (rx_data = "00000011") then
                     -- The mouse is present, so reset the timeout counter
                     reset_timeout_cnt <= '1';
                     state <= read_byte_1;
                  else
                     state <= reset;
                  end if;
               elsif(err = '1') then
                  state <= reset;
               elsif (timeout = '1') then-- Timeout ocurred, so the mouse is not present, go to the reset state
                  state <= reset;
               else
                  state <= check_read_id_wait_id;
               end if;

            -- set new_event high
            -- it will be reset in next state
            -- informs client new packet received and processed
            when mark_new_event =>
               new_event <= '1';
               state <= read_byte_1;

            -- if invalid transition occurred, reset
            when others =>
               state <= reset;
      
         end case;
      end if;
   end process manage_fsm;


end Behavioral;