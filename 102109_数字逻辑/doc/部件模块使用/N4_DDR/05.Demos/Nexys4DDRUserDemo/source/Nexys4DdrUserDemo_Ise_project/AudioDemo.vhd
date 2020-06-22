----------------------------------------------------------------------------
-- Author:  Mihaita Nagy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    15:45:01 02/10/2014 
-- Design Name: 
-- Module Name:    AudioDemo - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
-- Description: 
--    This module represents the Audio Demo for the Nexys4 DDR onboard ADMP421 Omnidirectional Microphone
--    The module consists of several components:
--     - Deserializer, module name PdmDes, that generates the pdm_clk signal for the microphone,
--       receives the microphone data and deserializes it in 16-bit samples. No PDM decoding is done
--
--     - RAM controller, module name RamCntrl, that emulates a RAM Memory Controller and writes the samples to, 
--       or reads from a RAM Memory.
--
--     - RAM to DDR interface and DDR controller, module name Ram2Ddr, that creates a RAM-style interface 
--       for the Nexys4 onboard Micron MT47H64M16HR-25 1Gb (64M X 16) DDR2 memory. The DDR interface was generated 
--       using Memory Interface Generator (MIG) v 1.9. The Ram2Ddr component is controlled by the RamCntrl controller
--       described above and can be also used separately as a memory solution for ISE designs
--
--     - Serializer, module name PdmSer, that receives the samples read from the memory, deserializes and
--       sends the PDM modulated data to the Sallen-Key Butterworth Low Pass 4th Order Filter, from where
--       audio can be listened on the Mono Audio Out connector (J8)
--
--     - Led-Bar, module name LedBar, that generates the progressbar on LD15..LD0 which moves to left 
--       when recording ands to right when playing back
--
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity AudioDemo is
   port (
      -- Common
      clk_i                : in    std_logic;
      clk_200_i            : in    std_logic;
      device_temp_i        : in    std_logic_vector(11 downto 0);
      rst_i                : in    std_logic;

      -- Peripherals      
      btn_u                : in    std_logic;
      leds_o               : out   std_logic_vector(15 downto 0);
      
      -- Microphone PDM signals
      pdm_m_clk_o    : out   std_logic; -- Output M_CLK signal to the microphone
      pdm_m_data_i   : in    std_logic; -- Input PDM data from the microphone
      pdm_lrsel_o    : out   std_logic; -- Set to '0', therefore data is read on the positive edge
      
      -- Audio output signals
      pwm_audio_o    : inout   std_logic; -- Output Audio data to the lowpass filters
      pwm_sdaudio_o  : out   std_logic; -- Output Audio enable

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
      ddr2_dqs_n           : inout std_logic_vector(1 downto 0);

      pdm_clk_rising_o : out std_logic -- Signaling the rising edge of M_CLK, used by the MicDisplay
                                       -- component in the VGA controller
   );
end AudioDemo;

architecture Behavioral of AudioDemo is

------------------------------------------------------------------------
-- Component Declarations
------------------------------------------------------------------------
component Dbncr is
generic(
   NR_OF_CLKS  : integer := 4095);
port(
   clk_i       : in std_logic;
   sig_i       : in std_logic;
   pls_o       : out std_logic);
end component;

-- deserializer
component PdmDes is
   generic(
      C_NR_OF_BITS : integer := 16;
      C_SYS_CLK_FREQ_MHZ : integer := 100;
      C_PDM_FREQ_HZ : integer := 2000000
   );
   port(
      clk_i : in std_logic;
      en_i : in std_logic; -- Enable deserializing (during record)
      
      done_o : out std_logic; -- Signaling that 16 bits are deserialized
      data_o : out std_logic_vector(C_NR_OF_BITS - 1 downto 0); -- output deserialized data
      
      -- PDM
      pdm_m_clk_o : out std_logic; -- Output M_CLK signal to the microphone
      pdm_m_data_i : in std_logic; -- Input PDM data from the microphone
      pdm_lrsel_o : out std_logic; -- Set to '0', therefore data is read on the positive edge
      pdm_clk_rising_o : out std_logic -- Signaling the rising edge of M_CLK, used by the MicDisplay
                                       -- component in the VGA controller
       );
end component;

