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
-- User entered comments
------------------------------------------------------------------------------
-- None
--
------------------------------------------------------------------------------
-- "Output    Output      Phase     Duty      Pk-to-Pk        Phase"
-- "Clock    Freq (MHz) (degrees) Cycle (%) Jitter (ps)  Error (ps)"
------------------------------------------------------------------------------
-- CLK_OUT1___100.000______0.000______50.0______130.958_____98.575
-- CLK_OUT2___200.000______0.000______50.0______114.829_____98.575
--
------------------------------------------------------------------------------
-- "Input Clock   Freq (MHz)    Input Jitter (UI)"
------------------------------------------------------------------------------
-- __primary_________100.000____________0.010


-- The following code must appear in the VHDL architecture header:
------------- Begin Cut here for COMPONENT Declaration ------ COMP_TAG
component ClkGen
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

-- COMP_TAG_END ------ End COMPONENT Declaration ------------
-- The following code must appear in the VHDL architecture
-- body. Substitute your own instance name and net names.
------------- Begin Cut here for INSTANTIATION Template ----- INST_TAG
your_instance_name : ClkGen
  port map
   (-- Clock in ports
    clk_100MHz_i => clk_100MHz_i,
    -- Clock out ports
    clk_100MHz_o => clk_100MHz_o,
    clk_200MHz_o => clk_200MHz_o,
    -- Status and control signals
    reset_i  => reset_i,
    locked_o => locked_o);
-- INST_TAG_END ------ End INSTANTIATION Template ------------
