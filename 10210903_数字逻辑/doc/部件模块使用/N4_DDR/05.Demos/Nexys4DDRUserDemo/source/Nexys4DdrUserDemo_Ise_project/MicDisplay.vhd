----------------------------------------------------------------------------
-- Author:  Sam Bobrowicz, Albert Fazakas
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- Design Name: 
-- Module Name:    MicDisplay - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_unsigned.all;
use IEEE.math_real.all;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity MicDisplay is
    Generic (
           X_WIDTH : integer := 1000;
           Y_HEIGHT : integer := 375;
           X_START : integer range 2 to (Integer'high) := 25;
           Y_START : integer := 512;
           PXLCLK_FREQ_HZ : integer := 108000000;
           H_MAX : integer := 1688;
           SAMPLE_RATE_DIV : integer := 4096;
           BG_COLOR : STD_LOGIC_VECTOR (11 downto 0) := x"FFF";
           ACTIVE_COLOR : STD_LOGIC_VECTOR (11 downto 0) := x"008"
           );
    Port ( CLK_I : in  STD_LOGIC;
           SYSCLK : in STD_LOGIC;
           MIC_M_DATA_I : in  STD_LOGIC;
           MIC_M_CLK_RISING  : IN STD_LOGIC;
           H_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           V_COUNT_I : in  STD_LOGIC_VECTOR (11 downto 0);
           RED_O : out  STD_LOGIC_VECTOR (3 downto 0);
           GREEN_O : out  STD_LOGIC_VECTOR (3 downto 0);
           BLUE_O : out  STD_LOGIC_VECTOR (3 downto 0)
           );
end MicDisplay;

architecture Behavioral of MicDisplay is

--SAMPLE_OFFSET accounts for the offset from center caused by scaling the microphone sensitivity
--Calculated for HEIGHT=375 and sensitivy=3x, accounts for overflow in sample_reg
constant SAMPLE_OFFSET : integer := 105; --95

-- Microphone display frame limits
constant MIC_LEFT				: natural := X_START - 1;
constant MIC_RIGHT			: natural := X_START + X_WIDTH + 1;
constant MIC_TOP				: natural := Y_START - 1;
constant MIC_BOTTOM			: natural := Y_START + Y_HEIGHT + 1;

-----------------------------------------------------------------------------------------------------------
--dependent constants
-----------------------------------------------------------------------------------------------------------
-- Number of bits needed to store the samples
constant SAMPLE_WIDTH : integer := natural(ceil(LOG2(real(Y_HEIGHT))));
constant SR_WIDTH : integer := (Y_HEIGHT);

--    The time needed to draw the microphone display window is Y_HEIGHT * (H_MAX / PXLCLK_FREQ_HZ)
-- in our case 375 * (1688/108000000) = 5.861mS
--    The sample frequency is PXLCLK_FREQ_HZ/SAMPLE_RATE_DIV = 26367.18 Hz, therefore the sample period
-- is 37.92 uS.
--    While the microphone display is active, the samples should be stored in a buffer region which is not
-- read during the microphone display. Therefore we need an extra buffer size called padding.
--    The needed padding size therefore is the time needed to draw the microphone window diwided by 
-- the sample time, i.e. (Y_HEIGHT * H_MAX)/SAMPLE_RATE_DIV  = 154.54 ~ 155
constant BUF_PADDING_NEEDED : natural := natural(ceil((real(Y_HEIGHT) * real(H_MAX))/(real(SAMPLE_RATE_DIV))));

-- Total buffer size needed
constant BUF_DEPTH_NEEDED : natural := X_WIDTH + BUF_PADDING_NEEDED;
-- The sample buffer will be implemented in a BRAM, therefore the buffer size will be a power of two
constant BUF_ADDR_WIDTH : natural := natural(ceil(LOG2(real(BUF_DEPTH_NEEDED))));
-- Size of the whole buffer
constant BUF_DEPTH : integer := 2**BUF_ADDR_WIDTH;
-- Extra buffer size used as buffer padding
constant BUF_PADDING : integer := (BUF_DEPTH - X_WIDTH) ;


-- Create a BRAM to store and display samples
type SAMPLE_MEM_TYPE is array ((BUF_DEPTH - 1) downto 0) of std_logic_vector((SAMPLE_WIDTH - 1) downto 0);
signal sample_buf_ram : SAMPLE_MEM_TYPE;
-- Force BRAM implementation for sample_buf_ram
attribute RAM_STYLE : string;
attribute RAM_STYLE of sample_buf_ram: signal is "BLOCK";
-- RAM Write Enable
signal en_wr_sample_buf_ram : std_logic;

-- Data read from the memory
signal rd_data : std_logic_vector (SAMPLE_WIDTH - 1 downto 0);

-- Memory write address
signal wr_addr_reg : natural range 0 to (BUF_DEPTH - 1) := 0;
-- Memory read current address
signal rd_addr_reg : natural range 0 to (BUF_DEPTH - 1) := 0;
-- Memory read starting address
-- This will be set at BUF_PADDING distance from the write address value
-- at the beginning of drawing the audio frame
signal rd_frame_start_reg : natural range 0 to (BUF_DEPTH - 1) := 0;

-- Synchronize the MIC_M_CLK_RISING signal, coming from the 100MHz Clock Domain
signal mic_m_clk_rising_sync1, mic_m_clk_rising_sync0, mic_m_clk_rising_sync : std_logic;
-- The MIC_M_CLK_RISING signal legth is two SYSCLK (100MHz) periods length,
-- therefore create a one-shot signal
signal mic_m_clk_rising_os : std_logic;

-- Sample window register  - shift register for the microphone data
signal mic_sr_reg : std_logic_vector (SR_WIDTH - 1 downto 0) := (others=>'0');
-- Sample register data
signal sample_reg : std_logic_vector (SAMPLE_WIDTH - 1 downto 0) := (others=>'0');

-- Counter for the sample rate
signal clk_cntr_reg : integer range 0 to SAMPLE_RATE_DIV - 1 := 0; 

-- R, G and B output signal, each on 4 bits
signal color_out : std_logic_vector (11 downto 0);

begin

-------------------------------------------
------------     MIC     ------------------
-------------------------------------------

-- sinchronize MIC_M_CLK_RISING
process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    mic_m_clk_rising_sync1 <= MIC_M_CLK_RISING;
    mic_m_clk_rising_sync0 <= mic_m_clk_rising_sync1;
    mic_m_clk_rising_sync <= mic_m_clk_rising_sync0;    
  end if;
end process;
-- create the one-shot signal for MIC_M_CLK_RISING
mic_m_clk_rising_os <= mic_m_clk_rising_sync0 AND (NOT mic_m_clk_rising_sync);

--Create the sample rate counter
--mclk = 2 MHz, sample rate = 108MHz / 4096 = ~26.367 KHz
process(SYSCLK)
begin
  if (rising_edge(SYSCLK)) then
      if clk_cntr_reg = SAMPLE_RATE_DIV - 1 then
         clk_cntr_reg <= 0;
      else
         clk_cntr_reg <= clk_cntr_reg + 1;
      end if;
  end if;
end process;

--Enable write sample to RAM (triggered every 1/26367 seconds)
en_wr_sample_buf_ram <= '1' when clk_cntr_reg = SAMPLE_RATE_DIV - 1 else '0';

-------------------------------------------
-- Shift in Microphone data
-------------------------------------------
process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    if MIC_M_CLK_RISING_OS = '1' then  
    --Shift new data into the sample window
      mic_sr_reg(0) <= MIC_M_DATA_I;
      mic_sr_reg(SR_WIDTH - 1 downto 1) <= mic_sr_reg((SR_WIDTH - 2) downto 0);
      
    --Monitor the sample window by updating the value of the sample reg each
    --time data is shifted.
      if ((mic_sr_reg(SR_WIDTH - 1) = '0') and (MIC_M_DATA_I = '1')) then
        sample_reg <= sample_reg + 3;
      elsif ((mic_sr_reg(SR_WIDTH - 1) = '1') and (MIC_M_DATA_I = '0')) then
        sample_reg <= sample_reg - 3;
      end if;
    end if;
  end if;
end process;

-------------------------------------------
-- Increment and wrap around write address
-------------------------------------------
process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    if en_wr_sample_buf_ram = '1' then
      if (wr_addr_reg = (BUF_DEPTH - 1)) then
        wr_addr_reg <= 0;
      else
        wr_addr_reg <= wr_addr_reg + 1;
      end if;
    end if;
  end if;
end process;


--------------------------------------------------------------------------------
-- Set the starting RAM read address for this frame.
-- This is done at the beginning of the first line on which the window is drawn.
-- This mechanism prevents tearing of the frame that would occur 
-- if a new audio sample was captured while the frame is being drawn.
--------------------------------------------------------------------------------
process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    if (H_COUNT_I = 0 and V_COUNT_I = Y_START) then
      --Wrap the address if necessary
      if ((wr_addr_reg + BUF_PADDING) >= BUF_DEPTH) then
        rd_frame_start_reg <= (wr_addr_reg + BUF_PADDING) - BUF_DEPTH;
      else
        rd_frame_start_reg <= (wr_addr_reg + BUF_PADDING);
      end if;
    end if;
  end if;
end process;

-------------------------------------------
-- Increment and wrap around read address
-------------------------------------------
process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    if (H_COUNT_I = (X_START - 1)) then
      rd_addr_reg <= rd_frame_start_reg;
    elsif (rd_addr_reg = (BUF_DEPTH - 1)) then
      rd_addr_reg <= 0;
    else
      rd_addr_reg <= rd_addr_reg + 1;
    end if;
  end if;
end process;

------------------------------------------
-- Dual-Port BRAM read and write process
------------------------------------------
process(CLK_I)
begin
  if (rising_edge(CLK_I)) then
    if en_wr_sample_buf_ram = '1' then
      sample_buf_ram(wr_addr_reg) <= sample_reg + SAMPLE_OFFSET;
    end if;
    rd_data <= sample_buf_ram(rd_addr_reg);
  end if;
end process;


color_out <= ACTIVE_COLOR when ((((V_COUNT_I - Y_START) - rd_data) <= 3)  or (((rd_data + Y_START) - V_COUNT_I) <= 3)) else
             BG_COLOR;
-- 
-- Assign Outputs  

RED_O 	<= color_out(11 downto 8) when (H_COUNT_I > MIC_LEFT and H_COUNT_I < MIC_RIGHT)
                                    and (V_COUNT_I < MIC_BOTTOM and V_COUNT_I > MIC_TOP)
				else x"F";
			
GREEN_O	<= color_out(7 downto 4) when (H_COUNT_I > MIC_LEFT and H_COUNT_I < MIC_RIGHT)
                                   and (V_COUNT_I < MIC_BOTTOM and V_COUNT_I > MIC_TOP)
				else x"F";			

BLUE_O	<= color_out(3 downto 0) when (H_COUNT_I > MIC_LEFT and H_COUNT_I < MIC_RIGHT)
                                   and (V_COUNT_I < MIC_BOTTOM and V_COUNT_I > MIC_TOP)
            else x"F";


end Behavioral;
