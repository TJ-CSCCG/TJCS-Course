--------------------------------------------------------------------------------
--    DA2 Reference Component
--------------------------------------------------------------------------------
--    Author    :    Ioana Dabacan
--    CopyRight 2008 Digilent Ro.
--------------------------------------------------------------------------------
--    Desription    :    This file is the VHDL code for a PMOD-DA2 controller.
--
--------------------------------------------------------------------------------
--    Revision History:
--    Feb/29/2008 (Created)    Ioana Dabacan
--------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;


--------------------------------------------------------------------------------
--
-- Title      : DA1 controller entity
--
-- Inputs     : 5
-- Outputs    : 5
--
-- Description: This is the DA2 Reference Component entity. The input ports are 
--              a 50MHz clock and and an asynchronous reset button along with the
--              data to be serially shifted in the 2 DAC121S101 chips on a DA2
--              Pmod on each clock cycle.There is also a signal to start a
--              conversion.
--              The outputs of this entity are: a output clock signal, two serial
--              output signals D1 and D2, a sync signal to synchronize the data 
--              in the DAC121S101 chip, a done signal to tell that the chip is 
--              done converting the data and another set of data can be sent.
-- 
---------------------------------------------------------------------------------

entity DA2RefComp is
    Port ( 

     --General usage
       CLK      : in std_logic;    -- System Clock (50MHz)     
       RST      : in std_logic;
     
     --Pmod interface signals
       D1       : out std_logic;
       D2       : out std_logic;
       CLK_OUT  : out std_logic;
       nSYNC    : out std_logic;
        
     --User interface signals
       DATA1    : in std_logic_vector(11 downto 0);
       DATA2    : in std_logic_vector(11 downto 0);
       START    : in std_logic; 
       DONE     : out std_logic
              
        );
end DA2RefComp ;

architecture DA2 of DA2RefComp is


--      control constant: Normal Operation
          constant control     : std_logic_vector(3 downto 0) := "0000";
------------------------------------------------------------------------------------
--    Title      : signal assignments
--
-- Description: The following signals are enumerated signals for the 
--              finite state machine,2 temporary vectors to be shifted out to the 
--              DAC121S101 chips, a divided clock signal to drive the DAC121S101 chips,
--              a counter to divide the internal 50 MHz clock signal,
--              a 4-bit counter to be used to shift out the 16-bit register,
--              and 2 enable signals for the paralel load and shift of the 
--              shift register.
-- 
------------------------------------------------------------------------------------ 
          type states is (Idle,
                          ShiftOut, 
                          SyncData);  
          signal current_state : states;
          signal next_state    : states;
                     
          signal temp1         : std_logic_vector(15 downto 0);
          signal temp2         : std_logic_vector(15 downto 0);           
          signal clk_div       : std_logic;      
          signal clk_counter   : std_logic_vector(27 downto 0);    
          signal shiftCounter  : std_logic_vector(3 downto 0); 
          signal enShiftCounter: std_logic;
          signal enParalelLoad : std_logic;



begin



