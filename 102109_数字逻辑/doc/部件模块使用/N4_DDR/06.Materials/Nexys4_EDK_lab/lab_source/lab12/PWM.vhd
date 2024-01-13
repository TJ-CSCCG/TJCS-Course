----------------------------------------------------------------------------------
-- Company: Digilent RO
-- Engineer: Mircea DAbacan
-- 
-- Create Date:    16:47:13 12/16/2010 
-- Design Name: 
-- Module Name:    pwm - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--		PWM modulator:
--			input number: x
--			carrier: clk/256 (8 bit)
--			output: PWMOUT
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
-- Generates the PWM signal 

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_signed.all;
Library UNISIM;
use UNISIM.vcomponents.all;

entity PWM is
    port (
        clk: in std_logic;							-- system clock = 50.00MHz;
        x: in std_logic_vector (7 downto 0);	-- the number to be modulated
        PWMOUT: out std_logic						-- the Pulse With Modulated signal
    );
end PWM;

architecture PWM_arch of PWM is

	signal count : std_logic_vector (7 downto 0):="00000000"; 
	signal ckMul : std_logic; 		-- clock multiplied by 4
	
	signal clk_fb:std_logic;

begin

	counter: process (ckMul)
	begin
		if ckMul'event and ckMul='1' then		-- the counter rolls over; 
			count <= count + "00000001";	-- the carrier is (clk frequency)/256
		end if;
	end process;

	compare: process (x,count)
	begin
		if unsigned(count) < unsigned(x) then PWMOUT<='1';	-- x was built as unsigned integer
		else		PWMOUT<='0';
		end if;
	end process;


