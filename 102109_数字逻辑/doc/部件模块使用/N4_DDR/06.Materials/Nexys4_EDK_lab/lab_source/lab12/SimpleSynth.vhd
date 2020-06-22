----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    15:17:08 12/16/2010 
-- Design Name: 
-- Module Name:    SimpleSynth - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
-- 	Gets a tone code (0 to 31)
--		assign 0 to silence and other values to dividers.
--		Divides the system clock to get a base frequency 100 time higher than 
--		the needed audio frequency.
--		Generates synus, triangle, rectangle and sawtooth waveshapes.
--		Enables one of above to the audioSample output, based on mode pins

-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.std_logic_signed.all;	-- add to do arithmetic operations
use IEEE.std_logic_arith.all;		-- add to do arithmetic operations

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity SimpleSynth is
    Port ( ck : in  STD_LOGIC;
           tone : in  std_logic_vector (4 downto 0);
           mode : in  STD_LOGIC_VECTOR (2 downto 0);
           audioSample : out  std_logic_vector (7 downto 0)	-- audio signal samples
			  );
end SimpleSynth;

architecture Behavioral of SimpleSynth is

	signal count  : integer range 0 to 8191;	-- divider counter 
	signal divider: integer range 0 to 8191;	-- division ratio
	signal div_clk: std_logic;						-- divided clock

	signal x: std_logic_vector (7 downto 0) := "01100100";	-- cosinus samples
	signal y: std_logic_vector (7 downto 0) := "00000000";	-- sinus samples
	signal xt: std_logic_vector (7 downto 0):= "00000000";		-- triangle samples
	signal xr: std_logic_vector (7 downto 0);		-- rectangle samples
	signal xs: std_logic_vector (7 downto 0):= "00000000";		-- sawtooth samples
	signal sense: std_logic;	-- triangle slope, '1' for rising
	signal Cos2, tri2: std_logic_vector (15 downto 0);	-- cosinus^2 samples

begin

	with tone select
	divider <=
		2703	when	"00001"	,	 --	F#	184.9972114	Hz
		2551	when	"00010"	,	 --	G	195.997718	Hz
		2408	when	"00011"	,	 --	Ab	207.6523488	Hz
		2273	when	"00100"	,	 --	A	220	Hz
		2145	when	"00101"	,	 --	Bb	233.0818808	Hz
		2025	when	"00110"	,	 --	B	246.9416506	Hz
		1911	when	"00111"	,	 --	C	261.6255653	Hz
		1804	when	"01000"	,	 --	C#	277.182631	Hz
		1703	when	"01001"	,	 --	D	293.6647679	Hz
		1607	when	"01010"	,	 --	D#	311.1269837	Hz
		1517	when	"01011"	,	 --	E	329.6275569	Hz
		1432	when	"01100"	,	 --	F	349.2282314	Hz
		1351	when	"01101"	,	 --	F#	369.9944227	Hz
		1276	when	"01110"	,	 --	G	391.995436	Hz
		1204	when	"01111"	,	 --	Ab	415.3046976	Hz
		1136	when	"10000"	,	 --	A	440	Hz
		1073	when	"10001"	,	 --	Bb	466.1637615	Hz
		1012	when	"10010"	,	 --	B	493.8833013	Hz
		956	when	"10011"	,	 --	C	523.2511306	Hz
		902	when	"10100"	,	 --	C#	554.365262	Hz
		851	when	"10101"	,	 --	D	587.3295358	Hz
		804	when	"10110"	,	 --	D#	622.2539674	Hz
		758	when	"10111"	,	 --	E	659.2551138	Hz
		716	when	"11000"	,	 --	F	698.4564629	Hz
		676	when	"11001"	,	 --	F#	739.9888454	Hz
		638	when	"11010"	,	 --	G	783.990872	Hz
		602	when	"11011"	,	 --	Ab	830.6093952	Hz
		568	when	"11100"	,	 --	A	880	Hz
		536	when	"11101"	,	 --	Bb	932.327523	Hz
		506	when	"11110"	,	 --	B	987.7666025	Hz
		478	when	"11111"	,	 --	C	1046.502261	Hz
		0			when others;


	
	prog_div: process (ck)	-- divides the input frequency by the appropriate divider
	begin
		if ck'event and ck='1' then
			if count = divider - 1 then 
				count<=0;
				div_clk<='1';
			else 
				count <= count + 1;
				div_clk<='0';
			end if;
		end if;
	end process;

	cord: process(div_clk)
		variable xnew: std_logic_vector (7 downto 0);
	begin
		if div_clk'event and div_clk='1' then
			xnew:=x+y(7 downto 4);		-- xnew is a variable; its value is updated imediately
												-- previous value of y used !!!
			x<=xnew;							-- x signal will receive the new value only in the next process instantion
			y<=y-xnew(7 downto 4);		-- updated value of xnew used!!!
		end if;
	end process;		

	triang: process(div_clk)
	begin
		if div_clk'event and div_clk='1' then
			if sense='1' then					-- rising slope
				if xt="11100100" then	 	-- 228 decimal
					sense<='0';					-- change slope to falling
					xt<=xt-"00000100";
				else
					xt<=xt+"00000100";	-- 200 swing in 50 steps of 4
				end if;
			else									-- falling slope
--				if xt="00011100" then 		-- 28 decimal for 202 steps
				if xt="00100000" then 		-- 32 decimal for 200 steps
					sense<='1';					-- change slope to rising
					xt<=xt+"00000100";
				else
					xt<=xt-"00000100";		-- 200 swing in 50 steps of 4
				end if;
			end if;
		end if;
	end process;		

	xr <= "11100100" when sense = '1' else	
			"00011100";		-- rectanle signal from triangle slope
			-- same amplitude as sinus

	Saw_t: process(div_clk)
	begin
		if div_clk'event and div_clk='1' then
			if xs="11100100" then -- 228 decimal
				xs<="00011110";		-- 30 decimal
			else
				xs<=xs+"00000010";	-- 200 swing in 100 steps of 2
			end if;
		end if;
	end process;		


-- some additional combinations

	cos2 <= x * x;
	tri2 <= xt * xt;


-- selecting the sound type
	with mode select
	audioSample <=
		(not y(7)) & y(6 downto 0)	when	"000"	,	 --	sinus (converting 2's complement to Binary Offset code
		xt	when	"001"	,	 --	triangle
		xr	when	"010"	,	 --	rectangle
		xs	when	"011"	,	 --	sawtooth
		cos2(13 downto 6) when "100", -- cos^2
		(not y(7)) & y(6 downto 0)+('0'&cos2(15 downto 9)) when "101", -- sin + (cos^2)/2	
		xt+('0'&cos2(15 downto 9)) when "110", -- triangle + (cos^2)/2
		xt + tri2(15 downto 9) when others; -- triangle + (triangle^2)/2






end Behavioral;