------------------------------------------------------------------------------------
--
-- Title      : Clock Divider
--
-- Description: The following process takes a 50 MHz clock and divides it down to a
--              25 MHz clock signal by assigning the signals clk_out and clk_div 
--              to the 2nd bit of the clk_counter vector. clk_div is used by
--              the Finite State Machine and clk_out is used by the DAC121S101 chips.
--
------------------------------------------------------------------------------------

        clock_divide : process(rst,clk)
        begin
            if rst = '1' then
                clk_counter <= "0000000000000000000000000000";
            elsif (clk = '1' and clk'event) then
                clk_counter <= clk_counter + '1';
            end if;
        end process;

        clk_div <= clk_counter(0);
        clk_out <= clk_counter(0);


-----------------------------------------------------------------------------------
--
-- Title      : counter
--
-- Description: This is the process were the teporary registers will be loaded and 
--              shifted.When the enParalelLoad signal is generated inside the state 
--              the temp1 and temp2 registers will be loaded with the 8 bits of control
--              concatenated with the 8 bits of data. When the enShiftCounter is 
--              activated, the 16-bits of data inside the temporary registers will be 
--              shifted. A 4-bit counter is used to keep shifting the data 
--              inside temp1 and temp 2 for 16 clock cycles.
--    
-----------------------------------------------------------------------------------    

counter : process(clk_div, enParalelLoad, enShiftCounter)
        begin
            if (clk_div = '1' and clk_div'event) then
               if enParalelLoad = '1' then
                   shiftCounter <= "0000";
                   temp1 <= control & DATA1;
                   temp2 <= control & DATA2;
                elsif (enShiftCounter = '1') then 
                   temp1 <= temp1(14 downto 0)&temp1(15);
                   temp2 <= temp2(14 downto 0)&temp2(15);    
                   shiftCounter <= shiftCounter + '1';
                end if;
            end if;
        end process;

                    D1 <= temp1(15);                             
                    D2 <= temp2(15);

        
---------------------------------------------------------------------------------
--
-- Title      : Finite State Machine
--
-- Description: This 3 processes represent the FSM that contains three states. 
--              First one is the Idle state in which the temporary registers are 
--              assigned the updated value of the input "DATA1" and "DATA2".  
--              The next state is the ShiftOut state which is the state where the 
--              16-bits of temporary registers are shifted out left from the MSB 
--              to the two serial outputs, D1 and D2. Immediately following the 
--              second state is the third state SyncData. This state drives the 
--              output signal sync high for2 clock signals telling the DAC121S101 
--              to latch the 16-bit data it just recieved in the previous state. 
-- Notes:         The data will change on the upper edge of the clock signal. Their 
--                is also an asynchronous reset that will reset all signals to their 
--                original state.
--
-----------------------------------------------------------------------------------        
        
-----------------------------------------------------------------------------------
--
-- Title      : SYNC_PROC
--
-- Description: This is the process were the states are changed synchronously. At 
--              reset the current state becomes Idle state.
--    
-----------------------------------------------------------------------------------            
SYNC_PROC: process (clk_div, rst)
   begin
      if (clk_div'event and clk_div = '1') then
         if (rst = '1') then
            current_state <= Idle;
         else
            current_state <= next_state;
         end if;        
      end if;
   end process;
    
-----------------------------------------------------------------------------------
--
-- Title      : OUTPUT_DECODE
--
-- Description: This is the process were the output signals are generated
--              unsynchronously based on the state only (Moore State Machine).
--    
-----------------------------------------------------------------------------------        
OUTPUT_DECODE: process (current_state)
   begin
      if current_state = Idle then
            enShiftCounter <='0';
            DONE <='1';
            nSYNC <='1';
            enParalelLoad <= '1';
        elsif current_state = ShiftOut then
            enShiftCounter <='1';
            DONE <='0';
            nSYNC <='0';
            enParalelLoad <= '0';
        else --if current_state = SyncData then
            enShiftCounter <='0';
            DONE <='0';
            nSYNC <='1';
            enParalelLoad <= '0';
        end if;
   end process;
    
-----------------------------------------------------------------------------------
--
-- Title      : NEXT_STATE_DECODE
--
-- Description: This is the process were the next state logic is generated 
--              depending on the current state and the input signals.
--    
-----------------------------------------------------------------------------------    
    NEXT_STATE_DECODE: process (current_state, START, shiftCounter)
   begin
      
      next_state <= current_state;  --default is to stay in current state
     
      case (current_state) is
         when Idle =>
            if START = '1' then
               next_state <= ShiftOut;
            end if;
         when ShiftOut =>
            if shiftCounter = x"F" then
               next_state <= SyncData;
            end if;
         when SyncData =>
            if START = '0' then
            next_state <= Idle;
            end if;
         when others =>
            next_state <= Idle;
      end case;      
   end process;
    
            
end DA2;
            
          
