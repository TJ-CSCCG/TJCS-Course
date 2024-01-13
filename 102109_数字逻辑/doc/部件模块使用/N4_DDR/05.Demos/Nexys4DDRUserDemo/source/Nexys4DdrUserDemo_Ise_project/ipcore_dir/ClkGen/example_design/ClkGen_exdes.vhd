-- file: ClkGen_exdes.vhd
-- 
-- (c) Copyright 2008 - 2011 Xilinx, Inc. All rights reserved.
-- 
-- This file contains confidential and proprietary information
-- of Xilinx, Inc. and is protected under U.S. and
-- international copyright and other intellectual property
-- laws.
-- 
-- DISCLAIMER
-- This disclaimer is not a license and does not grant any
-- rights to the materials distributed herewith. Except as
-- otherwise provided in a valid license issued to you by
-- Xilinx, and to the maximum extent permitted by applicable
-- law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
-- WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
-- AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
-- BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
-- INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
-- (2) Xilinx shall not be liable (whether in contract or tort,
-- including negligence, or under any other theory of
-- liability) for any loss or damage of any kind or nature
-- related to, arising under or in connection with these
-- materials, including for any direct, or any indirect,
-- special, incidental, or consequential loss or damage
-- (including loss of data, profits, goodwill, or any type of
-- loss or damage suffered as a result of any action brought
-- by a third party) even if such damage or loss was
-- reasonably foreseeable or Xilinx had been advised of the
-- possibility of the same.
-- 
-- CRITICAL APPLICATIONS
-- Xilinx products are not designed or intended to be fail-
-- safe, or for use in any application requiring fail-safe
-- performance, such as life-support or safety devices or
-- systems, Class III medical devices, nuclear facilities,
-- applications related to the deployment of airbags, or any
-- other applications that could lead to death, personal
-- injury, or severe property or environmental damage
-- (individually and collectively, "Critical
-- Applications"). Customer assumes the sole risk and
-- liability of any use of Xilinx products in Critical
-- Applications, subject only to applicable laws and
-- regulations governing limitations on product liability.
-- 
-- THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
-- PART OF THIS FILE AT ALL TIMES.
-- 

------------------------------------------------------------------------------
-- Clocking wizard example design
------------------------------------------------------------------------------
-- This example design instantiates the created clocking network, where each
--   output clock drives a counter. The high bit of each counter is ported.
------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;
use ieee.numeric_std.all;

library unisim;
use unisim.vcomponents.all;

entity ClkGen_exdes is
generic (
  TCQ               : in time := 100 ps);
port
 (-- Clock in ports
  CLK_IN1           : in  std_logic;
  -- Reset that only drives logic in example design
  COUNTER_RESET     : in  std_logic;
  CLK_OUT           : out std_logic_vector(2 downto 1) ;
  -- High bits of counters driven by clocks
  COUNT             : out std_logic_vector(2 downto 1);
  -- Status and control signals
  RESET             : in  std_logic;
  LOCKED            : out std_logic
 );
end ClkGen_exdes;

architecture xilinx of ClkGen_exdes is

  -- Parameters for the counters
  ---------------------------------
  -- Counter width
  constant C_W        : integer := 16;

  -- Number of counters
  constant NUM_C      : integer := 2;
  -- Array typedef
  type ctrarr is array (1 to NUM_C) of std_logic_vector(C_W-1 downto 0);

  -- When the clock goes out of lock, reset the counters
  signal   locked_int : std_logic;
  signal   reset_int  : std_logic                     := '0';
  -- Declare the clocks and counters
  signal   clk        : std_logic_vector(NUM_C downto 1);
 
  signal   clk_int    : std_logic_vector(NUM_C downto 1);
  signal   counter    : ctrarr := (( others => (others => '0')));
  signal rst_sync : std_logic_vector(NUM_C downto 1);
  signal rst_sync_int : std_logic_vector(NUM_C downto 1);
  signal rst_sync_int1 : std_logic_vector(NUM_C downto 1);
  signal rst_sync_int2 : std_logic_vector(NUM_C downto 1);


component ClkGen is
port
 (-- Clock in ports
  clk_100MHz_i           : in     std_logic;
  -- Clock out ports
  clk_100MHz_o          : out    std_logic;
  clk_200MHz_o          : out    std_logic;
  -- Status and control signals
  reset_i             : in     std_logic;
  locked_o            : out    std_logic
 );
end component;

begin
  -- Alias output to internally used signal
  LOCKED    <= locked_int;

  -- When the clock goes out of lock, reset the counters
  reset_int <= (not locked_int) or RESET or COUNTER_RESET;


  counters_1: for count_gen in 1 to NUM_C generate begin
 process (clk(count_gen), reset_int) begin
   if (reset_int = '1') then
       rst_sync(count_gen) <= '1';
       rst_sync_int(count_gen) <= '1';
       rst_sync_int1(count_gen) <= '1';
       rst_sync_int2(count_gen) <= '1';
   elsif (clk(count_gen) 'event and clk(count_gen)='1') then
       rst_sync(count_gen) <= '0';
       rst_sync_int(count_gen) <= rst_sync(count_gen);
       rst_sync_int1(count_gen) <= rst_sync_int(count_gen);
       rst_sync_int2(count_gen) <= rst_sync_int1(count_gen);
   end if;
 end process;
end generate counters_1;


  -- Instantiation of the clocking network
  ----------------------------------------
  clknetwork : ClkGen
  port map
   (-- Clock in ports
    clk_100MHz_i            => CLK_IN1,
    -- Clock out ports
    clk_100MHz_o           => clk_int(1),
    clk_200MHz_o           => clk_int(2),
    -- Status and control signals
    reset_i              => RESET,
    locked_o             => locked_int);


  gen_outclk_oddr: 
  for clk_out_pins in 1 to NUM_C generate 
  begin
  clkout_oddr : ODDR port map
    (Q  => CLK_OUT(clk_out_pins),
     C  => clk(clk_out_pins),
     CE => '1',
     D1 => '1',
     D2 => '0',
     R  => '0',
     S  => '0');
   end generate;

  -- Connect the output clocks to the design
  -------------------------------------------
  clk(1) <= clk_int(1);
  clk(2) <= clk_int(2);

  -- Output clock sampling
  -------------------------------------
  counters: for count_gen in 1 to NUM_C generate begin
    process (clk(count_gen), rst_sync_int2(count_gen)) begin
        if (rst_sync_int2(count_gen) = '1') then
          counter(count_gen) <= (others => '0') after TCQ;
        elsif (rising_edge (clk(count_gen))) then
          counter(count_gen) <= counter(count_gen) + 1 after TCQ;
        end if;
    end process;

    -- alias the high bit of each counter to the corresponding
    --   bit in the output bus
    COUNT(count_gen) <= counter(count_gen)(C_W-1);

  end generate counters;



end xilinx;
