-------------------------------------------------------------------------------
--                                                                 
--  COPYRIGHT (C) 2014, Digilent RO. All rights reserved
--                                                                  
-------------------------------------------------------------------------------
-- FILE NAME      : ram2ddr.vhd
-- MODULE NAME    : RAM to DDR2 Interface Converter with internal XADC
--                  instantiation
-- AUTHOR         : Mihaita Nagy
-- AUTHOR'S EMAIL : mihaita.nagy@digilent.ro
-------------------------------------------------------------------------------
-- REVISION HISTORY
-- VERSION  DATE         AUTHOR         DESCRIPTION
-- 1.0      2014-02-04   Mihaita Nagy   Created
-- 1.1      2014-04-04   Mihaita Nagy   Fixed double registering write bug
-------------------------------------------------------------------------------
-- DESCRIPTION    : This module implements a simple Static RAM to DDR2 interface
--                  converter designed to be used with Digilent Nexys4-DDR board
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

------------------------------------------------------------------------
-- Module Declaration
------------------------------------------------------------------------
entity Ram2Ddr is
   port (
      -- Common
      clk_200MHz_i         : in    std_logic; -- 200 MHz system clock
      rst_i                : in    std_logic; -- active high system reset
      device_temp_i        : in    std_logic_vector(11 downto 0);
      
      -- RAM interface
      ram_a                : in    std_logic_vector(26 downto 0);
      ram_dq_i             : in    std_logic_vector(15 downto 0);
      ram_dq_o             : out   std_logic_vector(15 downto 0);
      ram_cen              : in    std_logic;
      ram_oen              : in    std_logic;
      ram_wen              : in    std_logic;
      ram_ub               : in    std_logic;
      ram_lb               : in    std_logic;
      
      -- DDR2 interface
      ddr2_addr            : out   std_logic_vector(12 downto 0);
      ddr2_ba              : out   std_logic_vector(2 downto 0);
      ddr2_ras_n           : out   std_logic;
      ddr2_cas_n           : out   std_logic;
      ddr2_we_n            : out   std_logic;
      ddr2_ck_p            : out   std_logic_vector(0 downto 0);
      ddr2_ck_n            : out   std_logic_vector(0 downto 0);
      ddr2_cke             : out   std_logic_vector(0 downto 0);
      ddr2_cs_n            : out   std_logic_vector(0 downto 0);
      ddr2_dm              : out   std_logic_vector(1 downto 0);
      ddr2_odt             : out   std_logic_vector(0 downto 0);
      ddr2_dq              : inout std_logic_vector(15 downto 0);
      ddr2_dqs_p           : inout std_logic_vector(1 downto 0);
      ddr2_dqs_n           : inout std_logic_vector(1 downto 0)
   );
end Ram2Ddr;

architecture Behavioral of Ram2Ddr is

------------------------------------------------------------------------
-- Component Declarations
------------------------------------------------------------------------
component ddr
port (
   -- Inouts
   ddr2_dq              : inout std_logic_vector(15 downto 0);
   ddr2_dqs_p           : inout std_logic_vector(1 downto 0);
   ddr2_dqs_n           : inout std_logic_vector(1 downto 0);
   -- Outputs
   ddr2_addr            : out   std_logic_vector(12 downto 0);
   ddr2_ba              : out   std_logic_vector(2 downto 0);
   ddr2_ras_n           : out   std_logic;
   ddr2_cas_n           : out   std_logic;
   ddr2_we_n            : out   std_logic;
   ddr2_ck_p            : out   std_logic_vector(0 downto 0);
   ddr2_ck_n            : out   std_logic_vector(0 downto 0);
   ddr2_cke             : out   std_logic_vector(0 downto 0);
   ddr2_cs_n            : out   std_logic_vector(0 downto 0);
   ddr2_dm              : out   std_logic_vector(1 downto 0);
   ddr2_odt             : out   std_logic_vector(0 downto 0);
   -- Inputs
   sys_clk_i            : in    std_logic;
   sys_rst              : in    std_logic;
   -- user interface signals
   app_addr             : in    std_logic_vector(26 downto 0);
   app_cmd              : in    std_logic_vector(2 downto 0);
   app_en               : in    std_logic;
   app_wdf_data         : in    std_logic_vector(127 downto 0);
   app_wdf_end          : in    std_logic;
   app_wdf_mask         : in    std_logic_vector(15 downto 0);
   app_wdf_wren         : in    std_logic;
   app_rd_data          : out   std_logic_vector(127 downto 0);
   app_rd_data_end      : out   std_logic;
   app_rd_data_valid    : out   std_logic;
   app_rdy              : out   std_logic;
   app_wdf_rdy          : out   std_logic;
   app_sr_req           : in    std_logic;
   app_sr_active        : out   std_logic;
   app_ref_req          : in    std_logic;
   app_ref_ack          : out   std_logic;
   app_zq_req           : in    std_logic;
   app_zq_ack           : out   std_logic;
   ui_clk               : out   std_logic;
   ui_clk_sync_rst      : out   std_logic;
   device_temp_i        : in    std_logic_vector(11 downto 0);
   init_calib_complete  : out   std_logic);
