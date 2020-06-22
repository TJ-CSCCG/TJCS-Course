-----------------------------------------------------------------------------
--                                                                 
--  COPYRIGHT (C) 2013, Digilent RO. All rights reserved
--                                                                  
-------------------------------------------------------------------------------
-- FILE NAME      : RamCtrl.vhd
-- MODULE NAME    : Static Random Access Memory Controller
-- AUTHOR         : Mihaita Nagy
-- AUTHOR'S EMAIL : mihaita.nagy@digilent.ro
-------------------------------------------------------------------------------
-- REVISION HISTORY
-- VERSION  DATE         AUTHOR         DESCRIPTION
-- 1.0 	   2011-12-08   Mihaita Nagy   Created
-------------------------------------------------------------------------------
-- DESCRIPTION    : This module implements the state machine to control the
--                  basic read and write procedures of a RAM memory. 
--                  It is also capable of 32-bit access using two consecutive 
--                  16-bit accesses  and 8-bit access as well, using the UB/LB pins.
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity RamCntrl is
   generic (
      -- read/write cycle (ns)
      C_RW_CYCLE_NS : integer := 100
   );
   port (
      -- Control interface
      clk_i    : in  std_logic; -- 100 MHz system clock
      rst_i    : in  std_logic; -- active high system reset
      rnw_i    : in  std_logic; -- read/write
      be_i     : in  std_logic_vector(3 downto 0); -- byte enable
      addr_i   : in  std_logic_vector(31 downto 0); -- address input
      data_i   : in  std_logic_vector(31 downto 0); -- data input
      cs_i     : in  std_logic; -- active high chip select
      data_o   : out std_logic_vector(31 downto 0); -- data output
      rd_ack_o : out std_logic; -- read acknowledge flag
      wr_ack_o : out std_logic; -- write acknowledge flag
      
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
end RamCntrl;

architecture Behavioral of RamCntrl is

------------------------------------------------------------------------
-- Local Type Declarations
------------------------------------------------------------------------
-- State machine state names
type States is(Idle, AssertCen, AssertOenWen, Waitt, Deassert, SendData,
               Ack, Done);

------------------------------------------------------------------------
-- Signal Declarations
------------------------------------------------------------------------
-- State machine signals
signal State, NState: States := Idle;
	
-- For a 32-bit access, two cycles are needed
signal TwoCycle: std_logic := '0';
	
-- Memory LSB
signal AddrLsb: std_logic;
	
-- RnW registerred signal
signal RnwInt: std_logic;
	
-- Byte enable internal bus
signal BeInt: std_logic_vector(3 downto 0);
	
-- Internal registerred Bus2IP_Addr bus
signal AddrInt: std_logic_vector(31 downto 0);
	
-- Internal registerred Bus2IP_Data bus
signal Data2WrInt: std_logic_vector(31 downto 0);
	
-- Internal registerred IP2_Bus bus
signal DataRdInt: std_logic_vector(31 downto 0);
	
-- Integer for the counter of the rd/wr cycle time
signal CntCycleTime: integer range 0 to 255;

signal RstInt : std_logic;

------------------------------------------------------------------------
-- Module Implementation
------------------------------------------------------------------------
begin
   
------------------------------------------------------------------------
-- Register internal signals
------------------------------------------------------------------------
   REGISTER_INT: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if State = Idle then 
            RnwInt <= rnw_i;
            BeInt <= be_i;
            AddrInt <= addr_i;
            Data2WrInt <= data_i;
            RstInt <=rst_i;
         end if;
      end if;
   end process REGISTER_INT;

------------------------------------------------------------------------
-- State Machine
------------------------------------------------------------------------
-- Initialize the state machine
   FSM_REGISTER_STATES: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if RstInt = '1' then
            State <= Idle;
         else
            State <= NState;
         end if;
      end if;
   end process FSM_REGISTER_STATES;
   
-- State machine transitions
   FSM_TRANSITIONS: process(cs_i, TwoCycle, CntCycleTime, State)
   begin
      case State is
         when Idle =>
            if cs_i = '1' then
               NState <= AssertCen;
            else
               NState <= Idle;
            end if;
         when AssertCen => NState <= AssertOenWen;
         when AssertOenWen => NState <= Waitt;
         when Waitt =>
            if CntCycleTime = ((C_RW_CYCLE_NS/10) - 2) then
               NState <= Deassert;
            else
               NState <= Waitt;
            end if;
         when Deassert => NState <= SendData;
         when SendData =>
            if TwoCycle = '1' then
               NState <= AssertCen;
            else
               NState <= Ack;
            end if;
         when Ack => NState <= Done;
         when Done => NState <= Idle;
         when others => Nstate <= Idle;
      end case;
   end process FSM_TRANSITIONS;

------------------------------------------------------------------------
-- Counter for the write/read cycle time
------------------------------------------------------------------------
   CYCLE_COUNTER: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if State = Waitt then
            CntCycleTime <= CntCycleTime + 1;
         else
            CntCycleTime <= 0;
         end if;
      end if;
   end process CYCLE_COUNTER;

------------------------------------------------------------------------
-- Assert CEN
------------------------------------------------------------------------
   ASSERT_CEN: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if State = AssertOenWen or 
            State = Waitt or 
            State = Deassert then
            Mem_CEN <= '0';
         else
            Mem_CEN <= '1';
         end if;
      end if;
   end process ASSERT_CEN;

------------------------------------------------------------------------
-- Assert WEN/OEN
------------------------------------------------------------------------
   ASSERT_WENOEN: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if State = Waitt or State = Deassert then
            if RnwInt = '1' then
               Mem_OEN <= '0';
               Mem_WEN <= '1';
            else
               Mem_OEN <= '1';
               Mem_WEN <= '0';
            end if;
         else
            Mem_OEN <= '1';
            Mem_WEN <= '1';
         end if;
      end if;
   end process ASSERT_WENOEN;

------------------------------------------------------------------------
-- When a 32-bit access mode has to be performed, assert the TwoCycle 
-- signal
------------------------------------------------------------------------
   ASSIGN_TWOCYCLE: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if RstInt = '1' then
            TwoCycle <= '0';
         elsif State = AssertCen and be_i = "1111" then -- 32-bit access
				TwoCycle <= not TwoCycle;
         end if;
      end if;
   end process ASSIGN_TWOCYCLE;

------------------------------------------------------------------------
-- Assign AddrLsb signal
------------------------------------------------------------------------
   ASSIGN_ADDR_LSB: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if RstInt = '1' then
            AddrLsb <= '0';
         elsif State = AssertCen then
            case BeInt is
               -- In 32-bit access: first the lowest address then the highest 
               -- address is written
               when "1111" => AddrLsb <= not TwoCycle;
               -- Higher address
               when "1100"|"0100"|"1000" => AddrLsb <= '1';
               -- Lower address
               when "0011"|"0010"|"0001" => AddrLsb <= '0';
               when others => null;
            end case;
         end if;
      end if;
   end process ASSIGN_ADDR_LSB;

------------------------------------------------------------------------
-- Assign Mem_A
------------------------------------------------------------------------
   ASSIGN_ADDRESS: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if RstInt = '1' then
            Mem_A <= (others => '0');
         elsif State = AssertOenWen or 
               State = Waitt or 
               State = Deassert then
            Mem_A <= AddrInt(26 downto 1) & AddrLsb;
         end if;
      end if;
   end process ASSIGN_ADDRESS;

------------------------------------------------------------------------
-- Assign Mem_DQ_O and Mem_DQ_T
------------------------------------------------------------------------
   ASSIGN_DATA: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if ((State = AssertOenWen or State = Waitt or State = Deassert) and RnwInt = '0') then
            case BeInt is
               when "1111" => 
                  if TwoCycle = '1' then
                     -- Write lowest address with MSdata
                     Mem_DQ_O <= Data2WrInt(15 downto 0);
                  else
                     -- Write highest address with LSdata
                     Mem_DQ_O <= Data2WrInt(31 downto 16);
                  end if;
               when "0011"|"0010"|"0001" => Mem_DQ_O <= Data2WrInt(15 downto 0);
               when "1100"|"1000"|"0100" => Mem_DQ_O <= Data2WrInt(31 downto 16);
               when others => null;
            end case;
         else
            Mem_DQ_O <= (others => '0');
         end if;
      end if;
   end process ASSIGN_DATA;

   Mem_DQ_T <= (others => '1') when RnwInt = '1' else (others => '0');

------------------------------------------------------------------------
-- Read data from the memory
------------------------------------------------------------------------
   READ_DATA: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if RstInt = '1' then
            DataRdInt <= (others => '0');
         elsif State = Deassert then
            case BeInt is
               when "1111" => 
                  if TwoCycle = '1' then
                     -- Read lowest address with MSdata
                     DataRdInt(15 downto 0) <= Mem_DQ_I;
                  else
                     -- Read highest address with LSdata
                     DataRdInt(31 downto 16) <= Mem_DQ_I;
                  end if;
               -- Perform data mirroring
               when "0011"|"1100" => 
                  DataRdInt(15 downto 0)  <= Mem_DQ_I;
                  DataRdInt(31 downto 16) <= Mem_DQ_I;
               when "0100"|"0001" => 
                  DataRdInt(7 downto 0)   <= Mem_DQ_I(7 downto 0);
                  DataRdInt(15 downto 8)  <= Mem_DQ_I(7 downto 0);
                  DataRdInt(23 downto 16) <= Mem_DQ_I(7 downto 0);
                  DataRdInt(31 downto 24) <= Mem_DQ_I(7 downto 0);
               when "1000"|"0010" => 
                  DataRdInt(7 downto 0)   <= Mem_DQ_I(15 downto 8);
                  DataRdInt(15 downto 8)  <= Mem_DQ_I(15 downto 8);
                  DataRdInt(23 downto 16) <= Mem_DQ_I(15 downto 8);
                  DataRdInt(31 downto 24) <= Mem_DQ_I(15 downto 8);
               when others => null;
            end case;
         end if;
      end if;
   end process READ_DATA;

------------------------------------------------------------------------
-- Send data to AXI bus
------------------------------------------------------------------------
   REGISTER_DREAD: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if State = Ack then
            data_o <= DataRdInt;
         end if;
      end if;
   end process REGISTER_DREAD;

------------------------------------------------------------------------
-- Assign acknowledge signals
------------------------------------------------------------------------
   REGISTER_ACK: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if State = Ack and TwoCycle = '0' then
            if RnwInt = '1' then -- read
               rd_ack_o <= '1';
               wr_ack_o <= '0';
            else -- write
               rd_ack_o <= '0';
               wr_ack_o <= '1';
            end if;
         else
            rd_ack_o <= '0';
            wr_ack_o <= '0';
         end if;
      end if;
   end process REGISTER_ACK;

------------------------------------------------------------------------
-- Assign UB, LB (used in 8-bit write mode)
------------------------------------------------------------------------
   ASSIGN_UB_LB: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if RnwInt = '0' then
            if State = AssertOenWen or 
               State = Waitt or 
               State = Deassert then
               case BeInt is
                  -- Disable lower byte when MSByte is written
                  when "1000"|"0010" => 
                     Mem_UB <= '0'; 
                     Mem_LB <= '1';
                  -- Disable upper byte when LSByte is written
                  when "0100"|"0001" => 
                     Mem_UB <= '1'; 
                     Mem_LB <= '0'; 
                  -- Enable both bytes in other modes
                  when others => 
                     Mem_UB <= '0';
                     Mem_LB <= '0';
               end case;
            end if;
         else -- Enable both when reading
            Mem_UB <= '0'; 
            Mem_LB <= '0';
         end if;
      end if;
   end process ASSIGN_UB_LB;

end Behavioral;
