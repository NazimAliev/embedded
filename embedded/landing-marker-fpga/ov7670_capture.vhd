----------------------------------------------------------------------------------
-- Engineer: Mike Field <hamster@snap.net.nz>
-- 
-- Description: Captures the pixels coming from the OV7670 camera and 
--              Stores them in block RAM
----------------------------------------------------------------------------------
library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.cv_package.all;

entity ov7670_capture is
	Port (
				 btnr	 : in		std_logic;
				 pclk  : in   std_logic;
				 vsync : in   std_logic;
				 href  : in   std_logic;
				 din   : in   std_logic_vector (7 downto 0);
				 addr  : out  std_logic_vector (18 downto 0);
				 dout  : out  std_logic_vector (7 downto 0);
				 we    : out  std_logic);
end ov7670_capture;

architecture Behavioral of ov7670_capture is
	signal address : std_logic_vector(18 downto 0) := (others => '0');
	signal state    : std_logic := '0';
	signal x,y :  integer range 0 to 2048 := 0;

begin
	addr <= address;
	process(pclk)
	begin
		if rising_edge(pclk) then
			if vsync = '1' then 
				address <= (others => '0');
				we <= '0';
				state <= '0';
				y <= 0;
			else
				if href = '1' then
					x <= x + 1;
				else
					x <= 0;
				end if; -- href 

				if state = '1' and href = '1' then
					--state <= not state;
					state <= '0';
					
					if test_capture_c = '1' then
						--if x > w640_c - 500 and x < w640_c + 500 and y > h480_c/2 - 100 and y < h480_c/2 + 100 then
						if x/2 > y then
							dout <= x"ff";
						else
							dout <= x"00";
						end if;
					end if;
					
					if x >= w640_c*2 - 2 then
						y <= y + 1;
					end if;
					address <= std_logic_vector(unsigned(address)+1);
					we <= '0';
				else
				
					if test_capture_c = '0' then
						dout <= din;
					end if;					
					
					-- FIXME
					if btnr = '0' then
						we <= '1';
					else
						we <= '0';
					end if;
					-- FIXME
					
					state <= '1';
				end if; -- href and state
			end if; -- vsync
		end if; -- edge
		
	end process;

end Behavioral;
