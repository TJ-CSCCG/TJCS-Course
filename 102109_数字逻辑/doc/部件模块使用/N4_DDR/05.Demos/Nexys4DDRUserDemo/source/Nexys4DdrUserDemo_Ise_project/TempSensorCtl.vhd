----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Elod Gyorgy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    15:26:37 02/17/2014 
-- Design Name: 
-- Module Name:	TempSensorCtl - Behavioral 
-- Project Name: 	Nexys4 User Demo
-- Target Devices: 
-- Tool versions: 
-- Description: 
--    This module represents the controller for the Nexys4 onboard ADT7420
--	  temperature sensor. The module uses a Two-Wire Interface controller to
--	  configure the ADT7420 and read the temperature continuously.
--
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


-- Use the package defined in the TWICtl.vhd file
use work.TWIUtils.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity TempSensorCtl is
	Generic (CLOCKFREQ : natural := 100); -- input CLK frequency in MHz
	Port (
		TMP_SCL : inout STD_LOGIC;
		TMP_SDA : inout STD_LOGIC;
--		TMP_INT : in STD_LOGIC; -- Interrupt line from the ADT7420, not used in this project
--		TMP_CT : in STD_LOGIC;  -- Critical Temperature interrupt line from ADT7420, not used in this project
		
		TEMP_O : out STD_LOGIC_VECTOR(12 downto 0); --12-bit two's complement temperature with sign bit
		RDY_O : out STD_LOGIC;	--'1' when there is a valid temperature reading on TEMP_O
		ERR_O : out STD_LOGIC; --'1' if communication error
		
		CLK_I : in STD_LOGIC;
		SRST_I : in STD_LOGIC
	);
end TempSensorCtl;

architecture Behavioral of TempSensorCtl is

-- TWI Controller component declaration
	component TWICtl
   generic 
   (
		CLOCKFREQ : natural := 50;  -- input CLK frequency in MHz
		ATTEMPT_SLAVE_UNBLOCK : boolean := false --setting this true will attempt
		--to drive a few clock pulses for a slave to allow to finish a previous
		--interrupted read transfer, otherwise the bus might remain locked up		
	);
	port (
		MSG_I : in STD_LOGIC; --new message
		STB_I : in STD_LOGIC; --strobe
		A_I : in  STD_LOGIC_VECTOR (7 downto 0); --address input bus
		D_I : in  STD_LOGIC_VECTOR (7 downto 0); --data input bus
		D_O : out  STD_LOGIC_VECTOR (7 downto 0); --data output bus
		DONE_O : out  STD_LOGIC; --done status signal
      ERR_O : out  STD_LOGIC; --error status
		ERRTYPE_O : out error_type; --error type
		CLK : in std_logic;
		SRST : in std_logic;
----------------------------------------------------------------------------------
-- TWI bus signals
----------------------------------------------------------------------------------
		SDA : inout std_logic; --TWI SDA
		SCL : inout std_logic --TWI SCL
	);
end component;