-- RAM Controller
component RamCntrl is
generic (
   -- read/write cycle (ns)
   C_RW_CYCLE_NS  : integer := 100
);
port (
   -- Control interface
   clk_i          : in  std_logic; -- 100 MHz system clock
   rst_i          : in  std_logic; -- active high system reset
   rnw_i          : in  std_logic; -- read/write
   be_i           : in  std_logic_vector(3 downto 0); -- byte enable
   addr_i         : in  std_logic_vector(31 downto 0); -- address input
   data_i         : in  std_logic_vector(31 downto 0); -- data input
   cs_i           : in  std_logic; -- active high chip select
   data_o         : out std_logic_vector(31 downto 0); -- data output
   rd_ack_o       : out std_logic; -- read acknowledge flag
   wr_ack_o       : out std_logic; -- write acknowledge flag
   
   -- RAM Memory signals
   Mem_A    : out std_logic_vector(26 downto 0); -- Address
   Mem_DQ_O : out std_logic_vector(15 downto 0); -- Data Out
   Mem_DQ_I : in  std_logic_vector(15 downto 0); -- Data In
   Mem_DQ_T : out std_logic_vector(15 downto 0); -- Data Tristate Enable, used for a bidirectional data bus only
   Mem_CEN  : out std_logic; -- Chip Enable
   Mem_OEN  : out std_logic; -- Output Enable
   Mem_WEN  : out std_logic; -- Write Enable
   Mem_UB   : out std_logic; -- Upper Byte
   Mem_LB   : out std_logic -- Lower Byte 
   );
end component;


-- RAM to DDR interface and DDR Controller
component Ram2Ddr is
port (
   -- Common
   clk_200MHz_i   : in    std_logic; -- 200 MHz system clock
   rst_i          : in    std_logic; -- active high system reset
   device_temp_i  : in    std_logic_vector(11 downto 0);
   
   -- RAM interface
   ram_a          : in    std_logic_vector(26 downto 0);
   ram_dq_i       : in    std_logic_vector(15 downto 0);
   ram_dq_o       : out   std_logic_vector(15 downto 0);
   ram_cen        : in    std_logic;
   ram_oen        : in    std_logic;
   ram_wen        : in    std_logic;
   ram_ub         : in    std_logic;
   ram_lb         : in    std_logic;
   
   -- DDR2 interface
   ddr2_addr      : out   std_logic_vector(12 downto 0);
   ddr2_ba        : out   std_logic_vector(2 downto 0);
   ddr2_ras_n     : out   std_logic;
   ddr2_cas_n     : out   std_logic;
   ddr2_we_n      : out   std_logic;
   ddr2_ck_p      : out   std_logic_vector(0 downto 0);
   ddr2_ck_n      : out   std_logic_vector(0 downto 0);
   ddr2_cke       : out   std_logic_vector(0 downto 0);
   ddr2_cs_n      : out   std_logic_vector(0 downto 0);
   ddr2_dm        : out   std_logic_vector(1 downto 0);
   ddr2_odt       : out   std_logic_vector(0 downto 0);
   ddr2_dq        : inout std_logic_vector(15 downto 0);
   ddr2_dqs_p     : inout std_logic_vector(1 downto 0);
   ddr2_dqs_n     : inout std_logic_vector(1 downto 0));
end component;

-- pdm serializer
component PdmSer is
   generic(
      C_NR_OF_BITS : integer := 16;
      C_SYS_CLK_FREQ_MHZ : integer := 100;
      C_PDM_FREQ_HZ : integer := 2000000
   );
   port(
      clk_i : in std_logic;
      en_i : in std_logic; -- Enable serializing (during playback)
      
      done_o : out std_logic; -- Signaling that data_i is sent
      data_i : in std_logic_vector(C_NR_OF_BITS - 1 downto 0); -- input data
      
      -- PWM
      pwm_audio_o : inout std_logic -- Output audio data
   );
end component;

-- led-bar
component LedBar is
generic(
   C_SYS_CLK_FREQ_MHZ  : integer := 100;
   C_SECONDS_TO_RECORD : integer := 3);
port(
   clk_i          : in  std_logic; -- system clock
   en_i           : in  std_logic; -- active-high enable
   rnl_i          : in  std_logic; -- Right/Left shift select
   leds_o         : out std_logic_vector(15 downto 0)); -- output LED bus
end component;

------------------------------------------------------------------------
-- Constant Declarations
------------------------------------------------------------------------
constant SECONDS_TO_RECORD    : integer := 5;
constant PDM_FREQ_HZ          : integer := 2000000;
constant SYS_CLK_FREQ_MHZ     : integer := 100;
constant NR_OF_BITS           : integer := 16;
constant NR_SAMPLES_TO_REC    : integer := (((SECONDS_TO_RECORD*PDM_FREQ_HZ)/NR_OF_BITS) - 1);
constant RW_CYCLE_NS          : integer := 1200;

------------------------------------------------------------------------
-- Local Type Declarations
------------------------------------------------------------------------
type state_type is (stIdle, stRecord, stInter, stPlayback);

