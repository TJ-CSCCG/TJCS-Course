----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Mihaita Nagy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    14:25:21 04/02/2013 
-- Design Name: 
-- Module Name:    PdmSer - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--       This module represents the serializer for the audio output data. The module generates
--    an internal clk_int signal, having the same frequency as the M_CLK signal sent to the
--    ADMP421 Microphone (see description of the PdmDes component). Data is sent to the audio line
--    at the positive edge of this internal clk signal. 
--       The audio data is filtered on the board by the Sallen-Key Butterworth Low Pass 4th Order Filter
--    and sent to the audio output.
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity PdmSer is
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
end PdmSer;

architecture Behavioral of PdmSer is

------------------------------------------------------------------------
-- Signal Declarations
------------------------------------------------------------------------
-- divider to create clk_int
signal cnt_clk : integer range 0 to 255 := 0;
-- internal pdm_clk signal
signal clk_int : std_logic := '0';

-- Piped clk_int signal to create pdm_clk_rising
signal pdm_clk_rising : std_logic;

-- Shift register used to temporary store then serialize data
signal pdm_s_tmp : std_logic_vector((C_NR_OF_BITS-1) downto 0);
-- Count the number of bits
signal cnt_bits : integer range 0 to 31 := 0;

signal en_int, done_int : std_logic;

------------------------------------------------------------------------
-- Module Implementation
------------------------------------------------------------------------
begin

   -- Register en_i
   SYNC: process(clk_i)
   begin
      if rising_edge(clk_i) then
         en_int <= en_i;
      end if;
   end process SYNC;
   
-- Count the number of sampled bits
   CNT: process(clk_i) begin
      if rising_edge(clk_i) then
         if en_int = '0' then
            cnt_bits <= 0;
         else
            if pdm_clk_rising = '1' then
               if cnt_bits = (C_NR_OF_BITS-1) then
                  cnt_bits <= 0;
               else
                  cnt_bits <= cnt_bits + 1;
               end if;
            end if;
         end if;
      end if;
   end process CNT;
   
-- Generate done_o when the number of bits are serialized
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if pdm_clk_rising = '1' then
            if cnt_bits = (C_NR_OF_BITS-1) then
               done_int <= '1';
            end if;
         else
            done_int <= '0';
         end if;
      end if;
   end process;
   
   done_o <= done_int;
   
------------------------------------------------------------------------
-- Serializer
------------------------------------------------------------------------
   SHFT_OUT: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if pdm_clk_rising = '1' then
            if cnt_bits = 0 then
               pdm_s_tmp <= data_i;
            else
               pdm_s_tmp <= pdm_s_tmp(C_NR_OF_BITS-2 downto 0) & '0';
            end if;
         end if;
      end if;
   end process SHFT_OUT;
   
   -- output the serial pdm data
   pwm_audio_o <= '0' when pdm_s_tmp(C_NR_OF_BITS-1) = '0' else 'Z';

-- Generate the internal PDM Clock
   CLK_CNT: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if en_int = '0' then
            cnt_clk <= 0;
            pdm_clk_rising <= '0';
         else
            if cnt_clk = (((C_SYS_CLK_FREQ_MHZ*1000000)/C_PDM_FREQ_HZ)-1) then
               cnt_clk <= 0;
               pdm_clk_rising <= '1';
            else
               cnt_clk <= cnt_clk + 1;
               pdm_clk_rising <= '0';
            end if;
         end if;
      end if;
   end process CLK_CNT;
   
   
end Behavioral;