end component;

------------------------------------------------------------------------
-- Local Type Declarations
------------------------------------------------------------------------
-- FSM
type state_type is (stIdle, stPreset, stSendData, stSetCmdRd, stSetCmdWr,
                    stWaitCen);

------------------------------------------------------------------------
-- Constant Declarations
------------------------------------------------------------------------
-- ddr commands
constant CMD_WRITE         : std_logic_vector(2 downto 0) := "000";
constant CMD_READ          : std_logic_vector(2 downto 0) := "001";

------------------------------------------------------------------------
-- Signal Declarations
------------------------------------------------------------------------
-- state machine
signal cState, nState      : state_type; 

-- global signals
signal mem_ui_clk          : std_logic;
signal mem_ui_rst          : std_logic;
signal rst                 : std_logic;
signal rstn                : std_logic;
signal sreg                : std_logic_vector(1 downto 0);

-- ram internal signals
signal ram_a_int           : std_logic_vector(26 downto 0);
signal ram_dq_i_int        : std_logic_vector(15 downto 0);
signal ram_cen_int         : std_logic;
signal ram_oen_int         : std_logic;
signal ram_wen_int         : std_logic;
signal ram_ub_int          : std_logic;
signal ram_lb_int          : std_logic;

-- ddr user interface signals
signal mem_addr            : std_logic_vector(26 downto 0); -- address for current request
signal mem_cmd             : std_logic_vector(2 downto 0); -- command for current request
signal mem_en              : std_logic; -- active-high strobe for 'cmd' and 'addr'
signal mem_rdy             : std_logic;
signal mem_wdf_rdy         : std_logic; -- write data FIFO is ready to receive data (wdf_rdy = 1 & wdf_wren = 1)
signal mem_wdf_data        : std_logic_vector(127 downto 0);
signal mem_wdf_end         : std_logic; -- active-high last 'wdf_data'
signal mem_wdf_mask        : std_logic_vector(15 downto 0);
signal mem_wdf_wren        : std_logic;
signal mem_rd_data         : std_logic_vector(127 downto 0);
signal mem_rd_data_end     : std_logic; -- active-high last 'rd_data'
signal mem_rd_data_valid   : std_logic; -- active-high 'rd_data' valid
signal calib_complete      : std_logic; -- active-high calibration complete

------------------------------------------------------------------------
-- Signal attributes (debugging)
------------------------------------------------------------------------
attribute FSM_ENCODING              : string;
attribute FSM_ENCODING of cState    : signal is "GRAY";

attribute ASYNC_REG                 : string;
attribute ASYNC_REG of sreg         : signal is "TRUE";

------------------------------------------------------------------------
-- Module Implementation
------------------------------------------------------------------------
begin
   
------------------------------------------------------------------------
-- Registering the active-low reset for the MIG component
------------------------------------------------------------------------
   RSTSYNC: process(clk_200MHz_i)
   begin
      if rising_edge(clk_200MHz_i) then
         sreg <= sreg(0) & rst_i;
         rstn <= not sreg(1);
      end if;
   end process RSTSYNC;
   
------------------------------------------------------------------------
-- DDR controller instance
------------------------------------------------------------------------
   Inst_DDR: ddr
   port map (
      ddr2_dq              => ddr2_dq,
      ddr2_dqs_p           => ddr2_dqs_p,
      ddr2_dqs_n           => ddr2_dqs_n,
      ddr2_addr            => ddr2_addr,
      ddr2_ba              => ddr2_ba,
      ddr2_ras_n           => ddr2_ras_n,
      ddr2_cas_n           => ddr2_cas_n,
      ddr2_we_n            => ddr2_we_n,
      ddr2_ck_p            => ddr2_ck_p,
      ddr2_ck_n            => ddr2_ck_n,
      ddr2_cke             => ddr2_cke,
      ddr2_cs_n            => ddr2_cs_n,
      ddr2_dm              => ddr2_dm,
      ddr2_odt             => ddr2_odt,
      -- Inputs
      sys_clk_i            => clk_200MHz_i,
      sys_rst              => rstn,
      -- user interface signals
      app_addr             => mem_addr,
      app_cmd              => mem_cmd,
      app_en               => mem_en,
      app_wdf_data         => mem_wdf_data,
      app_wdf_end          => mem_wdf_end,
      app_wdf_mask         => mem_wdf_mask,
      app_wdf_wren         => mem_wdf_wren,
      app_rd_data          => mem_rd_data,
      app_rd_data_end      => mem_rd_data_end,
      app_rd_data_valid    => mem_rd_data_valid,
      app_rdy              => mem_rdy,
      app_wdf_rdy          => mem_wdf_rdy,
      app_sr_req           => '0',
      app_sr_active        => open,
      app_ref_req          => '0',
      app_ref_ack          => open,
      app_zq_req           => '0',
      app_zq_ack           => open,
      ui_clk               => mem_ui_clk,
      ui_clk_sync_rst      => mem_ui_rst,
      device_temp_i        => device_temp_i,
      init_calib_complete  => calib_complete);