------------------------------------------------------------------------
-- Signal Declarations
------------------------------------------------------------------------
signal state, next_state : state_type;

-- common
signal btnu_int         : std_logic;
signal rnw_int          : std_logic;
signal addr_int         : std_logic_vector(31 downto 0);
signal done_int         : std_logic;

signal pwm_audio_o_int  : std_logic;

-- record
signal en_des           : std_logic;
signal done_des         : std_logic;
signal done_async_des   : std_logic;
signal data_des         : std_logic_vector(15 downto 0) := (others => '0');
signal data_dess        : std_logic_vector(31 downto 0) := (others => '0');
signal addr_rec         : std_logic_vector(31 downto 0) := (others => '0');
signal cntRecSamples    : integer := 0;
signal done_des_dly     : std_logic;

-- playback
signal en_ser           : std_logic;
signal done_ser         : std_logic;
signal rd_ack_int       : std_logic;
signal data_ser         : std_logic_vector(31 downto 0);
signal data_serr        : std_logic_vector(15 downto 0);
signal done_async_ser   : std_logic;
signal addr_play        : std_logic_vector(31 downto 0) := (others => '0');
signal cntPlaySamples   : integer := 0;
signal done_ser_dly     : std_logic;

-- led-bar
signal en_leds          : std_logic;
signal rnl_int          : std_logic;

-- memory interconnection signals
signal mem_a            : std_logic_vector(26 downto 0);
signal mem_a_int        : std_logic_vector(26 downto 0);
signal mem_dq_i         : std_logic_vector(15 downto 0);
signal mem_dq_o         : std_logic_vector(15 downto 0);
signal mem_cen          : std_logic;
signal mem_oen          : std_logic;
signal mem_wen          : std_logic;
signal mem_ub           : std_logic;
signal mem_lb           : std_logic;

------------------------------------------------------------------------
-- Module Implementation
------------------------------------------------------------------------
begin
   
   Btnu: Dbncr
   generic map(
   NR_OF_CLKS              => 4095)
   port map(
      clk_i                => clk_i,
      sig_i                => btn_u,
      pls_o                => btnu_int);
   
------------------------------------------------------------------------
-- Deserializer
------------------------------------------------------------------------
   Deserializer: PdmDes
   generic map(
      C_NR_OF_BITS         => NR_OF_BITS,
      C_SYS_CLK_FREQ_MHZ   => SYS_CLK_FREQ_MHZ,
      C_PDM_FREQ_HZ       => PDM_FREQ_HZ)
   port map(
      clk_i                => clk_i,
      en_i                 => en_des,
      done_o               => done_async_des,
      data_o               => data_des,
      pdm_m_clk_o          => pdm_m_clk_o,
      pdm_m_data_i         => pdm_m_data_i,
      pdm_lrsel_o          => pdm_lrsel_o,
      pdm_clk_rising_o     => pdm_clk_rising_o
      );
   
------------------------------------------------------------------------
-- Memory
------------------------------------------------------------------------
   RAM: RamCntrl
   generic map (
      C_RW_CYCLE_NS        => RW_CYCLE_NS)
   port map (
      clk_i                => clk_i,
      rst_i                => rst_i,
      rnw_i                => rnw_int,
      be_i                 => "0011", -- 16-bit access
      addr_i               => addr_int,
      data_i               => data_dess,
      cs_i                 => done_int,
      data_o               => data_ser,
      rd_ack_o             => rd_ack_int,
      wr_ack_o             => open,
      -- RAM Memory signals
      Mem_A                => mem_a,
      Mem_DQ_O             => mem_dq_i,
      Mem_DQ_I             => mem_dq_o,
      Mem_DQ_T             => open,
      Mem_CEN              => mem_cen,
      Mem_OEN              => mem_oen,
      Mem_WEN              => mem_wen,
      Mem_UB               => mem_ub,
      Mem_LB               => mem_lb
      );
   
   DDR: Ram2Ddr
   port map (
      clk_200MHz_i         => clk_200_i,
      rst_i                => rst_i,
      device_temp_i        => device_temp_i,
      -- RAM interface
      ram_a                => mem_a,
      ram_dq_i             => mem_dq_i,
      ram_dq_o             => mem_dq_o,
      ram_cen              => mem_cen,
      ram_oen              => mem_oen,
      ram_wen              => mem_wen,
      ram_ub               => mem_ub,
      ram_lb               => mem_lb,
      -- DDR2 interface
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
      ddr2_dq              => ddr2_dq,
      ddr2_dqs_p           => ddr2_dqs_p,
      ddr2_dqs_n           => ddr2_dqs_n
      );
      
   done_int <= done_des when state = stRecord else
               done_ser when state = stPlayback else '0';
   