--   DCM_SP_inst : DCM_SP
--   generic map (
--      CLKDV_DIVIDE => 2.0, --  Divide by: 1.5,2.0,2.5,3.0,3.5,4.0,4.5,5.0,5.5,6.0,6.5
--                           --     7.0,7.5,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0 or 16.0
--      CLKFX_DIVIDE => 1,   --  Can be any interger from 1 to 32
--      CLKFX_MULTIPLY => 4, --  Can be any integer from 1 to 32
--      CLKIN_DIVIDE_BY_2 => FALSE, --  TRUE/FALSE to enable CLKIN divide by two feature
--      CLKIN_PERIOD => 20.0, --  Specify period of input clock
--      CLKOUT_PHASE_SHIFT => "NONE", --  Specify phase shift of "NONE", "FIXED" or "VARIABLE" 
--      CLK_FEEDBACK => "1X",         --  Specify clock feedback of "NONE", "1X" or "2X" 
--      DESKEW_ADJUST => "SYSTEM_SYNCHRONOUS", -- "SOURCE_SYNCHRONOUS", "SYSTEM_SYNCHRONOUS" or
--                                             --     an integer from 0 to 15
--      DLL_FREQUENCY_MODE => "LOW",     -- "HIGH" or "LOW" frequency mode for DLL
--      DUTY_CYCLE_CORRECTION => TRUE, --  Duty cycle correction, TRUE or FALSE
--      PHASE_SHIFT => 0,        --  Amount of fixed phase shift from -255 to 255
--      STARTUP_WAIT => FALSE) --  Delay configuration DONE until DCM_SP LOCK, TRUE/FALSE
--   port map (
--      CLK0 => open,     -- 0 degree DCM CLK ouptput
--      CLK180 => open, -- 180 degree DCM CLK output
--      CLK270 => open, -- 270 degree DCM CLK output
--      CLK2X => open,		-- 2X DCM CLK output
--      CLK2X180 => open, -- 2X, 180 degree DCM CLK out
--      CLK90 => open,   -- 90 degree DCM CLK output
--      CLKDV => open,   -- Divided DCM CLK out (CLKDV_DIVIDE)
--      CLKFX => ckMul,   -- DCM CLK synthesis out (M/D)
--      CLKFX180 => open, -- 180 degree CLK synthesis out
--      LOCKED => open, -- DCM LOCK status output
--      PSDONE => open, -- Dynamic phase adjust done output
--      STATUS => open, -- 8-bit DCM status bits output
--      CLKFB => open,   -- DCM clock feedback
--      CLKIN => clk,   -- Clock input (from IBUFG, BUFG or DCM)
--      PSCLK => open,   -- Dynamic phase adjust clock input
--      PSEN => '0',     -- Dynamic phase adjust enable input
--      PSINCDEC => '0', -- Dynamic phase adjust increment/decrement
--      RST =>'0'        -- DCM asynchronous reset input
--   );

   -- PLLE2_BASE: Base Phase Locked Loop (PLL)
   --             Artix-7
   -- Xilinx HDL Language Template, version 14.2

   PLLE2_BASE_inst : PLLE2_BASE
   generic map (
      BANDWIDTH => "OPTIMIZED",  -- OPTIMIZED, HIGH, LOW
      CLKFBOUT_MULT => 8,        -- Multiply value for all CLKOUT, (2-64)
      CLKFBOUT_PHASE => 0.0,     -- Phase offset in degrees of CLKFB, (-360.000-360.000).
      CLKIN1_PERIOD => 10.0,      -- Input clock period in ns to ps resolution (i.e. 33.333 is 30 MHz).
      -- CLKOUT0_DIVIDE - CLKOUT5_DIVIDE: Divide amount for each CLKOUT (1-128)
      CLKOUT0_DIVIDE => 4,
      CLKOUT1_DIVIDE => 1,
      CLKOUT2_DIVIDE => 1,
      CLKOUT3_DIVIDE => 1,
      CLKOUT4_DIVIDE => 1,
      CLKOUT5_DIVIDE => 1,
      -- CLKOUT0_DUTY_CYCLE - CLKOUT5_DUTY_CYCLE: Duty cycle for each CLKOUT (0.001-0.999).
      CLKOUT0_DUTY_CYCLE => 0.5,
      CLKOUT1_DUTY_CYCLE => 0.5,
      CLKOUT2_DUTY_CYCLE => 0.5,
      CLKOUT3_DUTY_CYCLE => 0.5,
      CLKOUT4_DUTY_CYCLE => 0.5,
      CLKOUT5_DUTY_CYCLE => 0.5,
      -- CLKOUT0_PHASE - CLKOUT5_PHASE: Phase offset for each CLKOUT (-360.000-360.000).
      CLKOUT0_PHASE => 0.0,
      CLKOUT1_PHASE => 0.0,
      CLKOUT2_PHASE => 0.0,
      CLKOUT3_PHASE => 0.0,
      CLKOUT4_PHASE => 0.0,
      CLKOUT5_PHASE => 0.0,
      DIVCLK_DIVIDE => 1,        -- Master division value, (1-56)
      REF_JITTER1 => 0.500,        -- Reference input jitter in UI, (0.000-0.999).
      STARTUP_WAIT => "FALSE"    -- Delay DONE until PLL Locks, ("TRUE"/"FALSE")
   )
   port map (
      -- Clock Outputs: 1-bit (each) output: User configurable clock outputs
      CLKOUT0 => ckMul,
      CLKOUT1 => open,
      CLKOUT2 => open,
      CLKOUT3 => open,
      CLKOUT4 => open,
      CLKOUT5 => open,
      -- Feedback Clocks: 1-bit (each) output: Clock feedback ports
      CLKFBOUT => clk_fb, -- 1-bit output: Feedback clock
      -- Status Port: 1-bit (each) output: PLL status ports
      LOCKED => open,     -- 1-bit output: LOCK
      -- Clock Input: 1-bit (each) input: Clock input
      CLKIN1 => clk,     -- 1-bit input: Input clock
      -- Control Ports: 1-bit (each) input: PLL control ports
      PWRDWN => '0',     -- 1-bit input: Power-down
      RST => '0',           -- 1-bit input: Reset
      -- Feedback Clocks: 1-bit (each) input: Clock feedback ports
      CLKFBIN => clk_fb    -- 1-bit input: Feedback clock
   );
	  
end PWM_arch;
