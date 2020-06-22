----------------------------------------------------------------------------------
----------------------------------------------------------------------------
-- Author:  Mihaita Nagy
--          Copyright 2014 Digilent, Inc.
----------------------------------------------------------------------------
-- 
-- Create Date:    18:48:32 03/05/2013 
-- Design Name: 
-- Module Name:    rgbLed - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
-- This module represents the controller for the RGB Leds. It uses three PWM components
-- to generate the sweeping RGB colors and four debouncers for the incoming buttons
--
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_signed.all;

entity RgbLed is
   port(
      clk_i : in std_logic;
      rstn_i : in std_logic;
      -- Command button signals
      btnl_i : in std_logic;
      btnc_i : in std_logic;
      btnr_i : in std_logic;
      btnd_i : in std_logic;
      -- LD16 PWM output signals
      pwm1_red_o : out std_logic;
      pwm1_green_o : out std_logic;
      pwm1_blue_o : out std_logic;
      -- LD17 PWM output signals
      pwm2_red_o : out std_logic;
      pwm2_green_o : out std_logic;
      pwm2_blue_o : out std_logic;
      -- R, G and B signals connecting to the VGA controller
      -- to be displayed on the screen
      RED_OUT : out std_logic_vector(7 downto 0);
      GREEN_OUT : out std_logic_vector(7 downto 0);
      BLUE_OUT : out std_logic_vector(7 downto 0)
   );
end RgbLed;

architecture Behavioral of RgbLed is

----------------------------------------------------------------------------------
-- Component Declarations
----------------------------------------------------------------------------------
-- Pwm generator
component Pwm is
port(
   clk_i : in std_logic;
   data_i : in std_logic_vector(7 downto 0);
   pwm_o : out std_logic);
end component;

-- Debouncer for the buttons
component Dbncr is
generic(
   NR_OF_CLKS : integer := 4095);
port(
   clk_i : in std_logic;
   sig_i : in std_logic;
   pls_o : out std_logic);
end component;

-- Clock divider, will determine the frequency at which
-- the color components will increase/decrease
-- 100MHz/10000000 = 10Hz
constant CLK_DIV : integer := 10000000;
--Clock divider counter and signal
signal clkCnt : integer := 0;
signal slowClk : std_logic;

-- colorCnt will determine the PWM values to be sent to the RGB Led color components
-- colorCnt(9 downto 5) will be increasing and colorCnt(4 downto 0) will be decreasing
signal colorCnt : std_logic_vector(9 downto 0) := "0000011111";
-- specCnt determines which color components are swept
-- 0: decrease Red, increase Green
-- 1: decrease Green, increase Blue
-- 2: decrease Blue, increase Red
signal specCnt : integer range 0 to 2 := 0;

-- Red, Green and Blue data signals
signal red, green, blue : std_logic_vector(7 downto 0);
-- PWM Red, Green and BLue signals going to the RGB Leds
signal pwm_red, pwm_green, pwm_blue : std_logic;

-- Debounced button signals
signal btnl, btnc, btnr, btnd : std_logic;

-- Signals that turn off LD16 and/or LD17
signal fLed2Off, fLed1Off : std_logic;

-- State machine states definition
type state_type is (stIdle,   -- Both Leds are on, show sweeping color
                     stRed,   -- Show Red color only
                     stGreen, -- Show Green color only
                     stBlue,  -- Show Blue color only
                     stLed2Off, -- Turn off Ld17
                     stLed1Off, -- Turn off Ld16
                     stLed12Off -- Turn off both Leds
                     ); 
-- State machine signal definitions
signal state, nState : state_type;


begin
   
   -- Assign outputs
   pwm1_red_o     <= pwm_red when fLed1Off = '0' else '0';
   pwm1_green_o   <= pwm_green when fLed1Off = '0' else '0';
   pwm1_blue_o    <= pwm_blue when fLed1Off = '0' else '0';
   
   pwm2_red_o     <= pwm_red when fLed2Off = '0' else '0';
   pwm2_green_o   <= pwm_green when fLed2Off = '0' else '0';
   pwm2_blue_o    <= pwm_blue when fLed2Off = '0' else '0';

   RegisterOutputs: process(clk_i, red, green, blue)
   begin
      if rising_edge(clk_i) then
         RED_OUT  <= red;
         GREEN_OUT <= green;
         BLUE_OUT  <= blue;
      end if;
   end process RegisterOutputs;

-- PWM generators:
   PwmRed: Pwm
   port map(
      clk_i    => clk_i,
      data_i   => red,
      pwm_o    => pwm_red);

   PwmGreen: Pwm
   port map(
      clk_i    => clk_i,
      data_i   => green,
      pwm_o    => pwm_green);
   
   PwmBlue: Pwm
   port map(
      clk_i    => clk_i,
      data_i   => blue,
      pwm_o    => pwm_blue);

