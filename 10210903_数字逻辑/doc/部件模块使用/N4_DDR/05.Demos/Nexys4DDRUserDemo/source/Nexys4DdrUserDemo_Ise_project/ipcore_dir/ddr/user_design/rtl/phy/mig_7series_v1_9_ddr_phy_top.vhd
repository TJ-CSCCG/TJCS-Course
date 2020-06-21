--*****************************************************************************
-- (c) Copyright 2008 - 2010 Xilinx, Inc. All rights reserved.
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
-- /___/  \  /    Vendor                : Xilinx
-- \   \   \/     Version               : 1.5
--  \   \         Application           : MIG
--  /   /         Filename              : ddr_phy_top.vhd
-- /___/   /\     Date Last Modified    : $date$
-- \   \  /  \    Date Created          : Jan 31 2012
--  \___\/\___\
--
--Device            : 7 Series
--Design Name       : DDR3 SDRAM
--Purpose           : Top level memory interface block. Instantiates a clock
--                    and reset generator, the memory controller, the phy and
--                    the user interface blocks.
--Reference         :
--Revision History  :
--*****************************************************************************

library ieee;
   use ieee.std_logic_1164.all;
   use ieee.numeric_std.all;


entity mig_7series_v1_9_ddr_phy_top is
  generic (

   TCQ             : integer := 100;     -- Register delay (simulation only)
   AL              : string  := "0";     -- Additive Latency option
   BANK_WIDTH      : integer := 3;       -- # of bank bits
   BURST_MODE      : string  := "8";     -- Burst length
   BURST_TYPE      : string  := "SEQ";   -- Burst type
   CA_MIRROR       : string  := "OFF";   -- C/A mirror opt for DDR3 dual rank
   CK_WIDTH        : integer := 1;       -- # of CK/CK# outputs to memory
   CL              : integer := 5;
   COL_WIDTH       : integer := 12;      -- column address width
   CS_WIDTH        : integer := 1;       -- # of unique CS outputs
   CKE_WIDTH       : integer := 1;       -- # of cke outputs
   CWL             : integer := 5;
   DM_WIDTH        : integer := 8;       -- # of DM (data mask)
   DQ_WIDTH        : integer := 64;      -- # of DQ (data)
   DQS_CNT_WIDTH   : integer := 3;       -- = ceil(log2(DQS_WIDTH))
   DQS_WIDTH       : integer := 8;       -- # of DQS (strobe)
   DRAM_TYPE       : string  := "DDR3";
   DRAM_WIDTH      : integer := 8;       -- # of DQ per DQS
   MASTER_PHY_CTL  : integer := 0;       -- The bank number where master PHY_CONTROL resides
   LP_DDR_CK_WIDTH : integer := 2;
   DATA_IO_IDLE_PWRDWN : string := "ON"; -- "ON" or "OFF"
   -- Hard PHY parameters
   PHYCTL_CMD_FIFO : string  := "FALSE";
   -- five fields, one per possible I/O bank, 4 bits in each field,
   -- 1 per lane data=1/ctl=0
   DATA_CTL_B0     : std_logic_vector(3 downto 0) := X"c";
   DATA_CTL_B1     : std_logic_vector(3 downto 0) := X"f";
   DATA_CTL_B2     : std_logic_vector(3 downto 0) := X"f";
   DATA_CTL_B3     : std_logic_vector(3 downto 0) := X"f";
   DATA_CTL_B4     : std_logic_vector(3 downto 0) := X"f";
   -- defines the byte lanes in I/O banks being used in the interface
   -- 1- Used, 0- Unused
   BYTE_LANES_B0   : std_logic_vector(3 downto 0) := "1111";
   BYTE_LANES_B1   : std_logic_vector(3 downto 0) := "0000";
   BYTE_LANES_B2   : std_logic_vector(3 downto 0) := "0000";
   BYTE_LANES_B3   : std_logic_vector(3 downto 0) := "0000";
   BYTE_LANES_B4   : std_logic_vector(3 downto 0) := "0000";
   -- defines the bit lanes in I/O banks being used in the interface. Each
   -- = 1 I/O bank = 4 byte lanes = 48 bit lanes. 1-Used, 0-Unused
   PHY_0_BITLANES  : std_logic_vector(47 downto 0) := X"000000000000";
   PHY_1_BITLANES  : std_logic_vector(47 downto 0) := X"000000000000";
   PHY_2_BITLANES  : std_logic_vector(47 downto 0) := X"000000000000";

   -- control/address/data pin mapping parameters
   CK_BYTE_MAP     : std_logic_vector(143 downto 0) := X"000000000000000000000000000000000000";
   ADDR_MAP        : std_logic_vector(191 downto 0) := X"000000000000000000000000000000000000000000000000";
   BANK_MAP        : std_logic_vector(35 downto 0) := X"000000000";
   CAS_MAP         : std_logic_vector(11 downto 0) := X"000";
   CKE_ODT_BYTE_MAP : std_logic_vector(7 downto 0) := X"00";
   CKE_MAP    : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   ODT_MAP    : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   CKE_ODT_AUX : string := "FALSE";
   CS_MAP     : std_logic_vector(119 downto 0) := X"000000000000000000000000000000";
   PARITY_MAP : std_logic_vector(11 downto 0) := X"000";
   RAS_MAP    : std_logic_vector(11 downto 0) := X"000";
   WE_MAP     : std_logic_vector(11 downto 0) := X"000";
   DQS_BYTE_MAP
     : std_logic_vector(143 downto 0) := X"000000000000000000000000000000000000";
   DATA0_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
   DATA1_MAP  : std_logic_vector(95 downto 0) := X"000000000000000000000000";
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
   MASK0_MAP  : std_logic_vector(107 downto 0) := X"000000000000000000000000000";
   MASK1_MAP  : std_logic_vector(107 downto 0) := X"000000000000000000000000000";

   -- This parameter must be set based on memory clock frequency
   -- It must be set to 4 for frequencies above 533 MHz?? (undecided)
   -- and set to 2 for 533 MHz and below
   PRE_REV3ES      : string  := "OFF";   -- Delay O/Ps using Phaser_Out fine dly
   nCK_PER_CLK     : integer := 2;       -- # of memory CKs per fabric CLK
   nCS_PER_RANK    : integer := 1;       -- # of unique CS outputs per rank
   ADDR_CMD_MODE   : string  := "1T";    -- ADDR/CTRL timing: "2T", "1T"
   IODELAY_HP_MODE : string  := "ON";
   BANK_TYPE       : string  := "HP_IO"; -- # = "HP_LP", "HR_LP", "DEFAULT"
   DATA_IO_PRIM_TYPE : string  := "DEFAULT"; -- # = "HP_LP", "HR_LP", "DEFAULT"
   IODELAY_GRP     : string  := "IODELAY_MIG";
   IBUF_LPWR_MODE  : string  := "OFF";   -- input buffer low power option
   OUTPUT_DRV      : string  := "HIGH";  -- to calib_top
   REG_CTRL        : string  := "OFF";   -- to calib_top
   RTT_NOM         : string  := "60";    -- to calib_top
   RTT_WR          : string  := "120";   -- to calib_top
   tCK             : integer := 2500;    -- pS
   tRFC            : integer := 110000;  -- pS
   DDR2_DQSN_ENABLE : string  := "YES";  -- Enable differential DQS for DDR2
   WRLVL           : string  := "OFF";   -- to calib_top
   DEBUG_PORT      : string  := "OFF";   -- to calib_top
   RANKS           : integer := 4;
   ODT_WIDTH       : integer := 1;
   ROW_WIDTH       : integer := 16;      -- DRAM address bus width
   SLOT_1_CONFIG   : std_logic_vector(7 downto 0) := "00000000";
   -- calibration Address. The address given below will be used for calibration
   -- read and write operations.
   CALIB_ROW_ADD   : std_logic_vector(15 downto 0) := X"0000"; -- Calibration row address
   CALIB_COL_ADD   : std_logic_vector(11 downto 0) := X"000";  -- Calibration column address
   CALIB_BA_ADD    : std_logic_vector(2 downto 0) := "000";    -- Calibration bank address
   -- Simulation /debug options
   SIM_BYPASS_INIT_CAL : string  := "OFF";
                                        -- Parameter used to force skipping
                                        -- or abbreviation of initialization
                                        -- and calibration. Overrides
                                        -- SIM_INIT_OPTION, SIM_CAL_OPTION,
                                        -- and disables various other blocks
   --parameter SIM_INIT_OPTION = "SKIP_PU_DLY", -- Skip various init steps
   --parameter SIM_CAL_OPTION  = "NONE",        -- Skip various calib steps
   REFCLK_FREQ     : real    := 200.0;         -- IODELAY ref clock freq (MHz)
   USE_CS_PORT     : integer := 1;             -- Support chip select output
   USE_DM_PORT     : integer := 1;             -- Support data mask output
   USE_ODT_PORT    : integer := 1;             -- Support ODT output
   RD_PATH_REG     : integer := 0              -- optional registers in the read path
                                              -- to MC for timing improvement.
                                              -- =1 enabled, = 0 disabled
  );
  port (
    clk                              : in    std_logic;            -- Fabric logic clock
                                             -- To MC, calib_top, hard PHY
    clk_ref                          : in    std_logic;        -- Idelay_ctrl reference clock
                                             -- To hard PHY (external source)
    freq_refclk                      : in    std_logic;    -- To hard PHY for Phasers
    mem_refclk                       : in    std_logic;     -- Memory clock to hard PHY
    pll_lock                         : in    std_logic;       -- System PLL lock signal
    sync_pulse                       : in    std_logic;     -- 1/N sync pulse used to
                                              -- synchronize all PHASERS
    error                            : in    std_logic;          -- Support for TG error detect
    rst_tg_mc                        : out    std_logic;      -- Support for TG error detect

    device_temp                      : in    std_logic_vector(11 downto 0);
    tempmon_sample_en                : in    std_logic;

    dbg_sel_pi_incdec                : in    std_logic;
    dbg_sel_po_incdec                : in    std_logic;
    dbg_byte_sel                     : in    std_logic_vector(DQS_CNT_WIDTH downto 0);
    dbg_pi_f_inc                     : in    std_logic;
    dbg_pi_f_dec                     : in    std_logic;
    dbg_po_f_inc                     : in    std_logic;
    dbg_po_f_stg23_sel               : in    std_logic;
    dbg_po_f_dec                     : in    std_logic;
    dbg_idel_down_all                : in    std_logic;
    dbg_idel_down_cpt                : in    std_logic;
    dbg_idel_up_all                  : in    std_logic;
    dbg_idel_up_cpt                  : in    std_logic;
    dbg_sel_all_idel_cpt             : in    std_logic;
    dbg_sel_idel_cpt                 : in    std_logic_vector(DQS_CNT_WIDTH-1 downto 0);
    rst                              : in    std_logic;
    slot_0_present                   : in    std_logic_vector(7 downto 0);
    slot_1_present                   : in    std_logic_vector(7 downto 0);
    -- From MC
    mc_ras_n                         : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
    mc_cas_n                         : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
    mc_we_n                          : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
    mc_address                       : in    std_logic_vector(nCK_PER_CLK*ROW_WIDTH-1 downto 0);
    mc_bank                          : in    std_logic_vector(nCK_PER_CLK*BANK_WIDTH-1 downto 0);
    mc_cs_n                          : in    std_logic_vector(CS_WIDTH*nCS_PER_RANK*nCK_PER_CLK-1 downto 0);
    mc_reset_n                       : in    std_logic;
    mc_odt                           : in    std_logic_vector(1 downto 0);
    mc_cke                           : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
    -- AUX - For ODT and CKE assertion during reads and writes
    mc_aux_out0                      : in    std_logic_vector(3 downto 0);
    mc_aux_out1                      : in    std_logic_vector(3 downto 0);
    mc_cmd_wren                      : in    std_logic;
    mc_ctl_wren                      : in    std_logic;
    mc_cmd                           : in    std_logic_vector(2 downto 0);
    mc_cas_slot                      : in    std_logic_vector(1 downto 0);
    mc_data_offset                   : in    std_logic_vector(5 downto 0);
    mc_data_offset_1                 : in    std_logic_vector(5 downto 0);
    mc_data_offset_2                 : in    std_logic_vector(5 downto 0);
    mc_rank_cnt                      : in    std_logic_vector(1 downto 0);
    -- Write
    mc_wrdata_en                     : in    std_logic;
    mc_wrdata                        : in    std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
    mc_wrdata_mask                   : in    std_logic_vector((2*nCK_PER_CLK*(DQ_WIDTH/8))-1 downto 0);
    idle                             : in    std_logic;
    -- DDR bus signals
    ddr_addr                         : out   std_logic_vector(ROW_WIDTH-1 downto 0);
    ddr_ba                           : out   std_logic_vector(BANK_WIDTH-1 downto 0);
    ddr_cas_n                        : out   std_logic;
    ddr_ck_n                         : out   std_logic_vector(CK_WIDTH-1 downto 0);
    ddr_ck                           : out   std_logic_vector(CK_WIDTH-1 downto 0);
    ddr_cke                          : out   std_logic_vector(CKE_WIDTH-1 downto 0);
    ddr_cs_n                         : out   std_logic_vector((CS_WIDTH*nCS_PER_RANK)-1 downto 0);
    ddr_dm                           : out   std_logic_vector(DM_WIDTH-1 downto 0);
    ddr_odt                          : out   std_logic_vector(ODT_WIDTH-1 downto 0);
    ddr_ras_n                        : out   std_logic;
    ddr_reset_n                      : out   std_logic;
    ddr_parity                       : out   std_logic;
    ddr_we_n                         : out   std_logic;
    ddr_dq                           : inout std_logic_vector(DQ_WIDTH-1 downto 0);
    ddr_dqs_n                        : inout std_logic_vector(DQS_WIDTH-1 downto 0);
    ddr_dqs                          : inout std_logic_vector(DQS_WIDTH-1 downto 0);

    dbg_calib_top                    : out   std_logic_vector(255 downto 0);
    dbg_cpt_first_edge_cnt           : out   std_logic_vector(6*DQS_WIDTH*RANKS-1 downto 0);
    dbg_cpt_second_edge_cnt          : out   std_logic_vector(6*DQS_WIDTH*RANKS-1 downto 0);
    dbg_cpt_tap_cnt                  : out   std_logic_vector(6*DQS_WIDTH*RANKS-1 downto 0);
    dbg_dq_idelay_tap_cnt            : out   std_logic_vector(5*DQS_WIDTH*RANKS-1 downto 0);
    dbg_phy_rdlvl                    : out   std_logic_vector(255 downto 0);
    dbg_phy_wrcal                    : out   std_logic_vector(99 downto 0);
    dbg_final_po_fine_tap_cnt        : out   std_logic_vector(6*DQS_WIDTH-1 downto 0);
    dbg_final_po_coarse_tap_cnt      : out   std_logic_vector(3*DQS_WIDTH-1 downto 0);
    dbg_rd_data_edge_detect          : out   std_logic_vector(DQS_WIDTH-1 downto 0);
    dbg_rddata                       : out   std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
    dbg_rddata_valid                 : out   std_logic;
    dbg_rdlvl_done                   : out   std_logic_vector(1 downto 0);
    dbg_rdlvl_err                    : out   std_logic_vector(1 downto 0);
    dbg_rdlvl_start                  : out   std_logic_vector(1 downto 0);
    dbg_tap_cnt_during_wrlvl         : out   std_logic_vector(5 downto 0);
    dbg_wl_edge_detect_valid         : out   std_logic;
    dbg_wrlvl_done                   : out   std_logic;
    dbg_wrlvl_err                    : out   std_logic;
    dbg_wrlvl_start                  : out   std_logic;
    dbg_wrlvl_fine_tap_cnt           : out   std_logic_vector(6*DQS_WIDTH-1 downto 0);
    dbg_wrlvl_coarse_tap_cnt         : out   std_logic_vector(3*DQS_WIDTH-1 downto 0);
    dbg_phy_wrlvl                    : out   std_logic_vector(255 downto 0);
    dbg_pi_phaselock_start           : out   std_logic;
    dbg_pi_phaselocked_done          : out   std_logic;
    dbg_pi_phaselock_err             : out   std_logic;
    dbg_pi_phase_locked_phy4lanes    : out   std_logic_vector(11 downto 0);
    dbg_pi_dqsfound_start            : out   std_logic;
    dbg_pi_dqsfound_done             : out   std_logic;
    dbg_pi_dqsfound_err              : out   std_logic;
    dbg_pi_dqs_found_lanes_phy4lanes : out   std_logic_vector(11 downto 0);
    dbg_wrcal_start                  : out   std_logic;
    dbg_wrcal_done                   : out   std_logic;
    dbg_wrcal_err                    : out   std_logic;
    -- FIFO status flags
    phy_mc_ctl_full                  : out   std_logic;
    phy_mc_cmd_full                  : out   std_logic;
    phy_mc_data_full                 : out   std_logic;
    -- Calibration status and resultant outputs
    init_calib_complete              : out   std_logic;
    init_wrcal_complete              : out   std_logic;
    calib_rd_data_offset_0           : out   std_logic_vector(6*RANKS-1 downto 0);
    calib_rd_data_offset_1           : out   std_logic_vector(6*RANKS-1 downto 0);
    calib_rd_data_offset_2           : out   std_logic_vector(6*RANKS-1 downto 0);
    phy_rddata_valid                 : out   std_logic;
    phy_rd_data                      : out   std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);

    ref_dll_lock                     : out   std_logic;
    rst_phaser_ref                   : in    std_logic;
    dbg_rd_data_offset               : out   std_logic_vector(6*RANKS-1 downto 0);
    dbg_phy_init                     : out   std_logic_vector(255 downto 0);
    dbg_prbs_rdlvl                   : out   std_logic_vector(255 downto 0);
    dbg_dqs_found_cal                : out   std_logic_vector(255 downto 0);
    dbg_pi_counter_read_val          : out   std_logic_vector(5 downto 0);
    dbg_po_counter_read_val          : out   std_logic_vector(8 downto 0);
    dbg_oclkdelay_calib_start        : out   std_logic;
    dbg_oclkdelay_calib_done         : out   std_logic;
    dbg_phy_oclkdelay_cal            : out   std_logic_vector(255 downto 0);
    dbg_oclkdelay_rd_data            : out   std_logic_vector(DRAM_WIDTH*16-1 downto 0)
   );