------------------------------------------------------------------------
-- Registering all inputs of the state machine to 'mem_ui_clk' domain
------------------------------------------------------------------------
   REG_IN: process(mem_ui_clk)
   begin
      if rising_edge(mem_ui_clk) then
         ram_a_int <= ram_a;
         ram_dq_i_int <= ram_dq_i;
         ram_cen_int <= ram_cen;
         ram_oen_int <= ram_oen;
         ram_wen_int <= ram_wen;
         ram_ub_int <= ram_ub;
         ram_lb_int <= ram_lb;
      end if;
   end process REG_IN;
   
------------------------------------------------------------------------
-- State Machine
------------------------------------------------------------------------
-- Register states
   SYNC_PROCESS: process(mem_ui_clk)
   begin
      if rising_edge(mem_ui_clk) then
         if mem_ui_rst = '1' then
            cState <= stIdle;
         else
            cState <= nState;
         end if;
      end if;
   end process SYNC_PROCESS;

-- Next state logic
   NEXT_STATE_DECODE: process(cState, calib_complete, ram_cen_int, 
   mem_rdy, mem_wdf_rdy)
   begin
      nState <= cState;
      case(cState) is
         -- If calibration is done successfully and CEN is
         -- deasserted then start a new transaction
         when stIdle =>
            if ram_cen_int = '0' and 
               calib_complete = '1' then
               nState <= stPreset;
            end if;
         -- In this state we store the address and data to
         -- be written or the address to read from. We need
         -- this additional state to make sure that all input
         -- transitions are fully settled and registered
         when stPreset =>
            if ram_wen_int = '0' then
               nState <= stSendData;
            elsif ram_oen_int = '0' then
               nState <= stSetCmdRd;
            end if;
         -- In a write transaction the data it written first
         -- giving higher priority to 'mem_wdf_rdy' frag over
         -- 'mem_rdy'
         when stSendData =>
            if mem_wdf_rdy = '1' then
               nState <= stSetCmdWr;
            end if;
         -- Sending the read command and wait for the 'mem_rdy'
         -- frag to be asserted (in case it's not)
         when stSetCmdRd =>
            if mem_rdy = '1' then
               nState <= stWaitCen;
            end if;
         -- Sending the write command after the data has been
         -- written to the controller FIFO and wait ro the
         -- 'mem_rdy' frag to be asserted (in case it's not)
         when stSetCmdWr =>
            if mem_rdy = '1' then
               nState <= stWaitCen;
            end if;
         -- After sending all the control signals and data, we
         -- wait for the external CEN to signal transaction
         -- end
         when stWaitCen =>
            if ram_cen_int = '1' then
               nState <= stIdle;
            end if;
         when others => nState <= stIdle;            
      end case;      
   end process;

------------------------------------------------------------------------
-- Generating the FIFO control and command signals according to the 
-- current state of the FSM
------------------------------------------------------------------------
   MEM_WR_CTL: process(cState)
   begin
      if cState = stSendData then
         mem_wdf_wren <= '1';
         mem_wdf_end <= '1';
      else
         mem_wdf_wren <= '0';
         mem_wdf_end <= '0';
      end if;
   end process MEM_WR_CTL;
   
   MEM_CTL: process(cState)
   begin
      if cState = stSetCmdRd then
         mem_en <= '1';
         mem_cmd <= CMD_READ;
      elsif cState = stSetCmdWr then
         mem_en <= '1';
         mem_cmd <= CMD_WRITE;
      else
         mem_en <= '0';
         mem_cmd <= (others => '0');
      end if;
   end process MEM_CTL;
   
------------------------------------------------------------------------
-- Decoding the least significant 3 bits of the address and creating
-- accordingly the 'mem_wdf_mask'
------------------------------------------------------------------------
   WR_DATA_MSK: process(mem_ui_clk)
   begin
      if rising_edge(mem_ui_clk) then
         if cState = stPreset then
            case(ram_a_int(3 downto 1)) is
               when "000" =>
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "1111111111111101";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1111111111111110";
                  else -- 16-bit
                     mem_wdf_mask <= "1111111111111100";
                  end if;
               when "001" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "1111111111110111";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1111111111111011";
                  else -- 16-bit
                     mem_wdf_mask <= "1111111111110011";
                  end if;
               when "010" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "1111111111011111";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1111111111101111";
                  else -- 16-bit
                     mem_wdf_mask <= "1111111111001111";
                  end if;
               when "011" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "1111111101111111";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1111111110111111";
                  else -- 16-bit
                     mem_wdf_mask <= "1111111100111111";
                  end if;
               when "100" =>
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "1111110111111111";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1111111011111111";
                  else -- 16-bit
                     mem_wdf_mask <= "1111110011111111";
                  end if;
               when "101" =>
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "1111011111111111";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1111101111111111";
                  else -- 16-bit
                     mem_wdf_mask <= "1111001111111111";
                  end if;
               when "110" =>
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "1101111111111111";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1110111111111111";
                  else -- 16-bit
                     mem_wdf_mask <= "1100111111111111";
                  end if;
               when "111" =>
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     mem_wdf_mask <= "0111111111111111";
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     mem_wdf_mask <= "1011111111111111";
                  else -- 16-bit
                     mem_wdf_mask <= "0011111111111111";
                  end if;
               when others => null;
            end case;
         end if;
      end if;
   end process WR_DATA_MSK;
   
------------------------------------------------------------------------
-- Registering write data and read/write address
------------------------------------------------------------------------
   WR_DATA_ADDR: process(mem_ui_clk)
   begin
      if rising_edge(mem_ui_clk) then
         if cState = stPreset then
            mem_wdf_data <= ram_dq_i_int & ram_dq_i_int & ram_dq_i_int & 
                            ram_dq_i_int & ram_dq_i_int & ram_dq_i_int &
                            ram_dq_i_int & ram_dq_i_int;
         end if;
      end if;
   end process WR_DATA_ADDR;

   WR_ADDR: process(mem_ui_clk)
   begin
      if rising_edge(mem_ui_clk) then
         if cState = stPreset then
            mem_addr <= ram_a_int(26 downto 4) & "0000";
         end if;
      end if;
   end process WR_ADDR;

------------------------------------------------------------------------
-- Mask and output the read data from the FIFO
------------------------------------------------------------------------
   RD_DATA: process(mem_ui_clk)
   begin
      if rising_edge(mem_ui_clk) then
         if cState = stWaitCen and mem_rd_data_valid = '1' and 
            mem_rd_data_end = '1' then
            case(ram_a_int(3 downto 1)) is
               when "000" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(15 downto 8) & 
                                 mem_rd_data(15 downto 8);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(7 downto 0) & 
                                 mem_rd_data(7 downto 0);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(15 downto 0);
                  end if;
               when "001" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(31 downto 24) & 
                                 mem_rd_data(31 downto 24);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(23 downto 16) & 
                                 mem_rd_data(23 downto 16);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(31 downto 16);
                  end if;
               when "010" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(47 downto 40) & 
                                 mem_rd_data(47 downto 40);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(39 downto 32) & 
                                 mem_rd_data(39 downto 32);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(47 downto 32);
                  end if;
               when "011" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(63 downto 56) & 
                                 mem_rd_data(63 downto 56);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(55 downto 48) & 
                                 mem_rd_data(55 downto 48);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(63 downto 48);
                  end if;
               when "100" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(79 downto 72) & 
                                 mem_rd_data(79 downto 72);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(71 downto 64) & 
                                 mem_rd_data(71 downto 64);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(79 downto 64);
                  end if;
               when "101" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(95 downto 88) & 
                                 mem_rd_data(95 downto 88);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(87 downto 80) & 
                                 mem_rd_data(87 downto 80);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(95 downto 80);
                  end if;
               when "110" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(111 downto 104) & 
                                 mem_rd_data(111 downto 104);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(103 downto 96) & 
                                 mem_rd_data(103 downto 96);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(111 downto 96);
                  end if;
               when "111" => 
                  if ram_ub_int = '0' and ram_lb_int = '1' then -- UB
                     ram_dq_o <= mem_rd_data(127 downto 120) & 
                                 mem_rd_data(127 downto 120);
                  elsif ram_ub_int = '1' and ram_lb_int = '0' then -- LB
                     ram_dq_o <= mem_rd_data(119 downto 112) & 
                                 mem_rd_data(119 downto 112);
                  else -- 16-bit
                     ram_dq_o <= mem_rd_data(127 downto 112);
                  end if;
               when others => null;
            end case;
         end if;
      end if;
   end process RD_DATA;

end Behavioral;