-- Button Debouncers:
   Btn1: Dbncr
   generic map(
      NR_OF_CLKS  => 4095)
   port map(
      clk_i    => clk_i,
      sig_i    => btnl_i,
      pls_o    => btnl);
   
   Btn2: Dbncr
   generic map(
      NR_OF_CLKS  => 4095)
   port map(
      clk_i    => clk_i,
      sig_i    => btnc_i,
      pls_o    => btnc);
   
   Btn3: Dbncr
   generic map(
      NR_OF_CLKS  => 4095)
   port map(
      clk_i    => clk_i,
      sig_i    => btnr_i,
      pls_o    => btnr);
   
   Btn4: Dbncr
   generic map(
      NR_OF_CLKS  => 4095)
   port map(
      clk_i    => clk_i,
      sig_i    => btnd_i,
      pls_o    => btnd);
   
   -- State machine registerred process
   SYNC_PROC: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if rstn_i = '0' then
            state <= stIdle;
         else
            state <= nState;
         end if;        
      end if;
   end process;
   
   -- Next State decode process
   NEXT_STATE_DECODE: process(state, btnl, btnc, btnr, btnd)
   begin
      nState <= state;  -- Default: Stay in the current state
      case state is
         when stIdle => -- show sweeping color
            if btnl = '1' then
               nState <= stRed;
            elsif btnc = '1' then
               nState <= stGreen;
            elsif btnr = '1' then
               nState <= stBlue;
            elsif btnd = '1' then
               nState <= stLed2Off;
            end if;
         when stRed => -- show red only
            if btnc = '1' then
               nState <= stGreen;
            elsif btnr = '1' then
               nState <= stBlue;
            elsif btnd = '1' then
               nState <= stIdle;
            end if;
         when stGreen => -- show green only
            if btnl = '1' then
               nState <= stRed;
            elsif btnr = '1' then
               nState <= stBlue;
            elsif btnd = '1' then
               nState <= stIdle;
            end if;
         when stBlue => -- show blue only
            if btnl = '1' then
               nState <= stRed;
            elsif btnc = '1' then
               nState <= stGreen;
            elsif btnd = '1' then
               nState <= stIdle;
            end if;
         when stLed2Off => -- turn off Ld17
            if btnd = '1' then
               nState <= stLed1Off;
            end if;
         when stLed1Off => -- turn off Ld16
            if btnd = '1' then
               nState <= stLed12Off;
            end if;
         when stLed12Off => -- turn off both Ld16 and Ld17
            if btnd = '1' then
               nState <= stIdle;
            end if;
         when others => nState <= stIdle;
      end case;      
   end process;
   
-- clock prescaler
   Prescaller: process(clk_i)
   begin
      if rising_edge(clk_i) then
         if rstn_i = '0' then
            clkCnt <= 0;
         elsif clkCnt = CLK_DIV-1 then
            clkCnt <= 0;
         else
            clkCnt <= clkCnt + 1;
         end if;
      end if;
   end process Prescaller;
   
   slowClk <= '1' when clkCnt = CLK_DIV-1 else '0';
      
   process(clk_i)
   begin
      if rising_edge(clk_i) then
         if rstn_i = '0' then
            colorCnt <= b"0000011111";
            specCnt <= 0;
         elsif slowClk = '1' then
            if colorCnt = b"1111000001" then -- at the end of the color sweeping, 
               colorCnt <= b"0000011111";    -- start over and change the colors which are swept
               if specCnt = 2 then 
                  specCnt <= 0;
               else
                  specCnt <= specCnt + 1;
               end if;
            else -- colorCnt (9 downto 5) will be increasing 
                 -- and colorCnt(4 downto 0) will be decreasing
               colorCnt <= colorCnt + b"0000011111";
            end if;
         end if;
      end if;
   end process;
   
   process(state, colorCnt, specCnt, btnl, btnc, btnr)
   begin
      if state = stRed then
         red   <= b"000" & b"11111";
         green <= b"0000" & b"0000";
         blue  <= b"0000" & b"0000";
      elsif state = stGreen then
         red   <= b"0000" & b"0000";
         green <= b"000" & b"11111";
         blue  <= b"0000" & b"0000";
      elsif state = stBlue then
         red   <= b"0000" & b"0000";
         green <= b"0000" & b"0000";
         blue  <= b"000" & b"11111";
      else
         case specCnt is 
            when 0 =>
               red   <= b"000" & colorCnt(4 downto 0); -- Decrease Red, increase Green
               green <= b"000" & colorCnt(9 downto 5);
               blue  <= b"0000" & b"0000";
            when 1 => 
               red   <= b"0000" & b"0000"; 
               green <= b"000" & colorCnt(4 downto 0); -- Decrease Green, increase Blue
               blue  <= b"000" & colorCnt(9 downto 5);
            when 2 => 
               red   <= b"000" & colorCnt(9 downto 5); -- Decrease Blue, increase Red
               green <= b"0000" & b"0000"; 
               blue  <= b"000" & colorCnt(4 downto 0);
            when others => 
               red   <= b"0000" & b"0000"; 
               green <= b"0000" & b"0000"; 
               blue  <= b"0000" & b"0000"; 
         end case;
      end if;
      if state = stLed2Off then
         fLed2Off <= '1';
         fLed1Off <= '0';
      elsif state = stLed1Off then
         fLed2Off <= '0';
         fLed1Off <= '1';
      elsif state = stLed12Off then
         fLed2Off <= '1';
         fLed1Off <= '1';
      else
         fLed2Off <= '0';
         fLed1Off <= '0';
      end if;
   end process;
   
end Behavioral;

