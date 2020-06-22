--*****************************************************************************
-- (c) Copyright 2009 - 2012 Xilinx, Inc. All rights reserved.
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
--*****************************************************************************
--   ____  ____
--  /   /\/   /
-- /___/  \  /    Vendor             : Xilinx
-- \   \   \/     Version            : 1.9
--  \   \         Application        : MIG
--  /   /         Filename           : example_top.vhd
-- /___/   /\     Date Last Modified : $Date: 2011/06/02 08:35:03 $
-- \   \  /  \    Date Created       : Wed Feb 01 2012
--  \___\/\___\
--
-- Device           : 7 Series
-- Design Name      : DDR2 SDRAM
-- Purpose          :
--   Top-level  module. This module serves both as an example,
--   and allows the user to synthesize a self-contained design,
--   which they can be used to test their hardware.
--   In addition to the memory controller, the module instantiates:
--     1. Synthesizable testbench - used to model user's backend logic
--        and generate different traffic patterns
-- Reference        :
-- Revision History :
--*****************************************************************************

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


entity example_top is
  generic
  (

   --***************************************************************************
   -- Traffic Gen related parameters
   --***************************************************************************
   BL_WIDTH              : integer := 10;
   PORT_MODE             : string  := "BI_MODE";
   DATA_MODE             : std_logic_vector(3 downto 0) := "0010";
   ADDR_MODE             : std_logic_vector(3 downto 0) := "0011";
   TST_MEM_INSTR_MODE    : string  := "R_W_INSTR_MODE";
   EYE_TEST              : string  := "FALSE";
                                     -- set EYE_TEST = "TRUE" to probe memory
                                     -- signals. Traffic Generator will only
                                     -- write to one single location and no
                                     -- read transactions will be generated.
   DATA_PATTERN          : string  := "DGEN_ALL";
                                      -- For small devices, choose one only.
                                      -- For large device, choose "DGEN_ALL"
                                      -- "DGEN_HAMMER", "DGEN_WALKING1",
                                      -- "DGEN_WALKING0","DGEN_ADDR","
                                      -- "DGEN_NEIGHBOR","DGEN_PRBS","DGEN_ALL"
   CMD_PATTERN           : string  := "CGEN_ALL";
                                      -- "CGEN_PRBS","CGEN_FIXED","CGEN_BRAM",
                                      -- "CGEN_SEQUENTIAL", "CGEN_ALL"
   BEGIN_ADDRESS         : std_logic_vector(31 downto 0) := X"00000000";
   END_ADDRESS           : std_logic_vector(31 downto 0) := X"00ffffff";
   MEM_ADDR_ORDER
     : string  := "TG_TEST";
   PRBS_EADDR_MASK_POS   : std_logic_vector(31 downto 0) := X"ff000000";
   CMD_WDT               : std_logic_vector(31 downto 0) := X"000003ff";
   WR_WDT                : std_logic_vector(31 downto 0) := X"00001fff";
   RD_WDT                : std_logic_vector(31 downto 0) := X"000003ff";
   SEL_VICTIM_LINE       : integer := 0;

   --***************************************************************************
   -- The following parameters refer to width of various ports
   --***************************************************************************
   BANK_WIDTH            : integer := 3;
                                     -- # of memory Bank Address bits.
   CK_WIDTH              : integer := 1;
                                     -- # of CK/CK# outputs to memory.
   COL_WIDTH             : integer := 10;
                                     -- # of memory Column Address bits.
   CS_WIDTH              : integer := 1;
                                     -- # of unique CS outputs to memory.
   nCS_PER_RANK          : integer := 1;
                                     -- # of unique CS outputs per rank for phy
   CKE_WIDTH             : integer := 1;
                                     -- # of CKE outputs to memory.
   DATA_BUF_ADDR_WIDTH   : integer := 5;
   DQ_CNT_WIDTH          : integer := 4;
                                     -- = ceil(log2(DQ_WIDTH))
   DQ_PER_DM             : integer := 8;
   DM_WIDTH              : integer := 2;
                                     -- # of DM (data mask)
   DQ_WIDTH              : integer := 16;
                                     -- # of DQ (data)
   DQS_WIDTH             : integer := 2;
   DQS_CNT_WIDTH         : integer := 1;
                                     -- = ceil(log2(DQS_WIDTH))
   DRAM_WIDTH            : integer := 8;
                                     -- # of DQ per DQS
   ECC                   : string  := "OFF";
   nBANK_MACHS           : integer := 4;
   RANKS                 : integer := 1;
                                     -- # of Ranks.
   ODT_WIDTH             : integer := 1;
                                     -- # of ODT outputs to memory.
   ROW_WIDTH             : integer := 13;
                                     -- # of memory Row Address bits.
   ADDR_WIDTH            : integer := 27;
                                     -- # = RANK_WIDTH + BANK_WIDTH
                                     --     + ROW_WIDTH + COL_WIDTH;
                                     -- Chip Select is always tied to low for
                                     -- single rank devices
   USE_CS_PORT          : integer := 1;
                                     -- # = 1, When Chip Select (CS#) output is enabled
                                     --   = 0, When Chip Select (CS#) output is disabled
                                     -- If CS_N disabled, user must connect
                                     -- DRAM CS_N input(s) to ground
   USE_DM_PORT           : integer := 1;
                                     -- # = 1, When Data Mask option is enabled
                                     --   = 0, When Data Mask option is disbaled
                                     -- When Data Mask option is disabled in
                                     -- MIG Controller Options page, the logic
                                     -- related to Data Mask should not get
                                     -- synthesized
   USE_ODT_PORT          : integer := 1;
                                     -- # = 1, When ODT output is enabled
                                     --   = 0, When ODT output is disabled
   PHY_CONTROL_MASTER_BANK : integer := 0;
                                     -- The bank index where master PHY_CONTROL resides,
                                     -- equal to the PLL residing bank
   MEM_DENSITY             : string  := "1Gb";
                                     -- Indicates the density of the Memory part
                                     -- Added for the sake of Vivado simulations
   MEM_SPEEDGRADE          : string  := "25E";
                                     -- Indicates the Speed grade of Memory Part
                                     -- Added for the sake of Vivado simulations
   MEM_DEVICE_WIDTH        : integer := 16;
                                     -- Indicates the device width of the Memory Part
                                     -- Added for the sake of Vivado simulations

   --***************************************************************************
   -- The following parameters are mode register settings
   --***************************************************************************
   AL                    : string  := "0";
                                     -- DDR3 SDRAM:
                                     -- Additive Latency (Mode Register 1).
                                     -- # = "0", "CL-1", "CL-2".
                                     -- DDR2 SDRAM:
                                     -- Additive Latency (Extended Mode Register).
   nAL                   : integer := 0;
                                     -- # Additive Latency in number of clock
                                     -- cycles.
   BURST_MODE            : string  := "8";
                                     -- DDR3 SDRAM:
                                     -- Burst Length (Mode Register 0).
                                     -- # = "8", "4", "OTF".
                                     -- DDR2 SDRAM:
                                     -- Burst Length (Mode Register).
                                     -- # = "8", "4".
   BURST_TYPE            : string  := "SEQ";
                                     -- DDR3 SDRAM: Burst Type (Mode Register 0).
                                     -- DDR2 SDRAM: Burst Type (Mode Register).
                                     -- # = "SEQ" - (Sequential),
                                     --   = "INT" - (Interleaved).
   CL                    : integer := 5;
                                     -- in number of clock cycles
                                     -- DDR3 SDRAM: CAS Latency (Mode Register 0).
                                     -- DDR2 SDRAM: CAS Latency (Mode Register).
   OUTPUT_DRV            : string  := "HIGH";
                                     -- Output Drive Strength (Extended Mode Register).
                                     -- # = "HIGH" - FULL,
                                     --   = "LOW" - REDUCED.
   RTT_NOM               : string  := "50";
                                     -- RTT (Nominal) (Extended Mode Register).
                                     --   = "150" - 150 Ohms,
                                     --   = "75" - 75 Ohms,
                                     --   = "50" - 50 Ohms.
   ADDR_CMD_MODE         : string  := "1T" ;
                                     -- # = "1T", "2T".
   REG_CTRL              : string  := "OFF";
                                     -- # = "ON" - RDIMMs,
                                     --   = "OFF" - Components, SODIMMs, UDIMMs.
   
   --***************************************************************************
   -- The following parameters are multiplier and divisor factors for PLLE2.
   -- Based on the selected design frequency these parameters vary.
   --***************************************************************************
   CLKIN_PERIOD          : integer := 4999;
                                     -- Input Clock Period
   CLKFBOUT_MULT         : integer := 6;
                                     -- write PLL VCO multiplier
   DIVCLK_DIVIDE         : integer := 1;
                                     -- write PLL VCO divisor
   CLKOUT0_PHASE         : real    := 0.0;
                                     -- Phase for PLL output clock (CLKOUT0)
   CLKOUT0_DIVIDE        : integer := 2;
                                     -- VCO output divisor for PLL output clock (CLKOUT0)
   CLKOUT1_DIVIDE        : integer := 4;
                                     -- VCO output divisor for PLL output clock (CLKOUT1)
   CLKOUT2_DIVIDE        : integer := 64;
                                     -- VCO output divisor for PLL output clock (CLKOUT2)
   CLKOUT3_DIVIDE        : integer := 16;
                                     -- VCO output divisor for PLL output clock (CLKOUT3)

   --***************************************************************************
   -- Memory Timing Parameters. These parameters varies based on the selected
   -- memory part.
   --***************************************************************************
   tCKE                  : integer := 7500;
                                     -- memory tCKE paramter in pS
   tFAW                  : integer := 45000;
                                     -- memory tRAW paramter in pS.
   tRAS                  : integer := 40000;
                                     -- memory tRAS paramter in pS.
   tRCD                  : integer := 15000;
                                     -- memory tRCD paramter in pS.
   tREFI                 : integer := 7800000;
                                     -- memory tREFI paramter in pS.
   tRFC                  : integer := 127500;
                                     -- memory tRFC paramter in pS.
   tRP                   : integer := 12500;
                                     -- memory tRP paramter in pS.
   tRRD                  : integer := 10000;
                                     -- memory tRRD paramter in pS.
   tRTP                  : integer := 7500;
                                     -- memory tRTP paramter in pS.
   tWTR                  : integer := 7500;
                                     -- memory tWTR paramter in pS.
   tZQI                  : integer := 128000000;
                                     -- memory tZQI paramter in nS.
   tZQCS                 : integer := 64;
                                     -- memory tZQCS paramter in clock cycles.

   --***************************************************************************
   -- Simulation parameters
   --***************************************************************************
   SIM_BYPASS_INIT_CAL   : string  := "OFF";
                                     -- # = "OFF" -  Complete memory init &
                                     --              calibration sequence
                                     -- # = "SKIP" - Not supported
                                     -- # = "FAST" - Complete memory init & use
                                     --              abbreviated calib sequence

   SIMULATION            : string  := "FALSE";
                                     -- Should be TRUE during design simulations and
                                     -- FALSE during implementations

   --***************************************************************************
   -- The following parameters varies based on the pin out entered in MIG GUI.
   -- Do not change any of these parameters directly by editing the RTL.
   -- Any changes required should be done through GUI and the design regenerated.
   --***************************************************************************
   BYTE_LANES_B0         : std_logic_vector(3 downto 0) := "1111";
                                     -- Byte lanes used in an IO column.
   BYTE_LANES_B1         : std_logic_vector(3 downto 0) := "0000";
                                     -- Byte lanes used in an IO column.
   BYTE_LANES_B2         : std_logic_vector(3 downto 0) := "0000";
                                     -- Byte lanes used in an IO column.
   BYTE_LANES_B3         : std_logic_vector(3 downto 0) := "0000";
                                     -- Byte lanes used in an IO column.
   BYTE_LANES_B4         : std_logic_vector(3 downto 0) := "0000";
                                     -- Byte lanes used in an IO column.
   DATA_CTL_B0           : std_logic_vector(3 downto 0) := "0101";
                                     -- Indicates Byte lane is data byte lane
                                     -- or control Byte lane. '1' in a bit
                                     -- position indicates a data byte lane and
                                     -- a '0' indicates a control byte lane
   DATA_CTL_B1           : std_logic_vector(3 downto 0) := "0000";
                                     -- Indicates Byte lane is data byte lane
                                     -- or control Byte lane. '1' in a bit
                                     -- position indicates a data byte lane and
                                     -- a '0' indicates a control byte lane
   DATA_CTL_B2           : std_logic_vector(3 downto 0) := "0000";
                                     -- Indicates Byte lane is data byte lane
                                     -- or control Byte lane. '1' in a bit
                                     -- position indicates a data byte lane and
                                     -- a '0' indicates a control byte lane
   DATA_CTL_B3           : std_logic_vector(3 downto 0) := "0000";
                                     -- Indicates Byte lane is data byte lane
                                     -- or control Byte lane. '1' in a bit
                                     -- position indicates a data byte lane and
                                     -- a '0' indicates a control byte lane
   DATA_CTL_B4           : std_logic_vector(3 downto 0) := "0000";
                                     -- Indicates Byte lane is data byte lane
                                     -- or control Byte lane. '1' in a bit
                                     -- position indicates a data byte lane and
                                     -- a '0' indicates a control byte lane
   PHY_0_BITLANES        : std_logic_vector(47 downto 0) := X"FFC3F7FFF3FE";
   PHY_1_BITLANES        : std_logic_vector(47 downto 0) := X"000000000000";
   PHY_2_BITLANES        : std_logic_vector(47 downto 0) := X"000000000000";

   -- control/address/data pin mapping parameters
   CK_BYTE_MAP
     : std_logic_vector(143 downto 0) := X"000000000000000000000000000000000003";
   ADDR_MAP
     : std_logic_vector(191 downto 0) := X"00000000001003301A01903203A034018036012011017015";
   BANK_MAP   : std_logic_vector(35 downto 0) := X"01301601B";
   CAS_MAP    : std_logic_vector(11 downto 0) := X"039";
   CKE_ODT_BYTE_MAP : std_logic_vector(7 downto 0) := X"00";
   CKE_MAP    : std_logic_vector(95 downto 0) := X"000000000000000000000038";
   ODT_MAP    : std_logic_vector(95 downto 0) := X"000000000000000000000035";
   CS_MAP     : std_logic_vector(119 downto 0) := X"000000000000000000000000000037";
   PARITY_MAP : std_logic_vector(11 downto 0) := X"000";
   RAS_MAP    : std_logic_vector(11 downto 0) := X"014";
   WE_MAP     : std_logic_vector(11 downto 0) := X"03B";
   DQS_BYTE_MAP
     : std_logic_vector(143 downto 0) := X"000000000000000000000000000000000200";
   DATA0_MAP  : std_logic_vector(95 downto 0) := X"008004009007005001006003";
   DATA1_MAP  : std_logic_vector(95 downto 0) := X"022028020024027025026021";
   DATA2_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA3_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA4_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA5_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA6_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA7_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA8_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA9_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA10_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA11_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA12_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA13_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA14_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA15_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA16_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA17_MAP : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   MASK0_MAP  : std_logic_vector(107 downto 0) := X"000000000000000000000029002";
   MASK1_MAP  : std_logic_vector(107 downto 0) := X"000000000000000000000000000";

   SLOT_0_CONFIG         : std_logic_vector(7 downto 0) := "00000001";
                                     -- Mapping of Ranks.
   SLOT_1_CONFIG         : std_logic_vector(7 downto 0) := "00000000";
                                     -- Mapping of Ranks.

   --***************************************************************************
   -- IODELAY and PHY related parameters
   --***************************************************************************
   IODELAY_HP_MODE       : string  := "ON";
                                     -- to phy_top
   IBUF_LPWR_MODE        : string  := "OFF";
                                     -- to phy_top
   DATA_IO_IDLE_PWRDWN   : string  := "ON";
                                     -- # = "ON", "OFF"
   BANK_TYPE             : string  := "HR_IO";
                                     -- # = "HP_IO", "HPL_IO", "HR_IO", "HRL_IO"
   DATA_IO_PRIM_TYPE     : string  := "HR_LP";
                                     -- # = "HP_LP", "HR_LP", "DEFAULT"
   CKE_ODT_AUX           : string  := "FALSE";
   USER_REFRESH          : string  := "OFF";
   WRLVL                 : string  := "OFF";
                                     -- # = "ON" - DDR3 SDRAM
                                     --   = "OFF" - DDR2 SDRAM.
   ORDERING              : string  := "STRICT";
                                     -- # = "NORM", "STRICT", "RELAXED".
   CALIB_ROW_ADD         : std_logic_vector(15 downto 0) := X"0000";
                                     -- Calibration row address will be used for
                                     -- calibration read and write operations
   CALIB_COL_ADD         : std_logic_vector(11 downto 0) := X"000";
                                     -- Calibration column address will be used for
                                     -- calibration read and write operations
   CALIB_BA_ADD          : std_logic_vector(2 downto 0) := "000";
                                     -- Calibration bank address will be used for
                                     -- calibration read and write operations
   TCQ                   : integer := 100;
   IODELAY_GRP           : string  := "IODELAY_MIG";
                                     -- It is associated to a set of IODELAYs with
                                     -- an IDELAYCTRL that have same IODELAY CONTROLLER
                                     -- clock frequency.
   SYSCLK_TYPE           : string  := "NO_BUFFER";
                                     -- System clock type DIFFERENTIAL, SINGLE_ENDED,
                                     -- NO_BUFFER
   REFCLK_TYPE           : string  := "USE_SYSTEM_CLOCK";
                                     -- Reference clock type DIFFERENTIAL, SINGLE_ENDED
                                     -- NO_BUFFER, USE_SYSTEM_CLOCK
   SYS_RST_PORT          : string  := "FALSE";
                                     -- "TRUE" - if pin is selected for sys_rst
                                     --          and IBUF will be instantiated.
                                     -- "FALSE" - if pin is not selected for sys_rst
      
   DRAM_TYPE             : string  := "DDR2";
   CAL_WIDTH             : string  := "HALF";
   STARVE_LIMIT          : integer := 2;
                                     -- # = 2,3,4.

   --***************************************************************************
   -- Referece clock frequency parameters
   --***************************************************************************
   REFCLK_FREQ           : real    := 200.0;
                                     -- IODELAYCTRL reference clock frequency
   DIFF_TERM_REFCLK      : string  := "TRUE";
                                     -- Differential Termination for idelay
                                     -- reference clock input pins
   --***************************************************************************
   -- System clock frequency parameters
   --***************************************************************************
   tCK                   : integer := 3333;
                                     -- memory tCK paramter.
                                     -- # = Clock Period in pS.
   nCK_PER_CLK           : integer := 4;
                                     -- # of memory CKs per fabric CLK
   DIFF_TERM_SYSCLK      : string  := "TRUE";
                                     -- Differential Termination for System
                                     -- clock input pins

   --***************************************************************************
   -- Debug parameters
   --***************************************************************************
   DEBUG_PORT            : string  := "OFF";
                                     -- # = "ON" Enable debug signals/controls.
                                     --   = "OFF" Disable debug signals/controls.

   --***************************************************************************
   -- Temparature monitor parameter
   --***************************************************************************
   TEMP_MON_CONTROL         : string  := "EXTERNAL";
                                     -- # = "INTERNAL", "EXTERNAL"
      
   RST_ACT_LOW           : integer := 1
                                     -- =1 for active low reset,
                                     -- =0 for active high.
   );
  port
  (

   -- Inouts
   ddr2_dq                        : inout std_logic_vector(DQ_WIDTH-1 downto 0);
   ddr2_dqs_p                     : inout std_logic_vector(DQS_WIDTH-1 downto 0);
   ddr2_dqs_n                     : inout std_logic_vector(DQS_WIDTH-1 downto 0);

   -- Outputs
   ddr2_addr                      : out   std_logic_vector(ROW_WIDTH-1 downto 0);
   ddr2_ba                        : out   std_logic_vector(BANK_WIDTH-1 downto 0);
   ddr2_ras_n                     : out   std_logic;
   ddr2_cas_n                     : out   std_logic;
   ddr2_we_n                      : out   std_logic;
   ddr2_ck_p                      : out   std_logic_vector(CK_WIDTH-1 downto 0);
   ddr2_ck_n                      : out   std_logic_vector(CK_WIDTH-1 downto 0);
   ddr2_cke                       : out   std_logic_vector(CKE_WIDTH-1 downto 0);
   ddr2_cs_n                      : out   std_logic_vector(CS_WIDTH*nCS_PER_RANK-1 downto 0);
   ddr2_dm                        : out   std_logic_vector(DM_WIDTH-1 downto 0);
   ddr2_odt                       : out   std_logic_vector(ODT_WIDTH-1 downto 0);

   -- Inputs
   -- Single-ended system clock
   sys_clk_i                      : in    std_logic;
   
   
   tg_compare_error              : out std_logic;
   init_calib_complete           : out std_logic;
   device_temp_i                 : in  std_logic_vector(11 downto 0);
                      -- The 12 MSB bits of the temperature sensor transfer
                      -- function need to be connected to this port. This port
                      -- will be synchronized w.r.t. to fabric clock internally.
      

   -- System reset - Default polarity of sys_rst pin is Active Low.
   -- System reset polarity will change based on the option 
   -- selected in GUI.
      sys_rst                     : in    std_logic
 );

end entity example_top;

architecture arch_example_top of example_top is


  -- clogb2 function - ceiling of log base 2
  function clogb2 (size : integer) return integer is
    variable base : integer := 1;
    variable inp : integer := 0;
  begin
    inp := size - 1;
    while (inp > 1) loop
      inp := inp/2 ;
      base := base + 1;
    end loop;
    return base;
  end function;function STR_TO_INT(BM : string) return integer is
  begin
   if(BM = "8") then
     return 8;
   elsif(BM = "4") then
     return 4;
   else
     return 0;
   end if;
  end function;

  constant DATA_WIDTH            : integer := 16;

  function ECCWIDTH return integer is
  begin
    if(ECC = "OFF") then
      return 0;
    else
      if(DATA_WIDTH <= 4) then
        return 4;
      elsif(DATA_WIDTH <= 10) then
        return 5;
      elsif(DATA_WIDTH <= 26) then
        return 6;
      elsif(DATA_WIDTH <= 57) then
        return 7;
      elsif(DATA_WIDTH <= 120) then
        return 8;
      elsif(DATA_WIDTH <= 247) then
        return 9;
      else
        return 10;
      end if;
    end if;
  end function;

  constant RANK_WIDTH : integer := clogb2(RANKS);

  function XWIDTH return integer is
  begin
    if(CS_WIDTH = 1) then
      return 0;
    else
      return RANK_WIDTH;
    end if;
  end function;
  


  constant CMD_PIPE_PLUS1        : string  := "ON";
                                     -- add pipeline stage between MC and PHY
  constant ECC_WIDTH             : integer := ECCWIDTH;
  constant ECC_TEST              : string  := "OFF";
  constant DATA_BUF_OFFSET_WIDTH : integer := 1;
  constant MC_ERR_ADDR_WIDTH     : integer := XWIDTH + BANK_WIDTH + ROW_WIDTH
                                          + COL_WIDTH + DATA_BUF_OFFSET_WIDTH;
  constant tPRDI                 : integer := 1000000;
                                     -- memory tPRDI paramter in pS.
  constant PAYLOAD_WIDTH         : integer := DATA_WIDTH;
  constant BURST_LENGTH          : integer := STR_TO_INT(BURST_MODE);
  constant APP_DATA_WIDTH        : integer := 2 * nCK_PER_CLK * PAYLOAD_WIDTH;
  constant APP_MASK_WIDTH        : integer := APP_DATA_WIDTH / 8;

  --***************************************************************************
  -- Traffic Gen related parameters (derived)
  --***************************************************************************
  constant  TG_ADDR_WIDTH        : integer := XWIDTH + BANK_WIDTH + ROW_WIDTH + COL_WIDTH;
  constant MASK_SIZE             : integer := DATA_WIDTH/8;
      

-- Start of User Design top component

  component ddr
    generic(
      
     
      BANK_WIDTH            : integer;
      CK_WIDTH              : integer;
      COL_WIDTH             : integer;
      CS_WIDTH              : integer;
      nCS_PER_RANK          : integer;
      CKE_WIDTH             : integer;
      DATA_BUF_ADDR_WIDTH   : integer;
      DQ_CNT_WIDTH          : integer;
      DQ_PER_DM             : integer;
      DM_WIDTH              : integer;
      DQ_WIDTH              : integer;
      DQS_WIDTH             : integer;
      DQS_CNT_WIDTH         : integer;
      DRAM_WIDTH            : integer;
      ECC                   : string;
      DATA_WIDTH            : integer;
      ECC_TEST              : string;
      PAYLOAD_WIDTH         : integer;
      ECC_WIDTH             : integer;
      MC_ERR_ADDR_WIDTH     : integer;
      nBANK_MACHS           : integer;
      RANKS                 : integer;
      ODT_WIDTH             : integer;
      ROW_WIDTH             : integer;
      ADDR_WIDTH            : integer;
      USE_CS_PORT           : integer;
      USE_DM_PORT           : integer;
      USE_ODT_PORT          : integer;
      PHY_CONTROL_MASTER_BANK : integer;
      AL                    : string;
      nAL                   : integer;
      BURST_MODE            : string;
      BURST_TYPE            : string;
      CL                    : integer;
      OUTPUT_DRV            : string;
      RTT_NOM               : string;
      ADDR_CMD_MODE         : string;
      REG_CTRL              : string;
      CLKIN_PERIOD          : integer;
      CLKFBOUT_MULT         : integer;
      DIVCLK_DIVIDE         : integer;
      CLKOUT0_PHASE         : real;
      CLKOUT0_DIVIDE        : integer;
      CLKOUT1_DIVIDE        : integer;
      CLKOUT2_DIVIDE        : integer;
      CLKOUT3_DIVIDE        : integer;
      tCKE                  : integer;
      tFAW                  : integer;
      tRAS                  : integer;
      tRCD                  : integer;
      tREFI                 : integer;
      tRFC                  : integer;
      tRP                   : integer;
      tRRD                  : integer;
      tRTP                  : integer;
      tWTR                  : integer;
      tZQI                  : integer;
      tZQCS                 : integer;
      tPRDI                 : integer;
      SIM_BYPASS_INIT_CAL   : string;
      SIMULATION            : string;
      BYTE_LANES_B0         : std_logic_vector(3 downto 0);
      BYTE_LANES_B1         : std_logic_vector(3 downto 0);
      BYTE_LANES_B2         : std_logic_vector(3 downto 0);
      BYTE_LANES_B3         : std_logic_vector(3 downto 0);
      BYTE_LANES_B4         : std_logic_vector(3 downto 0);
      DATA_CTL_B0           : std_logic_vector(3 downto 0);
      DATA_CTL_B1           : std_logic_vector(3 downto 0);
      DATA_CTL_B2           : std_logic_vector(3 downto 0);
      DATA_CTL_B3           : std_logic_vector(3 downto 0);
      DATA_CTL_B4           : std_logic_vector(3 downto 0);
      PHY_0_BITLANES        : std_logic_vector(47 downto 0);
      PHY_1_BITLANES        : std_logic_vector(47 downto 0);
      PHY_2_BITLANES        : std_logic_vector(47 downto 0);
      CK_BYTE_MAP           : std_logic_vector(143 downto 0);
      ADDR_MAP              : std_logic_vector(191 downto 0);
      BANK_MAP              : std_logic_vector(35 downto 0);
      CAS_MAP               : std_logic_vector(11 downto 0);
      CKE_ODT_BYTE_MAP      : std_logic_vector(7 downto 0);
      CKE_MAP               : std_logic_vector(95 downto 0);
      ODT_MAP               : std_logic_vector(95 downto 0);
      CS_MAP                : std_logic_vector(119 downto 0);
      PARITY_MAP            : std_logic_vector(11 downto 0);
      RAS_MAP               : std_logic_vector(11 downto 0);
      WE_MAP                : std_logic_vector(11 downto 0);
      DQS_BYTE_MAP          : std_logic_vector(143 downto 0);
      DATA0_MAP             : std_logic_vector(95 downto 0);
      DATA1_MAP             : std_logic_vector(95 downto 0);
      DATA2_MAP             : std_logic_vector(95 downto 0);
      DATA3_MAP             : std_logic_vector(95 downto 0);
      DATA4_MAP             : std_logic_vector(95 downto 0);
      DATA5_MAP             : std_logic_vector(95 downto 0);
      DATA6_MAP             : std_logic_vector(95 downto 0);
      DATA7_MAP             : std_logic_vector(95 downto 0);
      DATA8_MAP             : std_logic_vector(95 downto 0);
      DATA9_MAP             : std_logic_vector(95 downto 0);
      DATA10_MAP            : std_logic_vector(95 downto 0);
      DATA11_MAP            : std_logic_vector(95 downto 0);
      DATA12_MAP            : std_logic_vector(95 downto 0);
      DATA13_MAP            : std_logic_vector(95 downto 0);
      DATA14_MAP            : std_logic_vector(95 downto 0);
      DATA15_MAP            : std_logic_vector(95 downto 0);
      DATA16_MAP            : std_logic_vector(95 downto 0);
      DATA17_MAP            : std_logic_vector(95 downto 0);
      MASK0_MAP             : std_logic_vector(107 downto 0);
      MASK1_MAP             : std_logic_vector(107 downto 0);
      SLOT_0_CONFIG         : std_logic_vector(7 downto 0);
      SLOT_1_CONFIG         : std_logic_vector(7 downto 0);
      MEM_ADDR_ORDER        : string;
      IODELAY_HP_MODE       : string;
      IBUF_LPWR_MODE        : string;
      DATA_IO_IDLE_PWRDWN   : string;
      BANK_TYPE             : string;
      DATA_IO_PRIM_TYPE     : string;
      CKE_ODT_AUX           : string;
      USER_REFRESH          : string;
      WRLVL                 : string;
      ORDERING              : string;
      CALIB_ROW_ADD         : std_logic_vector(15 downto 0);
      CALIB_COL_ADD         : std_logic_vector(11 downto 0);
      CALIB_BA_ADD          : std_logic_vector(2 downto 0);
      TCQ                   : integer;
      CMD_PIPE_PLUS1        : string;
      tCK                   : integer;
      nCK_PER_CLK           : integer;
      DIFF_TERM_SYSCLK      : string;
      DEBUG_PORT            : string;
      TEMP_MON_CONTROL      : string;
      
     
      IODELAY_GRP           : string;
      SYSCLK_TYPE           : string;
      REFCLK_TYPE           : string;
      SYS_RST_PORT          : string;
      REFCLK_FREQ           : real;
      DIFF_TERM_REFCLK      : string;
      
      DRAM_TYPE             : string;
      CAL_WIDTH             : string;
      STARVE_LIMIT          : integer;
      
      
      RST_ACT_LOW           : integer
      );
    port(
      
       
      ddr2_dq       : inout std_logic_vector(DQ_WIDTH-1 downto 0);
      ddr2_dqs_p    : inout std_logic_vector(DQS_WIDTH-1 downto 0);
      ddr2_dqs_n    : inout std_logic_vector(DQS_WIDTH-1 downto 0);

      ddr2_addr     : out   std_logic_vector(ROW_WIDTH-1 downto 0);
      ddr2_ba       : out   std_logic_vector(BANK_WIDTH-1 downto 0);
      ddr2_ras_n    : out   std_logic;
      ddr2_cas_n    : out   std_logic;
      ddr2_we_n     : out   std_logic;
      ddr2_ck_p     : out   std_logic_vector(CK_WIDTH-1 downto 0);
      ddr2_ck_n     : out   std_logic_vector(CK_WIDTH-1 downto 0);
      ddr2_cke      : out   std_logic_vector(CKE_WIDTH-1 downto 0);
      
       ddr2_cs_n     : out   std_logic_vector((CS_WIDTH*nCS_PER_RANK)-1 downto 0);
      ddr2_dm       : out   std_logic_vector(DM_WIDTH-1 downto 0);
      ddr2_odt      : out   std_logic_vector(ODT_WIDTH-1 downto 0);
      app_addr                  : in    std_logic_vector(ADDR_WIDTH-1 downto 0);
      app_cmd                   : in    std_logic_vector(2 downto 0);
      app_en                    : in    std_logic;
      app_wdf_data              : in    std_logic_vector((nCK_PER_CLK*2*PAYLOAD_WIDTH)-1 downto 0);
      app_wdf_end               : in    std_logic;
      app_wdf_mask         : in    std_logic_vector((nCK_PER_CLK*2*PAYLOAD_WIDTH)/8-1 downto 0);
      app_wdf_wren              : in    std_logic;
      app_rd_data               : out   std_logic_vector((nCK_PER_CLK*2*PAYLOAD_WIDTH)-1 downto 0);
      app_rd_data_end           : out   std_logic;
      app_rd_data_valid         : out   std_logic;
      app_rdy                   : out   std_logic;
      app_wdf_rdy               : out   std_logic;
      app_sr_req                : in    std_logic;
      app_sr_active             : out   std_logic;
      app_ref_req               : in    std_logic;
      app_ref_ack               : out   std_logic;
      app_zq_req                : in    std_logic;
      app_zq_ack                : out   std_logic;
      ui_clk                    : out   std_logic;
      ui_clk_sync_rst           : out   std_logic;
      init_calib_complete       : out   std_logic;
      
       
      -- System Clock Ports
      sys_clk_i                      : in    std_logic;
      -- Reference Clock Ports
      device_temp_i                            : in    std_logic_vector(11 downto 0);
      
      sys_rst             : in std_logic
      );
  end component ddr;

-- End of User Design top component



  component mig_7series_v1_9_traffic_gen_top
    generic (
      TCQ                      : integer;
      SIMULATION               : string;
      FAMILY                   : string;
      MEM_TYPE                 : string;
      TST_MEM_INSTR_MODE       : string;
      --BL_WIDTH                 : integer;
      nCK_PER_CLK              : integer;
      NUM_DQ_PINS              : integer;
      MEM_BURST_LEN            : integer;
      MEM_COL_WIDTH            : integer;
      ADDR_WIDTH               : integer;
      DATA_WIDTH               : integer;
      DATA_MODE                : std_logic_vector(3 downto 0);
      BEGIN_ADDRESS            : std_logic_vector(31 downto 0);
      END_ADDRESS              : std_logic_vector(31 downto 0);
      PRBS_EADDR_MASK_POS      : std_logic_vector(31 downto 0);
      EYE_TEST                 : string;
      CMD_WDT                  : std_logic_vector(31 downto 0) := X"000003ff";
      WR_WDT                   : std_logic_vector(31 downto 0) := X"00001fff";
      RD_WDT                   : std_logic_vector(31 downto 0) := X"000003ff";
      PORT_MODE                : string;
      DATA_PATTERN             : string;
      CMD_PATTERN              : string
      );
    port (
      clk                    : in   std_logic;
      rst                    : in   std_logic;
      manual_clear_error     : in   std_logic;
      tg_only_rst            : in   std_logic;
      memc_init_done         : in   std_logic;
      memc_cmd_full          : in   std_logic;
      memc_cmd_en            : out  std_logic;
      memc_cmd_instr         : out  std_logic_vector(2 downto 0);
      memc_cmd_bl            : out  std_logic_vector(5 downto 0);
      memc_cmd_addr          : out  std_logic_vector(31 downto 0);
      memc_wr_en             : out  std_logic;
      memc_wr_end            : out  std_logic;
      memc_wr_mask           : out  std_logic_vector(DATA_WIDTH/8-1 downto 0);
      memc_wr_data           : out  std_logic_vector(DATA_WIDTH-1 downto 0);
      memc_wr_full           : in   std_logic;
      memc_rd_en             : out  std_logic;
      memc_rd_data           : in   std_logic_vector(DATA_WIDTH-1 downto 0);
      memc_rd_empty          : in   std_logic;
      qdr_wr_cmd_o           : out  std_logic;
      qdr_rd_cmd_o           : out  std_logic;
      vio_pause_traffic      : in   std_logic;
      vio_modify_enable      : in   std_logic;
      vio_data_mode_value    : in   std_logic_vector(3 downto 0);
      vio_addr_mode_value    : in   std_logic_vector(2 downto 0);
      vio_instr_mode_value   : in   std_logic_vector(3 downto 0);
      vio_bl_mode_value      : in   std_logic_vector(1 downto 0);
      vio_fixed_bl_value     : in   std_logic_vector(9 downto 0);
      vio_fixed_instr_value  : in   std_logic_vector(2 downto 0);
      vio_data_mask_gen      : in   std_logic;
      fixed_addr_i           : in   std_logic_vector(31 downto 0);
      fixed_data_i           : in   std_logic_vector(31 downto 0);
      simple_data0           : in   std_logic_vector(31 downto 0);
      simple_data1           : in   std_logic_vector(31 downto 0);
      simple_data2           : in   std_logic_vector(31 downto 0);
      simple_data3           : in   std_logic_vector(31 downto 0);
      simple_data4           : in   std_logic_vector(31 downto 0);
      simple_data5           : in   std_logic_vector(31 downto 0);
      simple_data6           : in   std_logic_vector(31 downto 0);
      simple_data7           : in   std_logic_vector(31 downto 0);
      wdt_en_i               : in   std_logic;
      bram_cmd_i             : in   std_logic_vector(38 downto 0);
      bram_valid_i           : in   std_logic;
      bram_rdy_o             : out  std_logic;
      cmp_data               : out  std_logic_vector(DATA_WIDTH-1 downto 0);
      cmp_data_valid         : out  std_logic;
      cmp_error              : out  std_logic;
      wr_data_counts         : out   std_logic_vector(47 downto 0);
      rd_data_counts         : out   std_logic_vector(47 downto 0);
      dq_error_bytelane_cmp  : out  std_logic_vector((NUM_DQ_PINS/8)-1 downto 0);
      error                  : out  std_logic;
      error_status           : out  std_logic_vector((64+(2*DATA_WIDTH-1)) downto 0);
      cumlative_dq_lane_error : out  std_logic_vector((NUM_DQ_PINS/8)-1 downto 0);
      cmd_wdt_err_o          : out std_logic;
      wr_wdt_err_o           : out std_logic;
      rd_wdt_err_o           : out std_logic;
      mem_pattern_init_done   : out  std_logic
      );
  end component mig_7series_v1_9_traffic_gen_top;
      

  -- Signal declarations
      
  signal app_ecc_multiple_err        : std_logic_vector(2*nCK_PER_CLK-1 downto 0);
  signal app_addr                    : std_logic_vector(ADDR_WIDTH-1 downto 0);
  signal app_addr_i                  : std_logic_vector(31 downto 0);
  signal app_cmd                     : std_logic_vector(2 downto 0);
  signal app_en                      : std_logic;
  signal app_rdy                     : std_logic;
  signal app_rdy_i                   : std_logic;
  signal app_rd_data                 : std_logic_vector(APP_DATA_WIDTH-1 downto 0);
  signal app_rd_data_end             : std_logic;
  signal app_rd_data_valid           : std_logic;
  signal app_rd_data_valid_i         : std_logic;
  signal app_wdf_data                : std_logic_vector(APP_DATA_WIDTH-1 downto 0);
  signal app_wdf_end                 : std_logic;
  signal app_wdf_mask                : std_logic_vector(APP_MASK_WIDTH-1 downto 0);
  signal app_wdf_rdy                 : std_logic;
  signal app_wdf_rdy_i               : std_logic;
  signal app_sr_req                  : std_logic;
  signal app_sr_active               : std_logic;
  signal app_ref_req                 : std_logic;
  signal app_ref_ack                 : std_logic;
  signal app_zq_req                  : std_logic;
  signal app_zq_ack                  : std_logic;
  signal app_wdf_wren                : std_logic;
  signal error_status                : std_logic_vector(64 + (4*PAYLOAD_WIDTH*nCK_PER_CLK - 1) downto 0);
  signal cumlative_dq_lane_error     : std_logic_vector((PAYLOAD_WIDTH/8)-1 downto 0);
  signal mem_pattern_init_done       : std_logic;
  signal modify_enable_sel           : std_logic;
  signal data_mode_manual_sel        : std_logic_vector(2 downto 0);
  signal addr_mode_manual_sel        : std_logic_vector(2 downto 0);
  signal cmp_data                    : std_logic_vector(PAYLOAD_WIDTH*2*nCK_PER_CLK-1 downto 0);
  signal cmp_data_r                  : std_logic_vector(63 downto 0);
  signal cmp_data_valid              : std_logic;
  signal cmp_data_valid_r            : std_logic;
  signal cmp_error                   : std_logic;
  signal wr_data_counts              : std_logic_vector(47 downto 0);
  signal rd_data_counts              : std_logic_vector(47 downto 0);
  signal dq_error_bytelane_cmp       : std_logic_vector((PAYLOAD_WIDTH/8)-1 downto 0);
  signal init_calib_complete_i       : std_logic;
  signal tg_compare_error_i          : std_logic;
  signal tg_rst                      : std_logic;
  signal po_win_tg_rst               : std_logic;
  signal manual_clear_error          : std_logic;

  signal clk                         : std_logic;
  signal rst                         : std_logic;

  signal vio_modify_enable           : std_logic;
  signal vio_data_mode_value         : std_logic_vector(3 downto 0);
  signal vio_pause_traffic           : std_logic;
  signal vio_addr_mode_value         : std_logic_vector(2 downto 0);
  signal vio_instr_mode_value        : std_logic_vector(3 downto 0);
  signal vio_bl_mode_value           : std_logic_vector(1 downto 0);
  signal vio_fixed_bl_value          : std_logic_vector(BL_WIDTH-1 downto 0);
  signal vio_fixed_instr_value       : std_logic_vector(2 downto 0);
  signal vio_data_mask_gen           : std_logic;
  signal dbg_clear_error             : std_logic;
  signal vio_tg_rst                  : std_logic;
  signal dbg_sel_pi_incdec           : std_logic;
  signal dbg_pi_f_inc                : std_logic;
  signal dbg_pi_f_dec                : std_logic;
  signal dbg_sel_po_incdec           : std_logic;
  signal dbg_po_f_inc                : std_logic;
  signal dbg_po_f_stg23_sel          : std_logic;
  signal dbg_po_f_dec                : std_logic;
  signal all_zeros1                  : std_logic_vector(31 downto 0):= (others => '0');
  signal all_zeros2                  : std_logic_vector(38 downto 0):= (others => '0');
  signal wdt_en_w                    : std_logic;
  signal cmd_wdt_err_w               : std_logic;
  signal wr_wdt_err_w                : std_logic;
  signal rd_wdt_err_w                : std_logic;


begin

--***************************************************************************


  init_calib_complete <= init_calib_complete_i;
  tg_compare_error <= tg_compare_error_i;


  app_rdy_i                   <= not(app_rdy);
  app_wdf_rdy_i               <= not(app_wdf_rdy);
  app_rd_data_valid_i         <= not(app_rd_data_valid);
  app_addr                    <= app_addr_i(ADDR_WIDTH-1 downto 0);
      




      

-- Start of User Design top instance
--***************************************************************************
-- The User design is instantiated below. The memory interface ports are
-- connected to the top-level and the application interface ports are
-- connected to the traffic generator module. This provides a reference
-- for connecting the memory controller to system.
--***************************************************************************

  u_ddr : ddr
    generic map (
      
     TCQ                              => TCQ,
     ADDR_CMD_MODE                    => ADDR_CMD_MODE,
     AL                               => AL,
     PAYLOAD_WIDTH                    => PAYLOAD_WIDTH,
     BANK_WIDTH                       => BANK_WIDTH,
     BURST_MODE                       => BURST_MODE,
     BURST_TYPE                       => BURST_TYPE,
     CK_WIDTH                         => CK_WIDTH,
     COL_WIDTH                        => COL_WIDTH,
     CMD_PIPE_PLUS1                   => CMD_PIPE_PLUS1,
     CS_WIDTH                         => CS_WIDTH,
     nCS_PER_RANK                     => nCS_PER_RANK,
     CKE_WIDTH                        => CKE_WIDTH,
     DATA_WIDTH                       => DATA_WIDTH,
     DATA_BUF_ADDR_WIDTH              => DATA_BUF_ADDR_WIDTH,
     DQ_CNT_WIDTH                     => DQ_CNT_WIDTH,
     DQ_PER_DM                        => DQ_PER_DM,
     DQ_WIDTH                         => DQ_WIDTH,
     DQS_CNT_WIDTH                    => DQS_CNT_WIDTH,
     DQS_WIDTH                        => DQS_WIDTH,
     DRAM_WIDTH                       => DRAM_WIDTH,
     ECC                              => ECC,
     ECC_WIDTH                        => ECC_WIDTH,
     ECC_TEST                         => ECC_TEST,
     MC_ERR_ADDR_WIDTH                => MC_ERR_ADDR_WIDTH,
     nAL                              => nAL,
     nBANK_MACHS                      => nBANK_MACHS,
     CKE_ODT_AUX                      => CKE_ODT_AUX,
     ORDERING                         => ORDERING,
     OUTPUT_DRV                       => OUTPUT_DRV,
     IBUF_LPWR_MODE                   => IBUF_LPWR_MODE,
     IODELAY_HP_MODE                  => IODELAY_HP_MODE,
     DATA_IO_IDLE_PWRDWN              => DATA_IO_IDLE_PWRDWN,
     BANK_TYPE                        => BANK_TYPE,
     DATA_IO_PRIM_TYPE                => DATA_IO_PRIM_TYPE,
     REG_CTRL                         => REG_CTRL,
     RTT_NOM                          => RTT_NOM,
     CL                               => CL,
     tCKE                             => tCKE,
     tFAW                             => tFAW,
     tPRDI                            => tPRDI,
     tRAS                             => tRAS,
     tRCD                             => tRCD,
     tREFI                            => tREFI,
     tRFC                             => tRFC,
     tRP                              => tRP,
     tRRD                             => tRRD,
     tRTP                             => tRTP,
     tWTR                             => tWTR,
     tZQI                             => tZQI,
     tZQCS                            => tZQCS,
     USER_REFRESH                     => USER_REFRESH,
     WRLVL                            => WRLVL,
     DEBUG_PORT                       => DEBUG_PORT,
     RANKS                            => RANKS,
     ODT_WIDTH                        => ODT_WIDTH,
     ROW_WIDTH                        => ROW_WIDTH,
     ADDR_WIDTH                       => ADDR_WIDTH,
     SIM_BYPASS_INIT_CAL              => SIM_BYPASS_INIT_CAL,
     SIMULATION                       => SIMULATION,
     BYTE_LANES_B0                    => BYTE_LANES_B0,
     BYTE_LANES_B1                    => BYTE_LANES_B1,
     BYTE_LANES_B2                    => BYTE_LANES_B2,
     BYTE_LANES_B3                    => BYTE_LANES_B3,
     BYTE_LANES_B4                    => BYTE_LANES_B4,
     DATA_CTL_B0                      => DATA_CTL_B0,
     DATA_CTL_B1                      => DATA_CTL_B1,
     DATA_CTL_B2                      => DATA_CTL_B2,
     DATA_CTL_B3                      => DATA_CTL_B3,
     DATA_CTL_B4                      => DATA_CTL_B4,
     PHY_0_BITLANES                   => PHY_0_BITLANES,
     PHY_1_BITLANES                   => PHY_1_BITLANES,
     PHY_2_BITLANES                   => PHY_2_BITLANES,
     CK_BYTE_MAP                      => CK_BYTE_MAP,
     ADDR_MAP                         => ADDR_MAP,
     BANK_MAP                         => BANK_MAP,
     CAS_MAP                          => CAS_MAP,
     CKE_ODT_BYTE_MAP                 => CKE_ODT_BYTE_MAP,
     CKE_MAP                          => CKE_MAP,
     ODT_MAP                          => ODT_MAP,
     CS_MAP                           => CS_MAP,
     PARITY_MAP                       => PARITY_MAP,
     RAS_MAP                          => RAS_MAP,
     WE_MAP                           => WE_MAP,
     DQS_BYTE_MAP                     => DQS_BYTE_MAP,
     DATA0_MAP                        => DATA0_MAP,
     DATA1_MAP                        => DATA1_MAP,
     DATA2_MAP                        => DATA2_MAP,
     DATA3_MAP                        => DATA3_MAP,
     DATA4_MAP                        => DATA4_MAP,
     DATA5_MAP                        => DATA5_MAP,
     DATA6_MAP                        => DATA6_MAP,
     DATA7_MAP                        => DATA7_MAP,
     DATA8_MAP                        => DATA8_MAP,
     DATA9_MAP                        => DATA9_MAP,
     DATA10_MAP                       => DATA10_MAP,
     DATA11_MAP                       => DATA11_MAP,
     DATA12_MAP                       => DATA12_MAP,
     DATA13_MAP                       => DATA13_MAP,
     DATA14_MAP                       => DATA14_MAP,
     DATA15_MAP                       => DATA15_MAP,
     DATA16_MAP                       => DATA16_MAP,
     DATA17_MAP                       => DATA17_MAP,
     MASK0_MAP                        => MASK0_MAP,
     MASK1_MAP                        => MASK1_MAP,
     CALIB_ROW_ADD                    => CALIB_ROW_ADD,
     CALIB_COL_ADD                    => CALIB_COL_ADD,
     CALIB_BA_ADD                     => CALIB_BA_ADD,
     SLOT_0_CONFIG                    => SLOT_0_CONFIG,
     SLOT_1_CONFIG                    => SLOT_1_CONFIG,
     MEM_ADDR_ORDER                   => MEM_ADDR_ORDER,
     USE_CS_PORT                      => USE_CS_PORT,
     USE_DM_PORT                      => USE_DM_PORT,
     USE_ODT_PORT                     => USE_ODT_PORT,
     PHY_CONTROL_MASTER_BANK          => PHY_CONTROL_MASTER_BANK,
     TEMP_MON_CONTROL                 => TEMP_MON_CONTROL,
      
     
     DM_WIDTH                         => DM_WIDTH,
     
     nCK_PER_CLK                      => nCK_PER_CLK,
     tCK                              => tCK,
     DIFF_TERM_SYSCLK                 => DIFF_TERM_SYSCLK,
     CLKIN_PERIOD                     => CLKIN_PERIOD,
     CLKFBOUT_MULT                    => CLKFBOUT_MULT,
     DIVCLK_DIVIDE                    => DIVCLK_DIVIDE,
     CLKOUT0_PHASE                    => CLKOUT0_PHASE,
     CLKOUT0_DIVIDE                   => CLKOUT0_DIVIDE,
     CLKOUT1_DIVIDE                   => CLKOUT1_DIVIDE,
     CLKOUT2_DIVIDE                   => CLKOUT2_DIVIDE,
     CLKOUT3_DIVIDE                   => CLKOUT3_DIVIDE,
     
     SYSCLK_TYPE                      => SYSCLK_TYPE,
     REFCLK_TYPE                      => REFCLK_TYPE,
     SYS_RST_PORT                     => SYS_RST_PORT,
     REFCLK_FREQ                      => REFCLK_FREQ,
     DIFF_TERM_REFCLK                 => DIFF_TERM_REFCLK,
     IODELAY_GRP                      => IODELAY_GRP,
      
     CAL_WIDTH                        => CAL_WIDTH,
     STARVE_LIMIT                     => STARVE_LIMIT,
     DRAM_TYPE                        => DRAM_TYPE,
      
      
      RST_ACT_LOW                      => RST_ACT_LOW
      )
      port map (
        
       
-- Memory interface ports
       ddr2_addr                      => ddr2_addr,
       ddr2_ba                        => ddr2_ba,
       ddr2_cas_n                     => ddr2_cas_n,
       ddr2_ck_n                      => ddr2_ck_n,
       ddr2_ck_p                      => ddr2_ck_p,
       ddr2_cke                       => ddr2_cke,
       ddr2_ras_n                     => ddr2_ras_n,
       ddr2_we_n                      => ddr2_we_n,
       ddr2_dq                        => ddr2_dq,
       ddr2_dqs_n                     => ddr2_dqs_n,
       ddr2_dqs_p                     => ddr2_dqs_p,
       init_calib_complete            => init_calib_complete_i,
      
       ddr2_cs_n                      => ddr2_cs_n,
       ddr2_dm                        => ddr2_dm,
       ddr2_odt                       => ddr2_odt,
-- Application interface ports
       app_addr                       => app_addr,
       app_cmd                        => app_cmd,
       app_en                         => app_en,
       app_wdf_data                   => app_wdf_data,
       app_wdf_end                    => app_wdf_end,
       app_wdf_wren                   => app_wdf_wren,
       app_rd_data                    => app_rd_data,
       app_rd_data_end                => app_rd_data_end,
       app_rd_data_valid              => app_rd_data_valid,
       app_rdy                        => app_rdy,
       app_wdf_rdy                    => app_wdf_rdy,
       app_sr_req                     => '0',
       app_sr_active                  => app_sr_active,
       app_ref_req                    => '0',
       app_ref_ack                    => app_ref_ack,
       app_zq_req                     => '0',
       app_zq_ack                     => app_zq_ack,
       ui_clk                         => clk,
       ui_clk_sync_rst                => rst,
      
       app_wdf_mask                   => app_wdf_mask,
      
       
-- System Clock Ports
       sys_clk_i                       => sys_clk_i,
       device_temp_i                  => device_temp_i,
      
        sys_rst                        => sys_rst
        );
-- End of User Design top instance


--***************************************************************************
-- The traffic generation module instantiated below drives traffic (patterns)
-- on the application interface of the memory controller
--***************************************************************************

  tg_rst <= vio_tg_rst or po_win_tg_rst;

  u_traffic_gen_top : mig_7series_v1_9_traffic_gen_top
    generic map (
      TCQ                 => TCQ,
      SIMULATION          => SIMULATION,
      FAMILY              => "VIRTEX7",
      MEM_TYPE            => DRAM_TYPE,
      TST_MEM_INSTR_MODE  => TST_MEM_INSTR_MODE,
      --BL_WIDTH            => BL_WIDTH,
      nCK_PER_CLK         => nCK_PER_CLK,
      NUM_DQ_PINS         => PAYLOAD_WIDTH,
      MEM_BURST_LEN       => BURST_LENGTH,
      MEM_COL_WIDTH       => COL_WIDTH,
      PORT_MODE           => PORT_MODE,
      DATA_PATTERN        => DATA_PATTERN,
      CMD_PATTERN         => CMD_PATTERN,
      ADDR_WIDTH          => TG_ADDR_WIDTH,
      DATA_WIDTH          => APP_DATA_WIDTH,
      BEGIN_ADDRESS       => BEGIN_ADDRESS,
      DATA_MODE           => DATA_MODE,
      END_ADDRESS         => END_ADDRESS,
      PRBS_EADDR_MASK_POS => PRBS_EADDR_MASK_POS,
      CMD_WDT             => CMD_WDT,
      RD_WDT              => RD_WDT,
      WR_WDT              => WR_WDT,
      EYE_TEST            => EYE_TEST
      )
      port map (
        clk                  => clk,
        rst                  => rst,
        tg_only_rst          => tg_rst,
        manual_clear_error   => manual_clear_error,
        memc_init_done       => init_calib_complete_i,
        memc_cmd_full        => app_rdy_i,
        memc_cmd_en          => app_en,
        memc_cmd_instr       => app_cmd,
        memc_cmd_bl          => open,
        memc_cmd_addr        => app_addr_i,
        memc_wr_en           => app_wdf_wren,
        memc_wr_end          => app_wdf_end,
        memc_wr_mask         => app_wdf_mask((PAYLOAD_WIDTH*2*nCK_PER_CLK)/8-1 downto 0),
        memc_wr_data         => app_wdf_data(PAYLOAD_WIDTH*2*nCK_PER_CLK-1 downto 0),
        memc_wr_full         => app_wdf_rdy_i,
        memc_rd_en           => open,
        memc_rd_data         => app_rd_data(PAYLOAD_WIDTH*2*nCK_PER_CLK-1 downto 0),
        memc_rd_empty        => app_rd_data_valid_i,
        qdr_wr_cmd_o         => open,
        qdr_rd_cmd_o         => open,
        vio_pause_traffic    => vio_pause_traffic,
        vio_modify_enable    => vio_modify_enable,
        vio_data_mode_value  => vio_data_mode_value,
        vio_addr_mode_value  => vio_addr_mode_value,
        vio_instr_mode_value => vio_instr_mode_value,
        vio_bl_mode_value    => vio_bl_mode_value,
        vio_fixed_bl_value   => vio_fixed_bl_value,
        vio_fixed_instr_value=> vio_fixed_instr_value,
        vio_data_mask_gen    => vio_data_mask_gen,
        fixed_addr_i         => all_zeros1,
        fixed_data_i         => all_zeros1,
        simple_data0         => all_zeros1,
        simple_data1         => all_zeros1,
        simple_data2         => all_zeros1,
        simple_data3         => all_zeros1,
        simple_data4         => all_zeros1,
        simple_data5         => all_zeros1,
        simple_data6         => all_zeros1,
        simple_data7         => all_zeros1,
        wdt_en_i             => wdt_en_w,
        bram_cmd_i           => all_zeros2,
        bram_valid_i         => '0',
        bram_rdy_o           => open,
        cmp_data             => cmp_data,
        cmp_data_valid       => cmp_data_valid,
        cmp_error            => cmp_error,
        wr_data_counts       => wr_data_counts,
        rd_data_counts       => rd_data_counts,
        dq_error_bytelane_cmp => dq_error_bytelane_cmp,
        error                => tg_compare_error_i,
        error_status         => error_status,
        cumlative_dq_lane_error => cumlative_dq_lane_error,
        cmd_wdt_err_o        => cmd_wdt_err_w,
        wr_wdt_err_o         => wr_wdt_err_w,
        rd_wdt_err_o         => rd_wdt_err_w,
        mem_pattern_init_done   => mem_pattern_init_done
        );


  --*****************************************************************
  -- Default values are assigned to the debug inputs of the traffic
  -- generator
  --*****************************************************************
  vio_modify_enable     <= '0';
  vio_data_mode_value   <= "0010";
  vio_addr_mode_value   <= "011";
  vio_instr_mode_value  <= "0010";
  vio_bl_mode_value     <= "10";
  vio_fixed_bl_value    <= "0000010000";
  vio_data_mask_gen     <= '0';
  vio_pause_traffic     <= '0';
  vio_fixed_instr_value <= "001";
  dbg_clear_error       <= '0';
  po_win_tg_rst         <= '0';
  vio_tg_rst            <= '0';
  wdt_en_w              <= '1';

  dbg_sel_pi_incdec       <= '0';
  dbg_sel_po_incdec       <= '0';
  dbg_pi_f_inc            <= '0';
  dbg_pi_f_dec            <= '0';
  dbg_po_f_inc            <= '0';
  dbg_po_f_dec            <= '0';
  dbg_po_f_stg23_sel      <= '0';

      

end architecture arch_example_top;