end entity;

architecture arch_ddr_phy_top of mig_7series_v1_9_ddr_phy_top is

   -- function to OR the bits in a vectored signal
   function OR_BR (inp_var: std_logic_vector)
            return std_logic is
       variable temp: std_logic := '0';
    begin
       for idx in inp_var'range loop
          temp := temp or inp_var(idx);
       end loop;
       return temp;
   end function;

   -- Calculate number of slots in the system
   function CALC_nSLOTS return integer is
   begin
      if (OR_BR(SLOT_1_CONFIG) = '1') then
         return (2);
      else
         return (1);
      end if;
   end function;

   function SIM_INIT_OPTION_W return string is
   begin
      if (SIM_BYPASS_INIT_CAL = "SKIP") then
         return ("SKIP_INIT");
      elsif (SIM_BYPASS_INIT_CAL = "FAST" or
             SIM_BYPASS_INIT_CAL = "SIM_FULL") then
         return ("SKIP_PU_DLY");
      else
         return ("NONE");
      end if;
   end function;

   function SIM_CAL_OPTION_W return string is
   begin
      if (SIM_BYPASS_INIT_CAL = "SKIP") then
         return ("SKIP_CAL");
      elsif (SIM_BYPASS_INIT_CAL = "FAST") then
         return ("FAST_CAL");
      elsif (SIM_BYPASS_INIT_CAL = "SIM_FULL" or
             SIM_BYPASS_INIT_CAL = "SIM_INIT_CAL_FULL") then
         return ("FAST_WIN_DETECT");
      else
         return ("NONE");
      end if;
   end function;

   function CALC_WRLVL_W return string is
   begin
      if (SIM_BYPASS_INIT_CAL = "SKIP") then
         return ("OFF");
      else
         return (WRLVL);
      end if;
   end function;

   function HIGHEST_BANK_W return integer is
   begin
      if (BYTE_LANES_B4 /= "0000") then
         return (5);
      elsif (BYTE_LANES_B3 /= "0000") then
         return (4);
      elsif (BYTE_LANES_B2 /= "0000") then
         return (3);
      elsif (BYTE_LANES_B1 /= "0000") then
         return (2);
      else
         return (1);
      end if;
   end function;

   function HIGHEST_LANE_B0_W return integer is
   begin
      if (BYTE_LANES_B0(3) = '1') then
         return (4);
      elsif (BYTE_LANES_B0(2) = '1') then
         return (3);
      elsif (BYTE_LANES_B0(1) = '1') then
         return (2);
      elsif (BYTE_LANES_B0(0) = '1') then
         return (1);
      else
         return (0);
      end if;
   end function;

   function HIGHEST_LANE_B1_W return integer is
   begin
      if (BYTE_LANES_B1(3) = '1') then
         return (4);
      elsif (BYTE_LANES_B1(2) = '1') then
         return (3);
      elsif (BYTE_LANES_B1(1) = '1') then
         return (2);
      elsif (BYTE_LANES_B1(0) = '1') then
         return (1);
      else
         return (0);
      end if;
   end function;

   function HIGHEST_LANE_B2_W return integer is
   begin
      if (BYTE_LANES_B2(3) = '1') then
         return (4);
      elsif (BYTE_LANES_B2(2) = '1') then
         return (3);
      elsif (BYTE_LANES_B2(1) = '1') then
         return (2);
      elsif (BYTE_LANES_B2(0) = '1') then
         return (1);
      else
         return (0);
      end if;
   end function;

   function HIGHEST_LANE_B3_W return integer is
   begin
      if (BYTE_LANES_B3(3) = '1') then
         return (4);
      elsif (BYTE_LANES_B3(2) = '1') then
         return (3);
      elsif (BYTE_LANES_B3(1) = '1') then
         return (2);
      elsif (BYTE_LANES_B3(0) = '1') then
         return (1);
      else
         return (0);
      end if;
   end function;

   function HIGHEST_LANE_B4_W return integer is
   begin
      if (BYTE_LANES_B4(3) = '1') then
         return (4);
      elsif (BYTE_LANES_B4(2) = '1') then
         return (3);
      elsif (BYTE_LANES_B4(1) = '1') then
         return (2);
      elsif (BYTE_LANES_B4(0) = '1') then
         return (1);
      else
         return (0);
      end if;
   end function;

   function HIGHEST_LANE_W return integer is
   begin
      if (HIGHEST_LANE_B4_W /= 0) then
         return (HIGHEST_LANE_B4_W+16);
      elsif (HIGHEST_LANE_B3_W /= 0) then
         return (HIGHEST_LANE_B3_W+12);
      elsif (HIGHEST_LANE_B2_W /= 0) then
         return (HIGHEST_LANE_B2_W+8);
      elsif (HIGHEST_LANE_B1_W /= 0) then
         return (HIGHEST_LANE_B1_W+4);
      else
         return (HIGHEST_LANE_B0_W);
      end if;
   end function;

   function N_CTL_LANES_B0 return integer is
       variable temp: integer := 0;
    begin
       for idx in 0 to 3 loop
         if (not(DATA_CTL_B0(idx)) = '1' and BYTE_LANES_B0(idx) = '1') then
           temp := temp + 1;
         else
           temp := temp;
         end if;
       end loop;
       return temp;
   end function;

   function N_CTL_LANES_B1 return integer is
       variable temp: integer := 0;
    begin
       for idx in 0 to 3 loop
         if (not(DATA_CTL_B1(idx)) = '1' and BYTE_LANES_B1(idx) = '1') then
           temp := temp + 1;
         else
           temp := temp;
         end if;
       end loop;
       return temp;
   end function;

   function N_CTL_LANES_B2 return integer is
       variable temp: integer := 0;
    begin
       for idx in 0 to 3 loop
         if (not(DATA_CTL_B2(idx)) = '1' and BYTE_LANES_B2(idx) = '1') then
           temp := temp + 1;
         else
           temp := temp;
         end if;
       end loop;
       return temp;
   end function;

   function N_CTL_LANES_B3 return integer is
       variable temp: integer := 0;
    begin
       for idx in 0 to 3 loop
         if (not(DATA_CTL_B3(idx)) = '1' and BYTE_LANES_B3(idx) = '1') then
           temp := temp + 1;
         else
           temp := temp;
         end if;
       end loop;
       return temp;
   end function;

   function N_CTL_LANES_B4 return integer is
       variable temp: integer := 0;
    begin
       for idx in 0 to 3 loop
         if (not(DATA_CTL_B4(idx)) = '1' and BYTE_LANES_B4(idx) = '1') then
           temp := temp + 1;
         else
           temp := temp;
         end if;
       end loop;
       return temp;
   end function;

   function CTL_BANK_B0 return std_logic is
    begin
      if ((not(DATA_CTL_B0(0)) = '1' and BYTE_LANES_B0(0) = '1') or
          (not(DATA_CTL_B0(1)) = '1' and BYTE_LANES_B0(1) = '1') or
          (not(DATA_CTL_B0(2)) = '1' and BYTE_LANES_B0(2) = '1') or
          (not(DATA_CTL_B0(3)) = '1' and BYTE_LANES_B0(3) = '1')) then
        return ('1')  ;
      else
        return ('0')  ;
      end if;
   end function;

   function CTL_BANK_B1 return std_logic is
    begin
      if ((not(DATA_CTL_B1(0)) = '1' and BYTE_LANES_B1(0) = '1') or
          (not(DATA_CTL_B1(1)) = '1' and BYTE_LANES_B1(1) = '1') or
          (not(DATA_CTL_B1(2)) = '1' and BYTE_LANES_B1(2) = '1') or
          (not(DATA_CTL_B1(3)) = '1' and BYTE_LANES_B1(3) = '1')) then
        return ('1')  ;
      else
        return ('0')  ;
      end if;
   end function;

   function CTL_BANK_B2 return std_logic is
    begin
      if ((not(DATA_CTL_B2(0)) = '1' and BYTE_LANES_B2(0) = '1') or
          (not(DATA_CTL_B2(1)) = '1' and BYTE_LANES_B2(1) = '1') or
          (not(DATA_CTL_B2(2)) = '1' and BYTE_LANES_B2(2) = '1') or
          (not(DATA_CTL_B2(3)) = '1' and BYTE_LANES_B2(3) = '1')) then
        return ('1')  ;
      else
        return ('0')  ;
      end if;
   end function;

   function CTL_BANK_B3 return std_logic is
    begin
      if ((not(DATA_CTL_B3(0)) = '1' and BYTE_LANES_B3(0) = '1') or
          (not(DATA_CTL_B3(1)) = '1' and BYTE_LANES_B3(1) = '1') or
          (not(DATA_CTL_B3(2)) = '1' and BYTE_LANES_B3(2) = '1') or
          (not(DATA_CTL_B3(3)) = '1' and BYTE_LANES_B3(3) = '1')) then
        return ('1')  ;
      else
        return ('0')  ;
      end if;
   end function;

   function CTL_BANK_B4 return std_logic is
    begin
      if ((not(DATA_CTL_B4(0)) = '1' and BYTE_LANES_B4(0) = '1') or
          (not(DATA_CTL_B4(1)) = '1' and BYTE_LANES_B4(1) = '1') or
          (not(DATA_CTL_B4(2)) = '1' and BYTE_LANES_B4(2) = '1') or
          (not(DATA_CTL_B4(3)) = '1' and BYTE_LANES_B4(3) = '1')) then
        return ('1')  ;
      else
        return ('0')  ;
      end if;
   end function;

  function CTL_BANK_W return std_logic_vector is
     variable ctl_bank_var : std_logic_vector(2 downto 0);
   begin
     if (CTL_BANK_B0 = '1') then
       ctl_bank_var := "000";
     elsif (CTL_BANK_B1 = '1') then
       ctl_bank_var := "001";
     elsif (CTL_BANK_B2 = '1') then
       ctl_bank_var := "010";
     elsif (CTL_BANK_B3 = '1') then
       ctl_bank_var := "011";
     elsif (CTL_BANK_B4 = '1') then
       ctl_bank_var := "100";
     else
       ctl_bank_var := "000";
     end if;
     return (ctl_bank_var);
  end function;

  function ODD_PARITY (inp_var : std_logic_vector) return std_logic is
    variable tmp : std_logic := '0';
    begin
      for idx in inp_var'range loop
        tmp := tmp XOR inp_var(idx);
      end loop;
     return tmp;
   end ODD_PARITY;

  -- Calculate number of slots in the system
  constant nSLOTS     : integer := CALC_nSLOTS;
  constant CLK_PERIOD : integer := tCK * nCK_PER_CLK;

  -- Parameter used to force skipping or abbreviation of initialization
  -- and calibration. Overrides SIM_INIT_OPTION, SIM_CAL_OPTION, and
  -- disables various other blocks depending on the option selected
  -- This option should only be used during simulation. In the case of
  -- the "SKIP" option, the testbench used should also not be modeling
  -- propagation delays.
  -- Allowable options = {"NONE", "SIM_FULL", "SKIP", "FAST"}
  --  "NONE"     = options determined by the individual parameter settings
  --  "SIM_FULL" = skip power-up delay. FULL calibration performed without
  --               averaging algorithm turned ON during window detection.
  --  "SKIP"     = skip power-up delay. Skip calibration not yet supported.
  --  "FAST"     = skip power-up delay, and calibrate (read leveling, write
  --               leveling, and phase detector) only using one DQS group, and
  --               apply the results to all other DQS groups.
  constant SIM_INIT_OPTION : string := SIM_INIT_OPTION_W;
  constant SIM_CAL_OPTION  : string := SIM_CAL_OPTION_W;
  constant WRLVL_W         : string := CALC_WRLVL_W;

  constant HIGHEST_BANK    : integer := HIGHEST_BANK_W;

  -- constant HIGHEST_LANE_B0 =  HIGHEST_LANE_B0_W;
  -- constant HIGHEST_LANE_B1 =  HIGHEST_LANE_B1_W;
  -- constant HIGHEST_LANE_B2 =  HIGHEST_LANE_B2_W;
  -- constant HIGHEST_LANE_B3 =  HIGHEST_LANE_B3_W;
  -- constant HIGHEST_LANE_B4 =  HIGHEST_LANE_B4_W;

  constant HIGHEST_LANE    : integer := HIGHEST_LANE_W;

  constant N_CTL_LANES     : integer := N_CTL_LANES_B0 + N_CTL_LANES_B1 + N_CTL_LANES_B2 + N_CTL_LANES_B3 + N_CTL_LANES_B4;

  -- Assuming Ck/Addr/Cmd and Control are placed in a single IO Bank
  -- This should be the case since the PLL should be placed adjacent
  -- to the same IO Bank as Ck/Addr/Cmd and Control
  constant CTL_BANK  : std_logic_vector(2 downto 0):= CTL_BANK_W;

  function CTL_BYTE_LANE_W return std_logic_vector is
     variable ctl_byte_lane_var: std_logic_vector(7 downto 0);
  begin
     if (N_CTL_LANES = 4) then
       ctl_byte_lane_var := "11100100";
     elsif (N_CTL_LANES = 3 and
            (((not(DATA_CTL_B0(0)) = '1') and BYTE_LANES_B0(0) = '1' and
              (not(DATA_CTL_B0(1)) = '1') and BYTE_LANES_B0(1) = '1' and
              (not(DATA_CTL_B0(2)) = '1') and BYTE_LANES_B0(2) = '1') or
             ((not(DATA_CTL_B1(0)) = '1') and BYTE_LANES_B1(0) = '1' and
              (not(DATA_CTL_B1(1)) = '1') and BYTE_LANES_B1(1) = '1' and
              (not(DATA_CTL_B1(2)) = '1') and BYTE_LANES_B1(2) = '1') or
             ((not(DATA_CTL_B2(0)) = '1') and BYTE_LANES_B2(0) = '1' and
              (not(DATA_CTL_B2(1)) = '1') and BYTE_LANES_B2(1) = '1' and
              (not(DATA_CTL_B2(2)) = '1') and BYTE_LANES_B2(2) = '1') or
             ((not(DATA_CTL_B3(0)) = '1') and BYTE_LANES_B3(0) = '1' and
              (not(DATA_CTL_B3(1)) = '1') and BYTE_LANES_B3(1) = '1' and
              (not(DATA_CTL_B3(2)) = '1') and BYTE_LANES_B3(2) = '1') or
             ((not(DATA_CTL_B4(0)) = '1') and BYTE_LANES_B4(0) = '1' and
              (not(DATA_CTL_B4(1)) = '1') and BYTE_LANES_B4(1) = '1' and
              (not(DATA_CTL_B4(2)) = '1') and BYTE_LANES_B4(2) = '1'))) then
       ctl_byte_lane_var := "00100100";
     elsif (N_CTL_LANES = 3 and
            (((not(DATA_CTL_B0(0)) = '1') and  BYTE_LANES_B0(0) = '1' and
              (not(DATA_CTL_B0(1)) = '1') and  BYTE_LANES_B0(1) = '1' and
              (not(DATA_CTL_B0(3)) = '1') and  BYTE_LANES_B0(3) = '1') or
             ((not(DATA_CTL_B1(0)) = '1') and  BYTE_LANES_B1(0) = '1' and
              (not(DATA_CTL_B1(1)) = '1') and  BYTE_LANES_B1(1) = '1' and
              (not(DATA_CTL_B1(3)) = '1') and  BYTE_LANES_B1(3) = '1') or
             ((not(DATA_CTL_B2(0)) = '1') and  BYTE_LANES_B2(0) = '1' and
              (not(DATA_CTL_B2(1)) = '1') and  BYTE_LANES_B2(1) = '1' and
              (not(DATA_CTL_B2(3)) = '1') and  BYTE_LANES_B2(3) = '1') or
             ((not(DATA_CTL_B3(0)) = '1') and  BYTE_LANES_B3(0) = '1' and
              (not(DATA_CTL_B3(1)) = '1') and  BYTE_LANES_B3(1) = '1' and
              (not(DATA_CTL_B3(3)) = '1') and  BYTE_LANES_B3(3) = '1') or
             ((not(DATA_CTL_B4(0)) = '1') and  BYTE_LANES_B4(0) = '1' and
              (not(DATA_CTL_B4(1)) = '1') and  BYTE_LANES_B4(1) = '1' and
              (not(DATA_CTL_B4(3)) = '1') and  BYTE_LANES_B4(3) = '1'))) then
       ctl_byte_lane_var := "00110100";
     elsif (N_CTL_LANES = 3 and
            (((not(DATA_CTL_B0(0)) = '1') and BYTE_LANES_B0(0) = '1' and
              (not(DATA_CTL_B0(2)) = '1') and BYTE_LANES_B0(2) = '1' and
              (not(DATA_CTL_B0(3)) = '1') and BYTE_LANES_B0(3) = '1') or
             ((not(DATA_CTL_B1(0)) = '1') and BYTE_LANES_B1(0) = '1' and
              (not(DATA_CTL_B1(2)) = '1') and BYTE_LANES_B1(2) = '1' and
              (not(DATA_CTL_B1(3)) = '1') and BYTE_LANES_B1(3) = '1') or
             ((not(DATA_CTL_B2(0)) = '1') and BYTE_LANES_B2(0) = '1' and
              (not(DATA_CTL_B2(2)) = '1') and BYTE_LANES_B2(2) = '1' and
              (not(DATA_CTL_B2(3)) = '1') and BYTE_LANES_B2(3) = '1') or
             ((not(DATA_CTL_B3(0)) = '1') and BYTE_LANES_B3(0) = '1' and
              (not(DATA_CTL_B3(2)) = '1') and BYTE_LANES_B3(2) = '1' and
              (not(DATA_CTL_B3(3)) = '1') and BYTE_LANES_B3(3) = '1') or
             ((not(DATA_CTL_B4(0)) = '1') and BYTE_LANES_B4(0) = '1' and
              (not(DATA_CTL_B4(2)) = '1') and BYTE_LANES_B4(2) = '1' and
              (not(DATA_CTL_B4(3)) = '1') and BYTE_LANES_B4(3) = '1'))) then
       ctl_byte_lane_var := "00111000";
     elsif (N_CTL_LANES = 3 and
            (((not(DATA_CTL_B0(0)) = '1') and BYTE_LANES_B0(0) = '1' and
              (not(DATA_CTL_B0(2)) = '1') and BYTE_LANES_B0(2) = '1' and
              (not(DATA_CTL_B0(3)) = '1') and BYTE_LANES_B0(3) = '1') or
             ((not(DATA_CTL_B1(0)) = '1') and BYTE_LANES_B1(0) = '1' and
              (not(DATA_CTL_B1(2)) = '1') and BYTE_LANES_B1(2) = '1' and
              (not(DATA_CTL_B1(3)) = '1') and BYTE_LANES_B1(3) = '1') or
             ((not(DATA_CTL_B2(0)) = '1') and BYTE_LANES_B2(0) = '1' and
              (not(DATA_CTL_B2(2)) = '1') and BYTE_LANES_B2(2) = '1' and
              (not(DATA_CTL_B2(3)) = '1') and BYTE_LANES_B2(3) = '1') or
             ((not(DATA_CTL_B3(0)) = '1') and BYTE_LANES_B3(0) = '1' and
              (not(DATA_CTL_B3(2)) = '1') and BYTE_LANES_B3(2) = '1' and
              (not(DATA_CTL_B3(3)) = '1') and BYTE_LANES_B3(3) = '1') or
             ((not(DATA_CTL_B4(0)) = '1') and BYTE_LANES_B4(0) = '1' and
              (not(DATA_CTL_B4(2)) = '1') and BYTE_LANES_B4(2) = '1' and
              (not(DATA_CTL_B4(3)) = '1') and BYTE_LANES_B4(3) = '1'))) then
       ctl_byte_lane_var := "00111001";
     elsif (N_CTL_LANES = 2 and
            (((not(DATA_CTL_B0(0)) = '1') and BYTE_LANES_B0(0) = '1' and
              (not(DATA_CTL_B0(1)) = '1') and BYTE_LANES_B0(1) = '1') or
             ((not(DATA_CTL_B1(0)) = '1') and BYTE_LANES_B1(0) = '1' and
              (not(DATA_CTL_B1(1)) = '1') and BYTE_LANES_B1(1) = '1') or
             ((not(DATA_CTL_B2(0)) = '1') and BYTE_LANES_B2(0) = '1' and
              (not(DATA_CTL_B2(1)) = '1') and BYTE_LANES_B2(1) = '1') or
             ((not(DATA_CTL_B3(0)) = '1') and BYTE_LANES_B3(0) = '1' and
              (not(DATA_CTL_B3(1)) = '1') and BYTE_LANES_B3(1) = '1') or
             ((not(DATA_CTL_B4(0)) = '1') and BYTE_LANES_B4(0) = '1' and
              (not(DATA_CTL_B4(1)) = '1') and BYTE_LANES_B4(1) = '1'))) then
       ctl_byte_lane_var := "00000100";
     elsif (N_CTL_LANES = 2 and
            (((not(DATA_CTL_B0(0)) = '1') and BYTE_LANES_B0(0) = '1' and
              (not(DATA_CTL_B0(3)) = '1') and BYTE_LANES_B0(3) = '1') or
             ((not(DATA_CTL_B1(0)) = '1') and BYTE_LANES_B1(0) = '1' and
              (not(DATA_CTL_B1(3)) = '1') and BYTE_LANES_B1(3) = '1') or
             ((not(DATA_CTL_B2(0)) = '1') and BYTE_LANES_B2(0) = '1' and
              (not(DATA_CTL_B2(3)) = '1') and BYTE_LANES_B2(3) = '1') or
             ((not(DATA_CTL_B3(0)) = '1') and BYTE_LANES_B3(0) = '1' and
              (not(DATA_CTL_B3(3)) = '1') and BYTE_LANES_B3(3) = '1') or
             ((not(DATA_CTL_B4(0)) = '1') and BYTE_LANES_B4(0) = '1' and
              (not(DATA_CTL_B4(3)) = '1') and BYTE_LANES_B4(3) = '1'))) then
       ctl_byte_lane_var := "00001100";
     elsif (N_CTL_LANES = 2 and
            (((not(DATA_CTL_B0(2)) = '1') and BYTE_LANES_B0(2) = '1' and
              (not(DATA_CTL_B0(3)) = '1') and BYTE_LANES_B0(3) = '1') or
             ((not(DATA_CTL_B1(2)) = '1') and BYTE_LANES_B1(2) = '1' and
              (not(DATA_CTL_B1(3)) = '1') and BYTE_LANES_B1(3) = '1') or
             ((not(DATA_CTL_B2(2)) = '1') and BYTE_LANES_B2(2) = '1' and
              (not(DATA_CTL_B2(3)) = '1') and BYTE_LANES_B2(3) = '1') or
             ((not(DATA_CTL_B3(2)) = '1') and BYTE_LANES_B3(2) = '1' and
              (not(DATA_CTL_B3(3)) = '1') and BYTE_LANES_B3(3) = '1') or
             ((not(DATA_CTL_B4(2)) = '1') and BYTE_LANES_B4(2) = '1' and
              (not(DATA_CTL_B4(3)) = '1') and BYTE_LANES_B4(3) = '1'))) then
       ctl_byte_lane_var := "00001110";
     elsif (N_CTL_LANES = 2 and
            (((not(DATA_CTL_B0(1)) = '1') and BYTE_LANES_B0(1) = '1' and
              (not(DATA_CTL_B0(2)) = '1') and BYTE_LANES_B0(2) = '1') or
             ((not(DATA_CTL_B1(1)) = '1') and BYTE_LANES_B1(1) = '1' and
              (not(DATA_CTL_B1(2)) = '1') and BYTE_LANES_B1(2) = '1') or
             ((not(DATA_CTL_B2(1)) = '1') and BYTE_LANES_B2(1) = '1' and
              (not(DATA_CTL_B2(2)) = '1') and BYTE_LANES_B2(2) = '1') or
             ((not(DATA_CTL_B3(1)) = '1') and BYTE_LANES_B3(1) = '1' and
              (not(DATA_CTL_B3(2)) = '1') and BYTE_LANES_B3(2) = '1') or
             ((not(DATA_CTL_B4(1)) = '1') and BYTE_LANES_B4(1) = '1' and
              (not(DATA_CTL_B4(2)) = '1') and BYTE_LANES_B4(2) = '1'))) then
       ctl_byte_lane_var := "00001001";
     elsif (N_CTL_LANES = 2 and
            (((not(DATA_CTL_B0(1)) = '1') and BYTE_LANES_B0(1) = '1' and
              (not(DATA_CTL_B0(3)) = '1') and BYTE_LANES_B0(3) = '1') or
             ((not(DATA_CTL_B1(1)) = '1') and BYTE_LANES_B1(1) = '1' and
              (not(DATA_CTL_B1(3)) = '1') and BYTE_LANES_B1(3) = '1') or
             ((not(DATA_CTL_B2(1)) = '1') and BYTE_LANES_B2(1) = '1' and
              (not(DATA_CTL_B2(3)) = '1') and BYTE_LANES_B2(3) = '1') or
             ((not(DATA_CTL_B3(1)) = '1') and BYTE_LANES_B3(1) = '1' and
              (not(DATA_CTL_B3(3)) = '1') and BYTE_LANES_B3(3) = '1') or
             ((not(DATA_CTL_B4(1)) = '1') and BYTE_LANES_B4(1) = '1' and
              (not(DATA_CTL_B4(3)) = '1') and BYTE_LANES_B4(3) = '1'))) then
       ctl_byte_lane_var := "00001101";
     elsif (N_CTL_LANES = 2 and
            (((not(DATA_CTL_B0(0)) = '1') and BYTE_LANES_B0(0) = '1' and
              (not(DATA_CTL_B0(2)) = '1') and BYTE_LANES_B0(2) = '1') or
             ((not(DATA_CTL_B1(0)) = '1') and BYTE_LANES_B1(0) = '1' and
              (not(DATA_CTL_B1(2)) = '1') and BYTE_LANES_B1(2) = '1') or
             ((not(DATA_CTL_B2(0)) = '1') and BYTE_LANES_B2(0) = '1' and
              (not(DATA_CTL_B2(2)) = '1') and BYTE_LANES_B2(2) = '1') or
             ((not(DATA_CTL_B3(0)) = '1') and BYTE_LANES_B3(0) = '1' and
              (not(DATA_CTL_B3(2)) = '1') and BYTE_LANES_B3(2) = '1') or
             ((not(DATA_CTL_B4(0)) = '1') and BYTE_LANES_B4(0) = '1' and
              (not(DATA_CTL_B4(2)) = '1') and BYTE_LANES_B4(2) = '1'))) then
       ctl_byte_lane_var := "00001000";
     else
       ctl_byte_lane_var := "11100100";
     end if;
     return (ctl_byte_lane_var);
  end function;

  constant CTL_BYTE_LANE  : std_logic_vector(7 downto 0):= CTL_BYTE_LANE_W;

  component mig_7series_v1_9_ddr_mc_phy_wrapper is
    generic (
      TCQ              : integer;
      tCK              : integer;
      BANK_TYPE        : string;
      DATA_IO_PRIM_TYPE : string;
      DATA_IO_IDLE_PWRDWN :string;
      IODELAY_GRP      : string;
      nCK_PER_CLK      : integer;
      nCS_PER_RANK     : integer;
      BANK_WIDTH       : integer;
      CKE_WIDTH        : integer;
      CS_WIDTH         : integer;
      CK_WIDTH         : integer;
      CWL              : integer;
      DDR2_DQSN_ENABLE : string;
      DM_WIDTH         : integer;
      DQ_WIDTH         : integer;
      DQS_CNT_WIDTH    : integer;
      DQS_WIDTH        : integer;
      DRAM_TYPE        : string;
      RANKS            : integer;
      ODT_WIDTH        : integer;
      REG_CTRL         : string;
      ROW_WIDTH        : integer;
      USE_CS_PORT      : integer;
      USE_DM_PORT      : integer;
      USE_ODT_PORT     : integer;
      IBUF_LPWR_MODE   : string;
      LP_DDR_CK_WIDTH  : integer;
      PHYCTL_CMD_FIFO : string;
      DATA_CTL_B0     : std_logic_vector(3 downto 0);
      DATA_CTL_B1     : std_logic_vector(3 downto 0);
      DATA_CTL_B2     : std_logic_vector(3 downto 0);
      DATA_CTL_B3     : std_logic_vector(3 downto 0);
      DATA_CTL_B4     : std_logic_vector(3 downto 0);
      BYTE_LANES_B0   : std_logic_vector(3 downto 0);
      BYTE_LANES_B1   : std_logic_vector(3 downto 0);
      BYTE_LANES_B2   : std_logic_vector(3 downto 0);
      BYTE_LANES_B3   : std_logic_vector(3 downto 0);
      BYTE_LANES_B4   : std_logic_vector(3 downto 0);
      PHY_0_BITLANES  : std_logic_vector(47 downto 0);
      PHY_1_BITLANES  : std_logic_vector(47 downto 0);
      PHY_2_BITLANES  : std_logic_vector(47 downto 0);
      HIGHEST_BANK    : integer;
      HIGHEST_LANE    : integer;
      CK_BYTE_MAP     : std_logic_vector(143 downto 0);
      ADDR_MAP        : std_logic_vector(191 downto 0);
      BANK_MAP        : std_logic_vector(35 downto 0);
      CAS_MAP         : std_logic_vector(11 downto 0);
      CKE_ODT_BYTE_MAP : std_logic_vector(7 downto 0);
      CKE_MAP         : std_logic_vector(95 downto 0);
      ODT_MAP         : std_logic_vector(95 downto 0);
      CKE_ODT_AUX     : string;
      CS_MAP          : std_logic_vector(119 downto 0);
      PARITY_MAP      : std_logic_vector(11 downto 0);
      RAS_MAP         : std_logic_vector(11 downto 0);
      WE_MAP          : std_logic_vector(11 downto 0);
      DQS_BYTE_MAP    : std_logic_vector(143 downto 0);
      DATA0_MAP       : std_logic_vector(95 downto 0);
      DATA1_MAP       : std_logic_vector(95 downto 0);
      DATA2_MAP       : std_logic_vector(95 downto 0);
      DATA3_MAP       : std_logic_vector(95 downto 0);
      DATA4_MAP       : std_logic_vector(95 downto 0);
      DATA5_MAP       : std_logic_vector(95 downto 0);
      DATA6_MAP       : std_logic_vector(95 downto 0);
      DATA7_MAP       : std_logic_vector(95 downto 0);
      DATA8_MAP       : std_logic_vector(95 downto 0);
      DATA9_MAP       : std_logic_vector(95 downto 0);
      DATA10_MAP      : std_logic_vector(95 downto 0);
      DATA11_MAP      : std_logic_vector(95 downto 0);
      DATA12_MAP      : std_logic_vector(95 downto 0);
      DATA13_MAP      : std_logic_vector(95 downto 0);
      DATA14_MAP      : std_logic_vector(95 downto 0);
      DATA15_MAP      : std_logic_vector(95 downto 0);
      DATA16_MAP      : std_logic_vector(95 downto 0);
      DATA17_MAP      : std_logic_vector(95 downto 0);
      MASK0_MAP       : std_logic_vector(107 downto 0);
      MASK1_MAP       : std_logic_vector(107 downto 0);
      SIM_CAL_OPTION  : string;
      MASTER_PHY_CTL  : integer
      );
    port (
      rst                              : in    std_logic;
      clk                              : in    std_logic;
      freq_refclk                      : in    std_logic;
      mem_refclk                       : in    std_logic;
      pll_lock                         : in    std_logic;
      sync_pulse                       : in    std_logic;
      idelayctrl_refclk                : in    std_logic;
      phy_cmd_wr_en                    : in    std_logic;
      phy_data_wr_en                   : in    std_logic;
      phy_ctl_wd                       : in    std_logic_vector(31 downto 0);
      phy_ctl_wr                       : in    std_logic;
      phy_if_empty_def                 : in    std_logic;
      phy_if_reset                     : in    std_logic;
      data_offset_1                    : in    std_logic_vector(5 downto 0);
      data_offset_2                    : in    std_logic_vector(5 downto 0);
      aux_in_1                         : in    std_logic_vector(3 downto 0);
      aux_in_2                         : in    std_logic_vector(3 downto 0);
      idelaye2_init_val                : out   std_logic_vector(4 downto 0);
      oclkdelay_init_val               : out   std_logic_vector(5 downto 0);
      if_empty                         : out   std_logic;
      phy_ctl_full                     : out   std_logic;
      phy_cmd_full                     : out   std_logic;
      phy_data_full                    : out   std_logic;
      phy_pre_data_a_full              : out   std_logic;
      ddr_clk                          : out   std_logic_vector(CK_WIDTH*LP_DDR_CK_WIDTH-1 downto 0);
      phy_mc_go                        : out   std_logic;
      phy_write_calib                  : in    std_logic;
      phy_read_calib                   : in    std_logic;
      calib_in_common                  : in    std_logic;
      calib_sel                        : in    std_logic_vector(5 downto 0);
      calib_zero_inputs                : in    std_logic_vector(HIGHEST_BANK-1 downto 0);
      calib_zero_ctrl                  : in    std_logic_vector(HIGHEST_BANK-1 downto 0);
      po_fine_enable                   : in    std_logic_vector(2 downto 0);
      po_coarse_enable                 : in    std_logic_vector(2 downto 0);
      po_fine_inc                      : in    std_logic_vector(2 downto 0);
      po_coarse_inc                    : in    std_logic_vector(2 downto 0);
      po_counter_load_en               : in    std_logic;
      po_counter_read_en               : in    std_logic;
      po_sel_fine_oclk_delay           : in    std_logic_vector(2 downto 0);
      po_counter_load_val              : in    std_logic_vector(8 downto 0);
      po_counter_read_val              : out   std_logic_vector(8 downto 0);
      pi_counter_read_val              : out   std_logic_vector(5 downto 0);
      pi_rst_dqs_find                  : in    std_logic_vector(HIGHEST_BANK-1 downto 0);
      pi_fine_enable                   : in    std_logic;
      pi_fine_inc                      : in    std_logic;
      pi_counter_load_en               : in    std_logic;
      pi_counter_load_val              : in    std_logic_vector(5 downto 0);
      idelay_ce                        : in    std_logic;
      idelay_inc                       : in    std_logic;
      idelay_ld                        : in    std_logic;
      idle                             : in    std_logic;
      pi_phase_locked                  : out   std_logic;
      pi_phase_locked_all              : out   std_logic;
      pi_dqs_found                     : out   std_logic;
      pi_dqs_found_all                 : out   std_logic;
      pi_dqs_out_of_range              : out   std_logic;
      phy_init_data_sel                : in    std_logic;
      mux_address                      : in    std_logic_vector(nCK_PER_CLK*ROW_WIDTH-1 downto 0);
      mux_bank                         : in    std_logic_vector(nCK_PER_CLK*BANK_WIDTH-1 downto 0);
      mux_cas_n                        : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
      mux_cs_n                         : in    std_logic_vector(CS_WIDTH*nCS_PER_RANK*nCK_PER_CLK-1 downto 0);
      mux_ras_n                        : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
      mux_odt                          : in    std_logic_vector(1 downto 0);
      mux_cke                          : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
      mux_we_n                         : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
      parity_in                        : in    std_logic_vector(nCK_PER_CLK-1 downto 0);
      mux_wrdata                       : in    std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
      mux_wrdata_mask                  : in    std_logic_vector(2*nCK_PER_CLK*(DQ_WIDTH/8)-1 downto 0);
      mux_reset_n                      : in    std_logic;
      rd_data                          : out   std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
      ddr_addr                         : out   std_logic_vector(ROW_WIDTH-1 downto 0);
      ddr_ba                           : out   std_logic_vector(BANK_WIDTH-1 downto 0);
      ddr_cas_n                        : out   std_logic;
      ddr_cke                          : out   std_logic_vector(CKE_WIDTH-1 downto 0);
      ddr_cs_n                         : out   std_logic_vector(CS_WIDTH*nCS_PER_RANK-1 downto 0);
      ddr_dm                           : out   std_logic_vector(DM_WIDTH-1 downto 0);
      ddr_odt                          : out   std_logic_vector(ODT_WIDTH-1 downto 0);
      ddr_parity                       : out   std_logic;
      ddr_ras_n                        : out   std_logic;
      ddr_we_n                         : out   std_logic;
      ddr_reset_n                      : out   std_logic;
      ddr_dq                           : inout std_logic_vector(DQ_WIDTH-1 downto 0);
      ddr_dqs                          : inout std_logic_vector(DQS_WIDTH-1 downto 0);
      ddr_dqs_n                        : inout std_logic_vector(DQS_WIDTH-1 downto 0);
      dbg_pi_counter_read_en           : in    std_logic;
      ref_dll_lock                     : out   std_logic;
      rst_phaser_ref                   : in    std_logic;
      dbg_pi_phase_locked_phy4lanes    : out   std_logic_vector(11 downto 0);
      dbg_pi_dqs_found_lanes_phy4lanes : out   std_logic_vector(11 downto 0)
      );
  end component mig_7series_v1_9_ddr_mc_phy_wrapper;

  component mig_7series_v1_9_ddr_calib_top is
    generic (
      TCQ             : integer;
      nCK_PER_CLK     : integer;
      tCK             : integer;
      CLK_PERIOD      : integer;
      N_CTL_LANES     : integer;
      DRAM_TYPE       : string;
      PRBS_WIDTH      : integer;
      HIGHEST_LANE    : integer;
      HIGHEST_BANK    : integer;
      BANK_TYPE       : string;
      BYTE_LANES_B0   : std_logic_vector(3 downto 0);
      BYTE_LANES_B1   : std_logic_vector(3 downto 0);
      BYTE_LANES_B2   : std_logic_vector(3 downto 0);
      BYTE_LANES_B3   : std_logic_vector(3 downto 0);
      BYTE_LANES_B4   : std_logic_vector(3 downto 0);
      DATA_CTL_B0     : std_logic_vector(3 downto 0);
      DATA_CTL_B1     : std_logic_vector(3 downto 0);
      DATA_CTL_B2     : std_logic_vector(3 downto 0);
      DATA_CTL_B3     : std_logic_vector(3 downto 0);
      DATA_CTL_B4     : std_logic_vector(3 downto 0);
      DQS_BYTE_MAP    : std_logic_vector(143 downto 0);
      CTL_BYTE_LANE   : std_logic_vector(7 downto 0);
      CTL_BANK        : std_logic_vector(2 downto 0);
      SLOT_1_CONFIG : std_logic_vector(7 downto 0);
      BANK_WIDTH      : integer;
      CA_MIRROR       : string;
      COL_WIDTH       : integer;
      nCS_PER_RANK    : integer;
      DQ_WIDTH        : integer;
      DQS_CNT_WIDTH   : integer;
      DQS_WIDTH       : integer;
      DRAM_WIDTH      : integer;
      ROW_WIDTH       : integer;
      RANKS           : integer;
      CS_WIDTH        : integer;
      CKE_WIDTH       : integer;
      DDR2_DQSN_ENABLE : string;
      PER_BIT_DESKEW  : string;
      CALIB_ROW_ADD   : std_logic_vector(15 downto 0);
      CALIB_COL_ADD   : std_logic_vector(11 downto 0);
      CALIB_BA_ADD    : std_logic_vector(2 downto 0);
      AL              : string;
      ADDR_CMD_MODE   : string;
      BURST_MODE      : string;
      BURST_TYPE      : string;
      nCL             : integer;
      nCWL            : integer;
      tRFC            : integer;
      OUTPUT_DRV      : string;
      REG_CTRL        : string;
      RTT_NOM         : string;
      RTT_WR          : string;
      USE_ODT_PORT    : integer;
      WRLVL           : string;
      PRE_REV3ES      : string;
      SIM_INIT_OPTION : string;
      SIM_CAL_OPTION  : string;
      CKE_ODT_AUX     : string;
      DEBUG_PORT      : string
      );
    port (
      clk                         : in    std_logic;
      rst                         : in    std_logic;
      slot_0_present              : in    std_logic_vector(7 downto 0);
      slot_1_present              : in    std_logic_vector(7 downto 0);
      phy_ctl_ready               : in    std_logic;
      phy_ctl_full                : in    std_logic;
      phy_cmd_full                : in    std_logic;
      phy_data_full               : in    std_logic;
      write_calib                 : out   std_logic;
      read_calib                  : out   std_logic;
      calib_ctl_wren              : out   std_logic;
      calib_cmd_wren              : out   std_logic;
      calib_seq                   : out   std_logic_vector(1 downto 0);
      calib_aux_out               : out   std_logic_vector(3 downto 0);
      calib_cke                   : out   std_logic_vector(nCK_PER_CLK-1 downto 0);
      calib_odt                   : out   std_logic_vector(1 downto 0);
      calib_cmd                   : out   std_logic_vector(2 downto 0);
      calib_wrdata_en             : out   std_logic;
      calib_rank_cnt              : out   std_logic_vector(1 downto 0);
      calib_cas_slot              : out   std_logic_vector(1 downto 0);
      calib_data_offset_0         : out   std_logic_vector(5 downto 0);
      calib_data_offset_1         : out   std_logic_vector(5 downto 0);
      calib_data_offset_2         : out   std_logic_vector(5 downto 0);
      phy_address                 : out   std_logic_vector(nCK_PER_CLK*ROW_WIDTH-1 downto 0);
      phy_bank                    : out   std_logic_vector(nCK_PER_CLK*BANK_WIDTH-1 downto 0);
      phy_cs_n                    : out   std_logic_vector(CS_WIDTH*nCS_PER_RANK*nCK_PER_CLK-1 downto 0);
      phy_ras_n                   : out   std_logic_vector(nCK_PER_CLK-1 downto 0);
      phy_cas_n                   : out   std_logic_vector(nCK_PER_CLK-1 downto 0);
      phy_we_n                    : out   std_logic_vector(nCK_PER_CLK-1 downto 0);
      phy_reset_n                 : out   std_logic;
      calib_sel                   : out   std_logic_vector(5 downto 0);
      calib_in_common             : out   std_logic;
      calib_zero_inputs           : out   std_logic_vector(HIGHEST_BANK-1 downto 0);
      calib_zero_ctrl             : out   std_logic_vector(HIGHEST_BANK-1 downto 0);
      phy_if_empty_def            : out   std_logic;
      phy_if_reset                : out   std_logic;
      pi_phaselocked              : in    std_logic;
      pi_phase_locked_all         : in    std_logic;
      pi_found_dqs                : in    std_logic;
      pi_dqs_found_all            : in    std_logic;
      pi_dqs_found_lanes          : in    std_logic_vector(HIGHEST_LANE-1 downto 0);
      pi_counter_read_val         : in    std_logic_vector(5 downto 0);
      pi_rst_stg1_cal             : out   std_logic_vector(HIGHEST_BANK-1 downto 0);
      pi_en_stg2_f                : out   std_logic;
      pi_stg2_f_incdec            : out   std_logic;
      pi_stg2_load                : out   std_logic;
      pi_stg2_reg_l               : out   std_logic_vector(5 downto 0);
      idelay_ce                   : out   std_logic;
      idelay_inc                  : out   std_logic;
      idelay_ld                   : out   std_logic;
      po_sel_stg2stg3             : out   std_logic_vector(2 downto 0);
      po_stg2_c_incdec            : out   std_logic_vector(2 downto 0);
      po_en_stg2_c                : out   std_logic_vector(2 downto 0);
      po_stg2_f_incdec            : out   std_logic_vector(2 downto 0);
      po_en_stg2_f                : out   std_logic_vector(2 downto 0);
      po_counter_load_en          : out   std_logic;
      po_counter_read_val         : in    std_logic_vector(8 downto 0);
      device_temp                 : in    std_logic_vector(11 downto 0);
      tempmon_sample_en           : in    std_logic;
      phy_if_empty                : in    std_logic;
      idelaye2_init_val           : in    std_logic_vector(4 downto 0);
      oclkdelay_init_val          : in    std_logic_vector(5 downto 0);
      tg_err                      : in    std_logic;
      rst_tg_mc                   : out   std_logic;
      phy_wrdata                  : out   std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
      dlyval_dq                   : out   std_logic_vector(5*RANKS*DQ_WIDTH-1 downto 0);
      phy_rddata                  : in    std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
      calib_rd_data_offset_0      : out   std_logic_vector(6*RANKS-1 downto 0);
      calib_rd_data_offset_1      : out   std_logic_vector(6*RANKS-1 downto 0);
      calib_rd_data_offset_2      : out   std_logic_vector(6*RANKS-1 downto 0);
      phy_rddata_valid            : out   std_logic;
      calib_writes                : out   std_logic;
      init_calib_complete         : out   std_logic;
      init_wrcal_complete         : out   std_logic;
      pi_phase_locked_err         : out   std_logic;
      pi_dqsfound_err             : out   std_logic;
      wrcal_err                   : out   std_logic;
      dbg_pi_phaselock_start      : out   std_logic;
      dbg_pi_dqsfound_start       : out   std_logic;
      dbg_pi_dqsfound_done        : out   std_logic;
      dbg_wrcal_start             : out   std_logic;
      dbg_wrcal_done              : out   std_logic;
      dbg_wrlvl_start             : out   std_logic;
      dbg_wrlvl_done              : out   std_logic;
      dbg_wrlvl_err               : out   std_logic;
      dbg_wrlvl_fine_tap_cnt      : out   std_logic_vector(6*DQS_WIDTH-1 downto 0);
      dbg_wrlvl_coarse_tap_cnt    : out   std_logic_vector(3*DQS_WIDTH-1 downto 0);
      dbg_phy_wrlvl               : out   std_logic_vector(255 downto 0);
      dbg_tap_cnt_during_wrlvl    : out   std_logic_vector(5 downto 0);
      dbg_wl_edge_detect_valid    : out   std_logic;
      dbg_rd_data_edge_detect     : out   std_logic_vector(DQS_WIDTH-1 downto 0);
      dbg_final_po_fine_tap_cnt   : out   std_logic_vector(6*DQS_WIDTH-1 downto 0);
      dbg_final_po_coarse_tap_cnt : out   std_logic_vector(3*DQS_WIDTH-1 downto 0);
      dbg_phy_wrcal               : out   std_logic_vector(99 downto 0);
      dbg_rdlvl_start             : out   std_logic_vector(1 downto 0);
      dbg_rdlvl_done              : out   std_logic_vector(1 downto 0);
      dbg_rdlvl_err               : out   std_logic_vector(1 downto 0);
      dbg_cpt_first_edge_cnt      : out   std_logic_vector(6*DQS_WIDTH*RANKS-1 downto 0);
      dbg_cpt_second_edge_cnt     : out   std_logic_vector(6*DQS_WIDTH*RANKS-1 downto 0);
      dbg_cpt_tap_cnt             : out   std_logic_vector(6*DQS_WIDTH*RANKS-1 downto 0);
      dbg_dq_idelay_tap_cnt       : out   std_logic_vector(5*DQS_WIDTH*RANKS-1 downto 0);
      dbg_sel_pi_incdec           : in    std_logic;
      dbg_sel_po_incdec           : in    std_logic;
      dbg_byte_sel                : in    std_logic_vector(DQS_CNT_WIDTH downto 0);
      dbg_pi_f_inc                : in    std_logic;
      dbg_pi_f_dec                : in    std_logic;
      dbg_po_f_inc                : in    std_logic;
      dbg_po_f_stg23_sel          : in    std_logic;
      dbg_po_f_dec                : in    std_logic;
      dbg_idel_up_all             : in    std_logic;
      dbg_idel_down_all           : in    std_logic;
      dbg_idel_up_cpt             : in    std_logic;
      dbg_idel_down_cpt           : in    std_logic;
      dbg_sel_idel_cpt            : in    std_logic_vector(DQS_CNT_WIDTH-1 downto 0);
      dbg_sel_all_idel_cpt        : in    std_logic;
      dbg_phy_rdlvl               : out   std_logic_vector(255 downto 0);
      dbg_calib_top               : out   std_logic_vector(255 downto 0);
      dbg_phy_init                : out   std_logic_vector(255 downto 0);
      dbg_prbs_rdlvl              : out   std_logic_vector(255 downto 0);
      dbg_dqs_found_cal           : out   std_logic_vector(255 downto 0);
      dbg_phy_oclkdelay_cal       : out   std_logic_vector(255 downto 0);
      dbg_oclkdelay_rd_data       : out   std_logic_vector(DRAM_WIDTH*16-1 downto 0);
      dbg_oclkdelay_calib_start   : out   std_logic;
      dbg_oclkdelay_calib_done    : out   std_logic
   );
  end component mig_7series_v1_9_ddr_calib_top;

  signal phy_din               : std_logic_vector(HIGHEST_LANE*80-1 downto 0);
  signal phy_dout              : std_logic_vector(HIGHEST_LANE*80-1 downto 0);
  signal ddr_cmd_ctl_data      : std_logic_vector(HIGHEST_LANE*12-1 downto 0);
  signal aux_out               : std_logic_vector((((HIGHEST_LANE+3)/4)*4)-1 downto 0);
  signal ddr_clk               : std_logic_vector(CK_WIDTH * LP_DDR_CK_WIDTH-1 downto 0);
  signal phy_mc_go             : std_logic;
  signal phy_ctl_full          : std_logic;
  signal phy_cmd_full          : std_logic;
  signal phy_data_full         : std_logic;
  signal phy_pre_data_a_full   : std_logic;
  signal if_empty              : std_logic;
  signal phy_write_calib       : std_logic;
  signal phy_read_calib        : std_logic;
  signal rst_stg1_cal          : std_logic_vector(HIGHEST_BANK-1 downto 0);
  signal calib_sel             : std_logic_vector(5 downto 0);
  signal calib_in_common       : std_logic;
  signal calib_zero_inputs     : std_logic_vector(HIGHEST_BANK-1 downto 0);
  signal calib_zero_ctrl       : std_logic_vector(HIGHEST_BANK-1 downto 0);
  signal pi_phase_locked       : std_logic;
  signal pi_phase_locked_all   : std_logic;
  signal pi_found_dqs          : std_logic;
  signal pi_dqs_found_all      : std_logic;
  signal pi_dqs_out_of_range   : std_logic;
  signal pi_enstg2_f           : std_logic;
  signal pi_stg2_fincdec       : std_logic;
  signal pi_stg2_load          : std_logic;
  signal pi_stg2_reg_l         : std_logic_vector(5 downto 0);
  signal idelay_ce             : std_logic;
  signal idelay_inc            : std_logic;
  signal idelay_ld             : std_logic;
  signal po_sel_stg2stg3       : std_logic_vector(2 downto 0);
  signal po_stg2_cincdec       : std_logic_vector(2 downto 0);
  signal po_enstg2_c           : std_logic_vector(2 downto 0);
  signal po_stg2_fincdec       : std_logic_vector(2 downto 0);
  signal po_enstg2_f           : std_logic_vector(2 downto 0);
  signal po_counter_read_val   : std_logic_vector(8 downto 0);
  signal pi_counter_read_val   : std_logic_vector(5 downto 0);
  signal phy_wrdata            : std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
  signal parity                : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal phy_address           : std_logic_vector(nCK_PER_CLK*ROW_WIDTH-1 downto 0);
  signal phy_bank              : std_logic_vector(nCK_PER_CLK*BANK_WIDTH-1 downto 0);
  signal phy_cs_n              : std_logic_vector(CS_WIDTH*nCS_PER_RANK*nCK_PER_CLK-1 downto 0);
  signal phy_ras_n             : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal phy_cas_n             : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal phy_we_n              : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal phy_reset_n           : std_logic;
  signal calib_aux_out         : std_logic_vector(3 downto 0);
  signal calib_cke             : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal calib_odt             : std_logic_vector(1 downto 0);
  signal calib_ctl_wren        : std_logic;
  signal calib_cmd_wren        : std_logic;
  signal calib_wrdata_en       : std_logic;
  signal calib_cmd             : std_logic_vector(2 downto 0);
  signal calib_seq             : std_logic_vector(1 downto 0);
  signal calib_data_offset_0   : std_logic_vector(5 downto 0);
  signal calib_data_offset_1   : std_logic_vector(5 downto 0);
  signal calib_data_offset_2   : std_logic_vector(5 downto 0);
  signal calib_rank_cnt        : std_logic_vector(1 downto 0);
  signal calib_cas_slot        : std_logic_vector(1 downto 0);
  signal mux_address           : std_logic_vector(nCK_PER_CLK*ROW_WIDTH-1 downto 0);
  signal mux_aux_out           : std_logic_vector(3 downto 0);
  signal aux_out_map           : std_logic_vector(3 downto 0);
  signal mux_bank              : std_logic_vector(nCK_PER_CLK*BANK_WIDTH-1 downto 0);
  signal mux_cmd               : std_logic_vector(2 downto 0);
  signal mux_cmd_wren          : std_logic;
  signal mux_cs_n              : std_logic_vector(CS_WIDTH*nCS_PER_RANK*nCK_PER_CLK-1 downto 0);
  signal mux_ctl_wren          : std_logic;
  signal mux_cas_slot          : std_logic_vector(1 downto 0);
  signal mux_data_offset       : std_logic_vector(5 downto 0);
  signal mux_data_offset_1     : std_logic_vector(5 downto 0);
  signal mux_data_offset_2     : std_logic_vector(5 downto 0);
  signal mux_ras_n             : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal mux_cas_n             : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal mux_rank_cnt          : std_logic_vector(1 downto 0);
  signal mux_reset_n           : std_logic;
  signal mux_we_n              : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal mux_wrdata            : std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
  signal mux_wrdata_mask       : std_logic_vector(2*nCK_PER_CLK*(DQ_WIDTH/8)-1 downto 0);
  signal mux_wrdata_en         : std_logic;
  signal mux_cke               : std_logic_vector(nCK_PER_CLK-1 downto 0);
  signal mux_odt               : std_logic_vector(1 downto 0);
  signal phy_if_empty_def      : std_logic;
  signal phy_if_reset          : std_logic;
  signal phy_init_data_sel     : std_logic;
  signal rd_data_map           : std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
  signal phy_rddata_valid_w    : std_logic;
  signal rddata_valid_reg      : std_logic;
  signal rd_data_reg           : std_logic_vector(2*nCK_PER_CLK*DQ_WIDTH-1 downto 0);
  signal idelaye2_init_val     : std_logic_vector(4 downto 0);
  signal oclkdelay_init_val    : std_logic_vector(5 downto 0);

  signal mc_cs_n_temp          : std_logic_vector(CS_WIDTH*nCS_PER_RANK*nCK_PER_CLK-1 downto 0);

  signal calib_rd_data_offset_i0 : std_logic_vector(6*RANKS-1 downto 0);
  signal init_wrcal_complete_i   : std_logic;
  signal phy_ctl_wd_i            : std_logic_vector(31 downto 0);
  signal po_counter_load_en      : std_logic;
  signal parity_0_wire           : std_logic_vector((ROW_WIDTH+BANK_WIDTH+3)-1 downto 0);
  signal parity_1_wire           : std_logic_vector((ROW_WIDTH+BANK_WIDTH+3)-1 downto 0);
  signal parity_2_wire           : std_logic_vector((ROW_WIDTH+BANK_WIDTH+3)-1 downto 0);
  signal parity_3_wire           : std_logic_vector((ROW_WIDTH+BANK_WIDTH+3)-1 downto 0);
  signal dbg_pi_dqs_found_lanes_phy4lanes_i : std_logic_vector(11 downto 0);
  signal all_zeros               : std_logic_vector(8 downto 0):= (others => '0');

  begin

  --***************************************************************************

  dbg_rddata_valid <= rddata_valid_reg;
  dbg_rddata       <= rd_data_reg;

  dbg_rd_data_offset     <= calib_rd_data_offset_i0;
  calib_rd_data_offset_0 <= calib_rd_data_offset_i0;

  dbg_pi_phaselocked_done <= pi_phase_locked_all;

  dbg_po_counter_read_val <= po_counter_read_val;
  dbg_pi_counter_read_val <= pi_counter_read_val;

  dbg_pi_dqs_found_lanes_phy4lanes <= dbg_pi_dqs_found_lanes_phy4lanes_i;

  init_wrcal_complete <= init_wrcal_complete_i;

  --***************************************************************************

  clock_gen : for i in 0 to (CK_WIDTH-1) generate
    ddr_ck(i)   <= ddr_clk(LP_DDR_CK_WIDTH * i);
    ddr_ck_n(i) <= ddr_clk((LP_DDR_CK_WIDTH * i) + 1);
  end generate;

  --***************************************************************************
  -- During memory initialization and calibration the calibration logic drives
  -- the memory signals. After calibration is complete the memory controller
  -- drives the memory signals.
  -- Do not expect timing issues in 4:1 mode at 800 MHz/1600 Mbps
  --***************************************************************************

    cs_rdimm : if((REG_CTRL = "ON") and (DRAM_TYPE = "DDR3") and (RANKS = 1) and (nCS_PER_RANK = 2)) generate
      cs_rdimm_gen: for v in 0 to (CS_WIDTH*nCS_PER_RANK*nCK_PER_CLK)-1 generate
        cs_rdimm_gen_i : if((v mod (CS_WIDTH*nCS_PER_RANK)) = 0) generate
          mc_cs_n_temp(v) <= mc_cs_n(v) ;
        end generate;

        cs_rdimm_gen_j : if(not((v mod (CS_WIDTH*nCS_PER_RANK)) = 0)) generate
          mc_cs_n_temp(v) <= '1' ;
       end generate;

      end generate;
    end generate;

    cs_others : if(not(REG_CTRL = "ON") or not(DRAM_TYPE = "DDR3") or not(RANKS = 1) or not(nCS_PER_RANK = 2)) generate
          mc_cs_n_temp <= mc_cs_n ;
    end generate;

  mux_wrdata      <= mc_wrdata      when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_wrdata;
  mux_wrdata_mask <= mc_wrdata_mask when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else (others => '0');
  mux_address     <= mc_address     when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_address;
  mux_bank        <= mc_bank        when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_bank;
  mux_cs_n        <= mc_cs_n_temp   when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_cs_n;
  mux_ras_n       <= mc_ras_n       when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_ras_n;
  mux_cas_n       <= mc_cas_n       when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_cas_n;
  mux_we_n        <= mc_we_n        when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_we_n;
  mux_reset_n     <= mc_reset_n     when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else phy_reset_n;
  mux_aux_out     <= mc_aux_out0    when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else calib_aux_out;
  mux_odt         <= mc_odt         when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else calib_odt;
  mux_cke         <= mc_cke         when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else calib_cke;
  mux_cmd_wren    <= mc_cmd_wren    when (phy_init_data_sel ='1' or init_wrcal_complete_i = '1') else calib_cmd_wren;
  mux_ctl_wren    <= mc_ctl_wren    when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else calib_ctl_wren;
  mux_wrdata_en   <= mc_wrdata_en   when (phy_init_data_sel = '1' or init_wrcal_complete_i = '1') else calib_wrdata_en;
  mux_cmd         <= mc_cmd         when (phy_init_data_sel ='1' or init_wrcal_complete_i ='1') else calib_cmd;
  mux_cas_slot    <= mc_cas_slot    when (phy_init_data_sel ='1' or init_wrcal_complete_i = '1') else calib_cas_slot;
  mux_data_offset <= mc_data_offset when (phy_init_data_sel ='1' or init_wrcal_complete_i = '1') else calib_data_offset_0;
  mux_data_offset_1 <= mc_data_offset_1 when (phy_init_data_sel ='1' or init_wrcal_complete_i = '1') else calib_data_offset_1;
  mux_data_offset_2 <= mc_data_offset_2 when (phy_init_data_sel ='1' or init_wrcal_complete_i = '1') else calib_data_offset_2;
  -- Reserved field. Hard coded to 2'b00 irrespective of the number of ranks. CR 643601
  mux_rank_cnt    <= "00";


  -- Assigning cke & odt for DDR2 & DDR3
  -- No changes for DDR3 & DDR2 dual rank
  -- DDR2 single rank systems might potentially need 3 odt signals.
  -- Aux_out[2] will have the odt toggled by phy and controller
  -- wiring aux_out[2] to 0 & 3. Depending upon the odt parameter
  -- all of the three odt bits or some of them might be used.
  -- mapping done in mc_phy_wrapper module
  aux_out_gen : if(CKE_ODT_AUX = "TRUE") generate
    aux_out_map <= (mux_aux_out(1) & mux_aux_out(1) & mux_aux_out(1) &
                    mux_aux_out(0)) when ((DRAM_TYPE = "DDR2") and
                                          (RANKS = 1)) else
                   mux_aux_out;
  end generate;

  wo_aux_out_gen : if(not(CKE_ODT_AUX = "TRUE")) generate
    aux_out_map <= "0000";
  end generate;

  init_calib_complete <= phy_init_data_sel;

  phy_mc_ctl_full  <= phy_ctl_full;
  phy_mc_cmd_full  <= phy_cmd_full;
  phy_mc_data_full <= phy_pre_data_a_full;


  --***************************************************************************
  -- Generate parity for DDR3 RDIMM.
  --***************************************************************************

  gen_ddr3_parity : if ((DRAM_TYPE = "DDR3") and (REG_CTRL = "ON")) generate

    gen_ddr3_parity_4by1: if (nCK_PER_CLK = 4) generate

      parity_0_wire <= (mux_address((ROW_WIDTH*4)-1 downto ROW_WIDTH*3) &
                        mux_bank((BANK_WIDTH*4)-1 downto BANK_WIDTH*3) &
                        mux_cas_n(3) & mux_ras_n(3) & mux_we_n(3));
      parity_1_wire <= (mux_address(ROW_WIDTH-1 downto 0) &
                        mux_bank(BANK_WIDTH-1 downto 0) & mux_cas_n(0) &
                        mux_ras_n(0) & mux_we_n(0));
      parity_2_wire <= (mux_address((ROW_WIDTH*2)-1 downto ROW_WIDTH) &
                        mux_bank((BANK_WIDTH*2)-1 downto BANK_WIDTH) &
                        mux_cas_n(1) & mux_ras_n(1) & mux_we_n(1));
      parity_3_wire <= (mux_address((ROW_WIDTH*3)-1 downto ROW_WIDTH*2) &
                        mux_bank((BANK_WIDTH*3)-1 downto BANK_WIDTH*2) &
                        mux_cas_n(2) & mux_ras_n(2) & mux_we_n(2));

      process (clk)
      begin
        if (clk'event and clk = '1') then
          parity(0) <= ODD_PARITY(parity_0_wire) after (TCQ) * 1 ps;
        end if;
      end process;

      process (mux_address, mux_bank, mux_cas_n, mux_ras_n, mux_we_n)
      begin
          parity(1) <= ODD_PARITY(parity_1_wire) after (TCQ) * 1 ps;
          parity(2) <= ODD_PARITY(parity_2_wire) after (TCQ) * 1 ps;
          parity(3) <= ODD_PARITY(parity_3_wire) after (TCQ) * 1 ps;
      end process;
    end generate;

    gen_ddr3_parity_2by1: if ( not(nCK_PER_CLK = 4)) generate

      parity_1_wire <= (mux_address(ROW_WIDTH-1 downto 0) &
                        mux_bank(BANK_WIDTH-1 downto 0) & mux_cas_n(0) &
                        mux_ras_n(0) & mux_we_n(0));
      parity_2_wire <= (mux_address((ROW_WIDTH*2)-1 downto ROW_WIDTH) &
                        mux_bank((BANK_WIDTH*2)-1 downto BANK_WIDTH) &
                        mux_cas_n(1) & mux_ras_n(1) & mux_we_n(1));

      process (clk)
      begin
        if (clk'event and clk='1') then
           parity(0) <= ODD_PARITY(parity_2_wire) after (TCQ) * 1 ps;
        end if;
      end process;

      process(mux_address, mux_bank, mux_cas_n, mux_ras_n, mux_we_n)
      begin
          parity(1) <= ODD_PARITY(parity_1_wire) after (TCQ) * 1 ps;
      end process;
    end generate;
  end generate;

  gen_ddr3_noparity : if (not(DRAM_TYPE = "DDR3") or not(REG_CTRL = "ON")) generate
    gen_ddr3_noparity_4by1 : if (nCK_PER_CLK = 4) generate
      process (clk)
      begin
        if (clk'event and clk='1') then
          parity(0) <= '0' after (TCQ)*1 ps;
          parity(1) <= '0' after (TCQ)*1 ps;
          parity(2) <= '0' after (TCQ)*1 ps;
          parity(3) <= '0' after (TCQ)*1 ps;
        end if;
      end process;
    end generate;

    gen_ddr3_noparity_2by1 : if (not(nCK_PER_CLK = 4)) generate
      process (clk)
      begin
        if (clk'event and clk='1') then
          parity(0) <= '0' after (TCQ)*1 ps;
          parity(1) <= '0' after (TCQ)*1 ps;
        end if;
      end process;
    end generate;

  end generate;

  --***************************************************************************
  -- Code for optional register stage in read path to MC for timing
  --***************************************************************************
    RD_REG_TIMING : if(RD_PATH_REG = 1) generate
      process (clk)
      begin
        if (clk'event and clk='1') then
          rddata_valid_reg <= phy_rddata_valid_w after (TCQ)*1 ps;
          rd_data_reg      <= rd_data_map after (TCQ)*1 ps;
        end if;
      end process;
    end generate;

    RD_REG_NO_TIMING : if( not(RD_PATH_REG = 1)) generate
      process (phy_rddata_valid_w, rd_data_map)
      begin
        rddata_valid_reg <= phy_rddata_valid_w;
        rd_data_reg      <= rd_data_map;
      end process;
    end generate;

  phy_rddata_valid <= rddata_valid_reg;
  phy_rd_data <= rd_data_reg;

  --***************************************************************************
  -- Hard PHY and accompanying bit mapping logic
  --***************************************************************************

  phy_ctl_wd_i <= ("00000" & mux_cas_slot & calib_seq & mux_data_offset &
                   mux_rank_cnt & "000" & aux_out_map & "00000" & mux_cmd);

  u_ddr_mc_phy_wrapper : mig_7series_v1_9_ddr_mc_phy_wrapper
    generic map (
      TCQ                => TCQ,
      tCK                => tCK,
      BANK_TYPE          => BANK_TYPE,
      DATA_IO_PRIM_TYPE  => DATA_IO_PRIM_TYPE,
      IODELAY_GRP        => IODELAY_GRP,
      DATA_IO_IDLE_PWRDWN=> DATA_IO_IDLE_PWRDWN,
      nCK_PER_CLK        => nCK_PER_CLK,
      nCS_PER_RANK       => nCS_PER_RANK,
      BANK_WIDTH         => BANK_WIDTH,
      CKE_WIDTH          => CKE_WIDTH,
      CS_WIDTH           => CS_WIDTH,
      CK_WIDTH           => CK_WIDTH,
      CWL                => CWL,
      DDR2_DQSN_ENABLE   => DDR2_DQSN_ENABLE,
      DM_WIDTH           => DM_WIDTH,
      DQ_WIDTH           => DQ_WIDTH,
      DQS_CNT_WIDTH      => DQS_CNT_WIDTH,
      DQS_WIDTH          => DQS_WIDTH,
      DRAM_TYPE          => DRAM_TYPE,
      RANKS              => RANKS,
      ODT_WIDTH          => ODT_WIDTH,
      REG_CTRL           => REG_CTRL,
      ROW_WIDTH          => ROW_WIDTH,
      USE_CS_PORT        => USE_CS_PORT,
      USE_DM_PORT        => USE_DM_PORT,
      USE_ODT_PORT       => USE_ODT_PORT,
      IBUF_LPWR_MODE     => IBUF_LPWR_MODE,
      LP_DDR_CK_WIDTH    => LP_DDR_CK_WIDTH,
      PHYCTL_CMD_FIFO    => PHYCTL_CMD_FIFO,
      DATA_CTL_B0        => DATA_CTL_B0,
      DATA_CTL_B1        => DATA_CTL_B1,
      DATA_CTL_B2        => DATA_CTL_B2,
      DATA_CTL_B3        => DATA_CTL_B3,
      DATA_CTL_B4        => DATA_CTL_B4,
      BYTE_LANES_B0      => BYTE_LANES_B0,
      BYTE_LANES_B1      => BYTE_LANES_B1,
      BYTE_LANES_B2      => BYTE_LANES_B2,
      BYTE_LANES_B3      => BYTE_LANES_B3,
      BYTE_LANES_B4      => BYTE_LANES_B4,
      PHY_0_BITLANES     => PHY_0_BITLANES,
      PHY_1_BITLANES     => PHY_1_BITLANES,
      PHY_2_BITLANES     => PHY_2_BITLANES,
      HIGHEST_BANK       => HIGHEST_BANK,
      HIGHEST_LANE       => HIGHEST_LANE,
      CK_BYTE_MAP        => CK_BYTE_MAP,
      ADDR_MAP           => ADDR_MAP,
      BANK_MAP           => BANK_MAP,
      CAS_MAP            => CAS_MAP,
      CKE_ODT_BYTE_MAP   => CKE_ODT_BYTE_MAP,
      CKE_MAP            => CKE_MAP,
      ODT_MAP            => ODT_MAP,
      CKE_ODT_AUX        => CKE_ODT_AUX,
      CS_MAP             => CS_MAP,
      PARITY_MAP         => PARITY_MAP,
      RAS_MAP            => RAS_MAP,
      WE_MAP             => WE_MAP,
      DQS_BYTE_MAP       => DQS_BYTE_MAP,
      DATA0_MAP          => DATA0_MAP,
      DATA1_MAP          => DATA1_MAP,
      DATA2_MAP          => DATA2_MAP,
      DATA3_MAP          => DATA3_MAP,
      DATA4_MAP          => DATA4_MAP,
      DATA5_MAP          => DATA5_MAP,
      DATA6_MAP          => DATA6_MAP,
      DATA7_MAP          => DATA7_MAP,
      DATA8_MAP          => DATA8_MAP,
      DATA9_MAP          => DATA9_MAP,
      DATA10_MAP         => DATA10_MAP,
      DATA11_MAP         => DATA11_MAP,
      DATA12_MAP         => DATA12_MAP,
      DATA13_MAP         => DATA13_MAP,
      DATA14_MAP         => DATA14_MAP,
      DATA15_MAP         => DATA15_MAP,
      DATA16_MAP         => DATA16_MAP,
      DATA17_MAP         => DATA17_MAP,
      MASK0_MAP          => MASK0_MAP,
      MASK1_MAP          => MASK1_MAP,
      SIM_CAL_OPTION     => SIM_CAL_OPTION,
      MASTER_PHY_CTL     => MASTER_PHY_CTL
      )
    port map (
      rst                              => rst,
      clk                              => clk,
      -- For memory frequencies between 400~1066 MHz freq_refclk = mem_refclk
      -- For memory frequencies below 400 MHz mem_refclk = mem_refclk and
      -- freq_refclk = 2x or 4x mem_refclk such that it remains in the
      -- 400~1066 MHz range
      freq_refclk                      => freq_refclk,
      mem_refclk                       => mem_refclk,
      pll_lock                         => pll_lock,
      sync_pulse                       => sync_pulse,
      idelayctrl_refclk                => clk_ref,
      phy_cmd_wr_en                    => mux_cmd_wren,
      phy_data_wr_en                   => mux_wrdata_en,
      -- phy_ctl_wd = {ACTPRE[31:30],EventDelay[29:25],seq[24:23],
      --               DataOffset[22:17],HiIndex[16:15],LowIndex[14:12],
      --               AuxOut[11:8],ControlOffset[7:3],PHYCmd[2:0]}
      -- The fields ACTPRE, and BankCount are only used
      -- when the hard PHY counters are used by the MC.
      phy_ctl_wd                       => phy_ctl_wd_i,
      phy_ctl_wr                       => mux_ctl_wren,
      phy_if_empty_def                 => phy_if_empty_def,
      phy_if_reset                     => phy_if_reset,
      data_offset_1                    => mux_data_offset_1,
      data_offset_2                    => mux_data_offset_2,
      aux_in_1                         => aux_out_map,
      aux_in_2                         => aux_out_map,
      idelaye2_init_val                => idelaye2_init_val,
      oclkdelay_init_val               => oclkdelay_init_val,
      if_empty                         => if_empty,
      phy_ctl_full                     => phy_ctl_full,
      phy_cmd_full                     => phy_cmd_full,
      phy_data_full                    => phy_data_full,
      phy_pre_data_a_full              => phy_pre_data_a_full,
      ddr_clk                          => ddr_clk,
      phy_mc_go                        => phy_mc_go,
      phy_write_calib                  => phy_write_calib,
      phy_read_calib                   => phy_read_calib,
      calib_in_common                  => calib_in_common,
      calib_sel                        => calib_sel,
      calib_zero_inputs                => calib_zero_inputs,
      calib_zero_ctrl                  => calib_zero_ctrl,
      po_fine_enable                   => po_enstg2_f,
      po_coarse_enable                 => po_enstg2_c,
      po_fine_inc                      => po_stg2_fincdec,
      po_coarse_inc                    => po_stg2_cincdec,
      po_counter_load_en               => po_counter_load_en,
      po_counter_read_en               => '1',
      po_sel_fine_oclk_delay           => po_sel_stg2stg3,
      po_counter_load_val              => all_zeros,
      po_counter_read_val              => po_counter_read_val,
      pi_counter_read_val              => pi_counter_read_val,
      pi_rst_dqs_find                  => rst_stg1_cal,
      pi_fine_enable                   => pi_enstg2_f,
      pi_fine_inc                      => pi_stg2_fincdec,
      pi_counter_load_en               => pi_stg2_load,
      pi_counter_load_val              => pi_stg2_reg_l,
      idelay_ce                        => idelay_ce,
      idelay_inc                       => idelay_inc,
      idelay_ld                        => idelay_ld,
      idle                             => idle,
      pi_phase_locked                  => pi_phase_locked,
      pi_phase_locked_all              => pi_phase_locked_all,
      pi_dqs_found                     => pi_found_dqs,
      pi_dqs_found_all                 => pi_dqs_found_all,
      -- Currently not being used. May be used in future if periodic reads
      -- become a requirement. This output could also be used to signal a
      -- catastrophic failure in read capture and the need for re-cal
      pi_dqs_out_of_range              => pi_dqs_out_of_range,
      phy_init_data_sel                => phy_init_data_sel,
      mux_address                      => mux_address,
      mux_bank                         => mux_bank,
      mux_cas_n                        => mux_cas_n,
      mux_cs_n                         => mux_cs_n,
      mux_ras_n                        => mux_ras_n,
      mux_odt                          => mux_odt,
      mux_cke                          => mux_cke,
      mux_we_n                         => mux_we_n,
      parity_in                        => parity,
      mux_wrdata                       => mux_wrdata,
      mux_wrdata_mask                  => mux_wrdata_mask,
      mux_reset_n                      => mux_reset_n,
      rd_data                          => rd_data_map,
      ddr_addr                         => ddr_addr,
      ddr_ba                           => ddr_ba,
      ddr_cas_n                        => ddr_cas_n,
      ddr_cke                          => ddr_cke,
      ddr_cs_n                         => ddr_cs_n,
      ddr_dm                           => ddr_dm,
      ddr_odt                          => ddr_odt,
      ddr_parity                       => ddr_parity,
      ddr_ras_n                        => ddr_ras_n,
      ddr_we_n                         => ddr_we_n,
      ddr_reset_n                      => ddr_reset_n,
      ddr_dq                           => ddr_dq,
      ddr_dqs                          => ddr_dqs,
      ddr_dqs_n                        => ddr_dqs_n,
      dbg_pi_counter_read_en           => '1',
      ref_dll_lock                     => ref_dll_lock,
      rst_phaser_ref                   => rst_phaser_ref,
      dbg_pi_phase_locked_phy4lanes    => dbg_pi_phase_locked_phy4lanes,
      dbg_pi_dqs_found_lanes_phy4lanes => dbg_pi_dqs_found_lanes_phy4lanes_i
      );

  --***************************************************************************
  -- Soft memory initialization and calibration logic
  --***************************************************************************

  u_ddr_calib_top : mig_7series_v1_9_ddr_calib_top
    generic map (
      TCQ              => TCQ,
      nCK_PER_CLK      => nCK_PER_CLK,
      tCK              => tCK,
      CLK_PERIOD       => CLK_PERIOD,
      N_CTL_LANES      => N_CTL_LANES,
      DRAM_TYPE        => DRAM_TYPE,
      PRBS_WIDTH       => 8,
      HIGHEST_LANE     => HIGHEST_LANE,
      HIGHEST_BANK     => HIGHEST_BANK,
      BANK_TYPE        => BANK_TYPE,
      BYTE_LANES_B0    => BYTE_LANES_B0,
      BYTE_LANES_B1    => BYTE_LANES_B1,
      BYTE_LANES_B2    => BYTE_LANES_B2,
      BYTE_LANES_B3    => BYTE_LANES_B3,
      BYTE_LANES_B4    => BYTE_LANES_B4,
      DATA_CTL_B0      => DATA_CTL_B0,
      DATA_CTL_B1      => DATA_CTL_B1,
      DATA_CTL_B2      => DATA_CTL_B2,
      DATA_CTL_B3      => DATA_CTL_B3,
      DATA_CTL_B4      => DATA_CTL_B4,
      DQS_BYTE_MAP     => DQS_BYTE_MAP,
      CTL_BYTE_LANE    => CTL_BYTE_LANE,
      CTL_BANK         => CTL_BANK,
      SLOT_1_CONFIG    => SLOT_1_CONFIG,
      BANK_WIDTH       => BANK_WIDTH,
      CA_MIRROR        => CA_MIRROR,
      COL_WIDTH        => COL_WIDTH,
      nCS_PER_RANK     => nCS_PER_RANK,
      DQ_WIDTH         => DQ_WIDTH,
      DQS_CNT_WIDTH    => DQS_CNT_WIDTH,
      DQS_WIDTH        => DQS_WIDTH,
      DRAM_WIDTH       => DRAM_WIDTH,
      ROW_WIDTH        => ROW_WIDTH,
      RANKS            => RANKS,
      CS_WIDTH         => CS_WIDTH,
      CKE_WIDTH        => CKE_WIDTH,
      DDR2_DQSN_ENABLE => DDR2_DQSN_ENABLE,
      PER_BIT_DESKEW   => "OFF",
      CALIB_ROW_ADD    => CALIB_ROW_ADD,
      CALIB_COL_ADD    => CALIB_COL_ADD,
      CALIB_BA_ADD     => CALIB_BA_ADD,
      AL               => AL,
      ADDR_CMD_MODE    => ADDR_CMD_MODE,
      BURST_MODE       => BURST_MODE,
      BURST_TYPE       => BURST_TYPE,
      nCL              => CL,
      nCWL             => CWL,
      tRFC             => tRFC,
      OUTPUT_DRV       => OUTPUT_DRV,
      REG_CTRL         => REG_CTRL,
      RTT_NOM          => RTT_NOM,
      RTT_WR           => RTT_WR,
      USE_ODT_PORT     => USE_ODT_PORT,
      WRLVL            => WRLVL_W,
      PRE_REV3ES       => PRE_REV3ES,
      SIM_INIT_OPTION  => SIM_INIT_OPTION,
      SIM_CAL_OPTION   => SIM_CAL_OPTION,
      CKE_ODT_AUX      => CKE_ODT_AUX,
      DEBUG_PORT       => DEBUG_PORT
      )
    port map (
      clk                         => clk,
      rst                         => rst,

      slot_0_present              => slot_0_present,
      slot_1_present              => slot_1_present,
      -- PHY Control Block and IN_FIFO status
      phy_ctl_ready               => phy_mc_go,
      phy_ctl_full                => '0',
      phy_cmd_full                => '0',
      phy_data_full               => '0',
      -- hard PHY calibration modes
      write_calib                 => phy_write_calib,
      read_calib                  => phy_read_calib,
      -- Signals from calib logic to be MUXED with MC
      -- signals before sending to hard PHY
      calib_ctl_wren              => calib_ctl_wren,
      calib_cmd_wren              => calib_cmd_wren,
      calib_seq                   => calib_seq,
      calib_aux_out               => calib_aux_out,
      calib_odt                   => calib_odt,
      calib_cke                   => calib_cke,
      calib_cmd                   => calib_cmd,
      calib_wrdata_en             => calib_wrdata_en,
      calib_rank_cnt              => calib_rank_cnt,
      calib_cas_slot              => calib_cas_slot,
      calib_data_offset_0         => calib_data_offset_0,
      calib_data_offset_1         => calib_data_offset_1,
      calib_data_offset_2         => calib_data_offset_2,
      phy_address                 => phy_address,
      phy_bank                    => phy_bank,
      phy_cs_n                    => phy_cs_n,
      phy_ras_n                   => phy_ras_n,
      phy_cas_n                   => phy_cas_n,
      phy_we_n                    => phy_we_n,
      phy_reset_n                 => phy_reset_n,
      -- DQS count and ck/addr/cmd to be mapped to calib_sel
      -- based on parameter that defines placement of ctl lanes
      -- and DQS byte groups in each bank. When phy_write_calib
      -- is de-asserted calib_sel should select CK/addr/cmd/ctl.
      calib_sel                   => calib_sel,
      calib_in_common             => calib_in_common,
      calib_zero_inputs           => calib_zero_inputs,
      calib_zero_ctrl             => calib_zero_ctrl,
      phy_if_empty_def            => phy_if_empty_def,
      phy_if_reset                => phy_if_reset,
      -- DQS Phaser_IN calibration/status signals
      pi_phaselocked              => pi_phase_locked,
      pi_phase_locked_all         => pi_phase_locked_all,
      pi_found_dqs                => pi_found_dqs,
      pi_dqs_found_all            => pi_dqs_found_all,
      pi_dqs_found_lanes          => dbg_pi_dqs_found_lanes_phy4lanes_i(HIGHEST_LANE-1 downto 0),
      pi_rst_stg1_cal             => rst_stg1_cal,
      pi_en_stg2_f                => pi_enstg2_f,
      pi_stg2_f_incdec            => pi_stg2_fincdec,
      pi_stg2_load                => pi_stg2_load,
      pi_stg2_reg_l               => pi_stg2_reg_l,
      pi_counter_read_val         => pi_counter_read_val,
      device_temp                 => device_temp,
      tempmon_sample_en           => tempmon_sample_en,
      -- IDELAY tap enable and inc signals
      idelay_ce                   => idelay_ce,
      idelay_inc                  => idelay_inc,
      idelay_ld                   => idelay_ld,
      -- DQS Phaser_OUT calibration/status signals
      po_sel_stg2stg3             => po_sel_stg2stg3,
      po_stg2_c_incdec            => po_stg2_cincdec,
      po_en_stg2_c                => po_enstg2_c,
      po_stg2_f_incdec            => po_stg2_fincdec,
      po_en_stg2_f                => po_enstg2_f,
      po_counter_load_en          => po_counter_load_en,
      po_counter_read_val         => po_counter_read_val,
      phy_if_empty                => if_empty,
      idelaye2_init_val           => idelaye2_init_val,
      oclkdelay_init_val          => oclkdelay_init_val,
      tg_err                      => error,
      rst_tg_mc                   => rst_tg_mc,
      phy_wrdata                  => phy_wrdata,
      -- From calib logic To data IN_FIFO
      -- DQ IDELAY tap value from Calib logic
      -- port to be added to mc_phy by Gary
      dlyval_dq                   => open,
      -- From data IN_FIFO To Calib logic and MC/UI
      phy_rddata                  => rd_data_map,
      -- From calib logic To MC
      phy_rddata_valid            => phy_rddata_valid_w,
      calib_rd_data_offset_0      => calib_rd_data_offset_i0,
      calib_rd_data_offset_1      => calib_rd_data_offset_1,
      calib_rd_data_offset_2      => calib_rd_data_offset_2,
      calib_writes                => open,
      -- Mem Init and Calibration status To MC
      init_calib_complete         => phy_init_data_sel,
      init_wrcal_complete         => init_wrcal_complete_i,
      -- Debug Error signals
      pi_phase_locked_err         => dbg_pi_phaselock_err,
      pi_dqsfound_err             => dbg_pi_dqsfound_err,
      wrcal_err                   => dbg_wrcal_err,
      -- Debug Signals
      dbg_pi_phaselock_start      => dbg_pi_phaselock_start,
      dbg_pi_dqsfound_start       => dbg_pi_dqsfound_start,
      dbg_pi_dqsfound_done        => dbg_pi_dqsfound_done,
      dbg_wrcal_start             => dbg_wrcal_start,
      dbg_wrcal_done              => dbg_wrcal_done,
      dbg_wrlvl_start             => dbg_wrlvl_start,
      dbg_wrlvl_done              => dbg_wrlvl_done,
      dbg_wrlvl_err               => dbg_wrlvl_err,
      dbg_wrlvl_fine_tap_cnt      => dbg_wrlvl_fine_tap_cnt,
      dbg_wrlvl_coarse_tap_cnt    => dbg_wrlvl_coarse_tap_cnt,
      dbg_phy_wrlvl               => dbg_phy_wrlvl,
      dbg_tap_cnt_during_wrlvl    => dbg_tap_cnt_during_wrlvl,
      dbg_wl_edge_detect_valid    => dbg_wl_edge_detect_valid,
      dbg_rd_data_edge_detect     => dbg_rd_data_edge_detect,
      dbg_final_po_fine_tap_cnt   => dbg_final_po_fine_tap_cnt,
      dbg_final_po_coarse_tap_cnt => dbg_final_po_coarse_tap_cnt,
      dbg_phy_wrcal               => dbg_phy_wrcal,
      dbg_rdlvl_start             => dbg_rdlvl_start,
      dbg_rdlvl_done              => dbg_rdlvl_done,
      dbg_rdlvl_err               => dbg_rdlvl_err,
      dbg_cpt_first_edge_cnt      => dbg_cpt_first_edge_cnt,
      dbg_cpt_second_edge_cnt     => dbg_cpt_second_edge_cnt,
      dbg_cpt_tap_cnt             => dbg_cpt_tap_cnt,
      dbg_dq_idelay_tap_cnt       => dbg_dq_idelay_tap_cnt,
      dbg_sel_pi_incdec           => dbg_sel_pi_incdec,
      dbg_sel_po_incdec           => dbg_sel_po_incdec,
      dbg_byte_sel                => dbg_byte_sel,
      dbg_pi_f_inc                => dbg_pi_f_inc,
      dbg_pi_f_dec                => dbg_pi_f_dec,
      dbg_po_f_inc                => dbg_po_f_inc,
      dbg_po_f_stg23_sel          => dbg_po_f_stg23_sel,
      dbg_po_f_dec                => dbg_po_f_dec,
      dbg_idel_up_all             => dbg_idel_up_all,
      dbg_idel_down_all           => dbg_idel_down_all,
      dbg_idel_up_cpt             => dbg_idel_up_cpt,
      dbg_idel_down_cpt           => dbg_idel_down_cpt,
      dbg_sel_idel_cpt            => dbg_sel_idel_cpt,
      dbg_sel_all_idel_cpt        => dbg_sel_all_idel_cpt,
      dbg_phy_rdlvl               => dbg_phy_rdlvl,
      dbg_calib_top               => dbg_calib_top,
      dbg_phy_init                => dbg_phy_init,
      dbg_prbs_rdlvl              => dbg_prbs_rdlvl,
      dbg_dqs_found_cal           => dbg_dqs_found_cal,
      dbg_phy_oclkdelay_cal       => dbg_phy_oclkdelay_cal,
      dbg_oclkdelay_rd_data       => dbg_oclkdelay_rd_data,
      dbg_oclkdelay_calib_start   => dbg_oclkdelay_calib_start,
      dbg_oclkdelay_calib_done    => dbg_oclkdelay_calib_done
      );

end architecture arch_ddr_phy_top;
