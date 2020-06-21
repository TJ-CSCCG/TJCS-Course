----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Elod Gyorgy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    17:18:33 02/21/2014 
-- Design Name: 
-- Module Name:    FPGAMonitor - Behavioral 
-- Project Name: 	Nexys4 User Demo
-- Target Devices: 
-- Tool versions: 
-- Description: 
--    This module measures the FPGA temperature using the FPGA internal
--    XADC temperature monitor
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.math_real.all;
use IEEE.std_logic_arith.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
library UNISIM;
use UNISIM.VComponents.all;

entity FPGAMonitor is
	Generic (CLOCKFREQ : natural := 100); -- input CLK frequency in MHz
    Port ( CLK_I : in  STD_LOGIC;
           RST_I : in  STD_LOGIC;
           TEMP_O : out  STD_LOGIC_VECTOR (11 downto 0));
end FPGAMonitor;

architecture Behavioral of FPGAMonitor is

component LocalRst
	 Generic ( RESET_PERIOD : natural := 4);
    Port ( RST_I : in  STD_LOGIC;
           CLK_I : in  STD_LOGIC;
           SRST_O : out  STD_LOGIC);
end component;

	constant DADDR_TEMP : std_logic_vector(6 downto 0) := "0000000";
	constant DELAY : NATURAL := 10; --us
	constant DELAY_CYCLES : NATURAL := 
		natural(ceil(real(DELAY*CLOCKFREQ)));
		
   type state_type is (stIdle, stReadRequest, stReadWait, stRead); 
   signal state, nstate : state_type := stIdle; 
	
	signal x_den, x_drdy, x_drdy_r : std_logic;
	signal x_do, x_do_r : std_logic_vector(15 downto 0);
	
	signal waitCnt : natural range 0 to DELAY_CYCLES := DELAY_CYCLES;
	signal waitCntEn, SysRst : std_logic;
	
begin

----------------------------------------------------------------------------------
-- Sync Reset
----------------------------------------------------------------------------------	
Sync_Reset : LocalRst
	port map (
		RST_I => RST_I,
      CLK_I => CLK_I,
		SRST_O => SysRst
	);
----------------------------------------------------------------------------------
-- Instantiate XADC primitive, single channel (temperature), continuous mode
----------------------------------------------------------------------------------	
 XADC_INST : XADC
     generic map(
        INIT_40 => X"1000", -- config reg 0
        INIT_41 => X"3f3f", -- config reg 1
        INIT_42 => X"0400", -- config reg 2
        INIT_48 => X"0100", -- Sequencer channel selection
        INIT_49 => X"0000", -- Sequencer channel selection
        INIT_4A => X"0000", -- Sequencer Average selection
        INIT_4B => X"0000", -- Sequencer Average selection
        INIT_4C => X"0000", -- Sequencer Bipolar selection
        INIT_4D => X"0000", -- Sequencer Bipolar selection
        INIT_4E => X"0000", -- Sequencer Acq time selection
        INIT_4F => X"0000", -- Sequencer Acq time selection
        INIT_50 => X"b5ed", -- Temp alarm trigger
        INIT_51 => X"57e4", -- Vccint upper alarm limit
        INIT_52 => X"a147", -- Vccaux upper alarm limit
        INIT_53 => X"ca33",  -- Temp alarm OT upper
        INIT_54 => X"a93a", -- Temp alarm reset
        INIT_55 => X"52c6", -- Vccint lower alarm limit
        INIT_56 => X"9555", -- Vccaux lower alarm limit
        INIT_57 => X"ae4e",  -- Temp alarm OT reset
        INIT_58 => X"5999",  -- Vbram upper alarm limit
        INIT_5C => X"5111",  -- Vbram lower alarm limit
        SIM_DEVICE => "7SERIES"
        )

port map (
        CONVST              => '0',
        CONVSTCLK           => '0',
        DADDR(6 downto 0)   => DADDR_TEMP,
        DCLK                => CLK_I,
        DEN                 => x_den,
        DI(15 downto 0)     => x"0000",
        DWE                 => '0',
        RESET               => '0',
        VAUXN(15 downto 0)  => x"0000",
        VAUXP(15 downto 0)  => x"0000",
        ALM                 => open,
        BUSY                => open,
        CHANNEL             => open,
        DO(15 downto 0)     => x_do,
        DRDY                => x_drdy,
        EOC                 => open,
        EOS                 => open,
        JTAGBUSY            => open,
        JTAGLOCKED          => open,
        JTAGMODIFIED        => open,
        OT                  => open,
     
        MUXADDR             => open,
        VN                  => '0',
        VP                  => '0'
         );

----------------------------------------------------------------------------------
-- Register Temperature
----------------------------------------------------------------------------------	
process(CLK_I)
begin
	if Rising_Edge(CLK_I) then
		if (x_drdy_r = '1') then
			TEMP_O <= x_do_r(15 downto 4);
		end if;
	end if;
end process;

----------------------------------------------------------------------------------
-- Register XADC outputs
----------------------------------------------------------------------------------	
process(CLK_I)
begin
	if Rising_Edge(CLK_I) then
		x_do_r <= x_do;
		x_drdy_r <= x_drdy;
	end if;
end process;			
			
----------------------------------------------------------------------------------
-- Delay Counter
----------------------------------------------------------------------------------	
	Wait_CNT: process (CLK_I) 
	begin
		if Rising_Edge(CLK_I) then
			if (waitCntEn = '0') then
				waitCnt <= DELAY_CYCLES;
			else
				waitCnt <= waitCnt - 1;
			end if;
		end if;
	end process;
	
----------------------------------------------------------------------------------
-- Continuous temperature read FSM
----------------------------------------------------------------------------------	
   SYNC_PROC: process (CLK_I)
   begin
      if Rising_Edge(CLK_I) then
         if (SysRst = '1') then
            state <= stIdle;
         else
            state <= nstate;
         end if;        
      end if;
   end process;
 	
   OUTPUT_DECODE: process (state)
   begin
		
		x_den <= '0';
		waitCntEn <= '0';
		
      case (state) is
         when stIdle =>
				waitCntEn <= '1';
				
         when stReadRequest =>
				x_den <= '1';
				
			when others =>
      end case;   
			
   end process;
 
   NEXT_STATE_DECODE: process (state)
   begin
      --declare default state for nstate to avoid latches
      nstate <= state;  --default is to stay in current state

      case (state) is
         when stIdle =>
				if (waitCnt = 0 and x_drdy_r = '0') then
					nstate <= stReadRequest;
				end if;
				
         when stReadRequest =>
				nstate <= stReadWait;
				
			when stReadWait =>
				if (x_drdy_r = '1') then
					nstate <= stIdle;
				end if;
				
         when others =>
            nstate <= stIdle;
      end case;      
   end process;


end Behavioral;