----------------------------------------------------------------------------------
-- Definitions for the I2C initialization vector
----------------------------------------------------------------------------------
	constant IRD : std_logic := '1'; -- init read
	constant IWR : std_logic := '0'; -- init write
	
	constant ADT7420_ADDR : std_logic_vector(7 downto 1)     := "1001011"; -- TWI Slave Address
	constant ADT7420_RID : std_logic_vector(7 downto 0)      := x"0B"; -- ID Register Address for the ADT7420
	constant ADT7420_RRESET : std_logic_vector(7 downto 0)   := x"2F"; -- Software Reset Register
	constant ADT7420_RTEMP : std_logic_vector(7 downto 0)    := x"00"; -- Temperature Read MSB Address
	constant ADT7420_ID : std_logic_vector(7 downto 0)       := x"CB"; -- ADT7420 Manufacturer ID
	
	constant DELAY : NATURAL := 1; --ms
	constant DELAY_CYCLES : NATURAL := natural(ceil(real(DELAY*1000*CLOCKFREQ)));
	constant RETRY_COUNT : NATURAL := 10;

   -- State MAchine states definition
   type state_type is (
                        stIdle, -- Idle State
                        stInitReg,  -- Send register address from the init vector
                        stInitData, -- Send data byte from the init vector
                        stRetry,    -- Retry state reached when there is a bus error, will retry RETRY_COUNT times
                        stReadTempR,  -- Send temperature register address
                        stReadTempD1, -- Read temperature MSB
                        stReadTempD2, -- Read temperature LSB
                        stError -- Error state when reached when there is a bus error after a successful init; stays here until reset
                        ); 
   signal state, nstate : state_type; 

	
   constant NO_OF_INIT_VECTORS : natural := 3; -- number of init vectors in TempSensInitMap
	constant DATA_WIDTH : integer := 1 + 8 + 8; -- RD/WR bit + 1 byte register address + 1 byte data
	constant ADDR_WIDTH : natural := natural(ceil(log(real(NO_OF_INIT_VECTORS), 2.0)));
	
	type TempSensInitMap_type is array (0 to NO_OF_INIT_VECTORS-1) of std_logic_vector(DATA_WIDTH-1 downto 0);
	signal TempSensInitMap: TempSensInitMap_type := (
		IRD & x"0B" & x"CB", -- Read ID R[0x0B]=0xCB
		IWR & x"2F" & x"00", -- Reset R[0x2F]=don't care
		IRD & x"0B" & x"CB" -- Read ID R[0x0B]=0xCB
		);	
	
	signal initWord: std_logic_vector (DATA_WIDTH-1 downto 0);
	signal initA : natural range 0 to NO_OF_INIT_VECTORS := 0; --init vector index
	signal initEn : std_logic;
	
	--Two-Wire Controller signals
	signal twiMsg, twiStb, twiDone, twiErr : std_logic;
	signal twiDi, twiDo, twiAddr : std_logic_vector(7 downto 0);

	--Wait counter used between retry attempts
	signal waitCnt : natural range 0 to DELAY_CYCLES := DELAY_CYCLES;
	signal waitCntEn : std_logic;
   
	--Retry counter to count down attempts to get rid of a bus error
	signal retryCnt : natural range 0 to RETRY_COUNT := RETRY_COUNT;
	signal retryCntEn : std_logic;	
	-- Temporary register to store received data
	signal tempReg : std_logic_vector(15 downto 0) := (others => '0');
	-- Flag indicating that a new temperature data is available
   signal fReady : boolean := false;
begin

----------------------------------------------------------------------------------
-- Outputs
----------------------------------------------------------------------------------
TEMP_O <= tempReg(15 downto 3);

RDY_O <= '1' when fReady else
			'0';
ERR_O <= '1' when state = stError else
			'0';

----------------------------------------------------------------------------------
-- I2C Master Controller
----------------------------------------------------------------------------------
Inst_TWICtl : TWICtl
	generic map (
		ATTEMPT_SLAVE_UNBLOCK => true,
		CLOCKFREQ => 100
	)
	port map (
		MSG_I => twiMsg,
		STB_I => twiStb,
		A_I => twiAddr,
		D_I => twiDi,
		D_O => twiDo,
		DONE_O => twiDone,
      ERR_O => twiErr,
		ERRTYPE_O => open,
		CLK => CLK_I,
		SRST => SRST_I,
		SDA => TMP_SDA,
		SCL => TMP_SCL
	);

----------------------------------------------------------------------------------
-- Initialiation Map RAM
----------------------------------------------------------------------------------
	initWord <= TempSensInitMap(initA);

	InitA_CNT: process (CLK_I) 
	begin
		if Rising_Edge(CLK_I) then
			if (state = stIdle or initA = NO_OF_INIT_VECTORS) then
				initA <= 0;
			elsif (initEn = '1') then
				initA <= initA + 1;
			end if;
		end if;
	end process;
	
----------------------------------------------------------------------------------
-- Delay and Retry Counters
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
	
	Retry_CNT: process (CLK_I) 
	begin
		if Rising_Edge(CLK_I) then
			if (state = stIdle) then
				retryCnt <= RETRY_COUNT;
			elsif (retryCntEn = '1') then
				retryCnt <= retryCnt - 1;
			end if;
		end if;
	end process;

----------------------------------------------------------------------------------
-- Temperature Registers
----------------------------------------------------------------------------------	
	TemperatureReg: process (CLK_I) 
	variable temp : std_logic_vector(7 downto 0);
	begin
		if Rising_Edge(CLK_I) then
			--MSB
			if (state = stReadTempD1 and twiDone = '1' and twiErr = '0') then
				temp := twiDo;
			end if;
			--LSB
			if (state = stReadTempD2 and twiDone = '1' and twiErr = '0') then
				tempReg <= temp & twiDo;
			end if;
		end if;
	end process;
	
----------------------------------------------------------------------------------
-- Ready Flag
----------------------------------------------------------------------------------	
	ReadyFlag: process (CLK_I) 
	begin
		if Rising_Edge(CLK_I) then
			if (state = stIdle or state = stError) then
				fReady <= false;
			elsif (state = stReadTempD2 and twiDone = '1' and twiErr = '0') then
				fReady <= true;
			end if;
		end if;
	end process;	
	