------------------------------------------------------------------------
-- Serializer
------------------------------------------------------------------------
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if rd_ack_int = '1' then
            data_serr <= data_ser(15 downto 0);
         end if;
         -- done deserializer
         done_des <= done_async_des;
         -- deserialized data
         data_dess <= x"0000" & data_des;
         -- done serializer
         done_ser <= done_async_ser;
      end if;
   end process;
   
   Serializer: PdmSer
   generic map(
      C_NR_OF_BITS         => NR_OF_BITS,
      C_SYS_CLK_FREQ_MHZ   => SYS_CLK_FREQ_MHZ,
      C_PDM_FREQ_HZ        => PDM_FREQ_HZ)
   port map(
      clk_i                => clk_i,
      en_i                 => en_ser,
      done_o               => done_async_ser,
      data_i               => data_serr,
      pwm_audio_o          => pwm_audio_o
      );
   
   -- count the recorded samples
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if state = stRecord then
            if done_des = '1' then
               cntRecSamples <= cntRecSamples + 1;
            end if;
            if done_des_dly = '1' then
               addr_rec <= addr_rec + "10";
            end if;
         else
            cntRecSamples <= 0;
            addr_rec <= (others => '0');
         end if;
         done_des_dly <= done_des;
      end if;
   end process;

   -- count the played samples
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if state = stPlayback then
            if done_ser = '1' then
               cntPlaySamples <= cntPlaySamples + 1;
            end if;
            if done_ser_dly = '1' then
               addr_play <= addr_play + "10";
            end if;
         else
            cntPlaySamples <= 0;
            addr_play <= (others => '0');
         end if;
         done_ser_dly <= done_ser;
      end if;
   end process;
   

------------------------------------------------------------------------
--  FSM
------------------------------------------------------------------------
   SYNC_PROC: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if rst_i = '1' then
            state <= stIdle;
         else
            state <= next_state;
         end if;        
      end if;
   end process;
 
   --Decode Outputs from the State Machine
   OUTPUT_DECODE: process(clk_i)
   begin
      if rising_edge(clk_i) then
         case (state) is
            when stIdle =>
               rnw_int  <= '0';
               en_ser   <= '0';
               en_des   <= '0';
               addr_int <= (others => '0');
               en_leds  <= '0';
               rnl_int  <= '0';
               pwm_sdaudio_o <= '1';
            when stRecord =>
               rnw_int  <= '0';
               en_ser   <= '0';
               en_des   <= '1';
               addr_int <= addr_rec;
               en_leds  <= '1';
               rnl_int  <= '1';
               pwm_sdaudio_o <= '1';
            when stInter =>
               rnw_int  <= '0';
               en_ser   <= '0';
               en_des   <= '0';
               addr_int <= (others => '0');
               en_leds  <= '0';
               rnl_int  <= '0';
               pwm_sdaudio_o <= '1';
            when stPlayback =>
               rnw_int  <= '1';
               en_ser   <= '1';
               en_des   <= '0';
               addr_int <= addr_play;
               en_leds  <= '1';
               rnl_int  <= '0';
               pwm_sdaudio_o <= '1';
            when others => 
               rnw_int  <= '0';
               en_ser   <= '0';
               en_des   <= '0';
               addr_int <= (others => '0');
               en_leds  <= '0';
               rnl_int  <= '0';
               pwm_sdaudio_o <= '1';
         end case;
      end if;
   end process;
 
   NEXT_STATE_DECODE: process(state, btnu_int, cntRecSamples, cntPlaySamples)
   begin
      next_state <= state;
      case (state) is
         when stIdle =>
            if btnu_int = '1' then
               next_state <= stRecord;
            end if;
         when stRecord =>
            if cntRecSamples = NR_SAMPLES_TO_REC then
               next_state <= stInter;
            end if;
         when stInter =>
            next_state <= stPlayback;
         when stPlayback =>
            if btnu_int = '1' then
               next_state <= stIdle;
            elsif cntPlaySamples = NR_SAMPLES_TO_REC then
               next_state <= stIdle;
            end if;
         when others =>
            next_state <= stIdle;
      end case;
   end process;
   
------------------------------------------------------------------------
--  LED-bar display
------------------------------------------------------------------------
	Leds: LedBar
   generic map(
      C_SYS_CLK_FREQ_MHZ   => SYS_CLK_FREQ_MHZ,
      C_SECONDS_TO_RECORD  => SECONDS_TO_RECORD)
   port map(
      clk_i                => clk_i,
      en_i                 => en_leds,
      rnl_i                => rnl_int,
      leds_o               => leds_o);
   
end Behavioral;