----------------------------------------------------------------------------------
-- Initialization FSM & Continuous temperature read
----------------------------------------------------------------------------------	
   SYNC_PROC: process (CLK_I)
   begin
      if (CLK_I'event and CLK_I = '1') then
         if (SRST_I = '1') then
            state <= stIdle;
         else
            state <= nstate;
         end if;        
      end if;
   end process;
 
   OUTPUT_DECODE: process (state, initWord, twiDone, twiErr, twiDo, retryCnt, waitCnt, initA)
   begin
		twiStb <= '0'; --byte send/receive strobe
		twiMsg <= '0'; --new transfer request (repeated start)
		waitCntEn <= '0'; --wait countdown enable
		twiDi <= "--------"; --byte to send
		twiAddr <= ADT7420_ADDR & '0'; --I2C device address with R/W bit
		initEn <= '0'; --increase init map address
		retryCntEn <= '0'; --retry countdown enable
		
		case (state) is
         when stIdle =>
			
         when stInitReg => --this state sends the register address from the current init vector
            twiStb <= '1';
				twiMsg <= '1';
				twiAddr(0) <= IWR; --Register address is a write
				twiDi <= initWord(15 downto 8);
			when stInitData => --this state sends the data byte from the current init vector
            twiStb <= '1';
				twiAddr(0) <= initWord(initWord'high); --could be read or write
				twiDi <= initWord(7 downto 0);
				if (twiDone = '1' and
					(twiErr = '0' or (initWord(16) = IWR and initWord(15 downto 8) = ADT7420_RRESET)) and
					(initWord(initWord'high) = IWR or twiDo = initWord(7 downto 0))) then
					initEn <= '1';
				end if;
			when stRetry=> --in case of an I2C error during initialization this state will be reached
				if (retryCnt /= 0) then				
					waitCntEn <= '1';
					if (waitCnt = 0) then
						retryCntEn <= '1';
					end if;
				end if;
			
			when stReadTempR => --this state sends the temperature register address
				twiStb <= '1';
				twiMsg <= '1';
				twiDi <= ADT7420_RTEMP;
				twiAddr(0) <= IWR; --Register address is a write				
			when stReadTempD1 => --this state reads the temperature MSB
				twiStb <= '1';
				twiAddr(0) <= IRD;
			when stReadTempD2 => --this state reads the temperature LSB
            twiStb <= '1';
				twiAddr(0) <= IRD;
				
			when stError => --in case of an I2C error during temperature poll
				null; --stay here
		end case;
			
   end process;
 
   NEXT_STATE_DECODE: process (state, twiDone, twiErr, initWord, twiDo, retryCnt, waitCnt)
   begin
      --declare default state for nstate to avoid latches
      nstate <= state;  --default is to stay in current state

      case (state) is
         when stIdle =>
            nstate <= stInitReg;
				
         when stInitReg =>
            if (twiDone = '1') then
					if (twiErr = '1') then
						nstate <= stRetry;
					else
						nstate <= stInitData;
					end if;
				end if;
				
			when stInitData =>
            if (twiDone = '1') then
					if (twiErr = '1') then
						nstate <= stRetry;
					else
						if (initWord(initWord'high) = IRD and twiDo /= initWord(7 downto 0)) then
							nstate <= stRetry;
						elsif (initA = NO_OF_INIT_VECTORS-1) then
							nstate <= stReadTempR;
						else
							nstate <= stInitReg;
						end if;
					end if;
				end if;				
			when stRetry =>
				if (retryCnt = 0) then
					nstate <= stError;
				elsif (waitCnt = 0) then
					nstate <= stInitReg; --new retry attempt
				end if;
				
			when stReadTempR =>
            if (twiDone = '1') then
					if (twiErr = '1') then
						nstate <= stError;
					else
						nstate <= stReadTempD1;
					end if;
				end if;
			when stReadTempD1 =>
            if (twiDone = '1') then
					if (twiErr = '1') then
						nstate <= stError;
					else
						nstate <= stReadTempD2;
					end if;
				end if;
			when stReadTempD2 =>
            if (twiDone = '1') then
					if (twiErr = '1') then
						nstate <= stError;
					else
						nstate <= stReadTempR;
					end if;
				end if;

         when stError =>
				null; --stay
				
         when others =>
            nstate <= stIdle;
      end case;      
   end process;

end Behavioral;

