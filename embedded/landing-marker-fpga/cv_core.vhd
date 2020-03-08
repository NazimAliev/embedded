--
-- Copyright (c) by Nazim Aliev
-- All rights reserved.
--
-- nazim.ru@gmail.com
--

library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
use work.cv_package.all;

entity core is
	Port (
				 btnd				:	in	std_logic;
				 clk12      : in  std_logic;
				 addr_mem0  : out std_logic_vector(18 downto 0);
				 addr_mem1  : out std_logic_vector(18 downto 0);
				 din 				: in  std_logic_vector(7 downto 0);
				 dout 			: out  std_logic_vector(3 downto 0);
				 clkcore		: out std_logic;
				 sync				: out std_logic;
				 x 					: out unsigned(15 downto 0);
				 y 					: out unsigned(15 downto 0);
				 grad_x 		: out signed(15 downto 0);
				 grad_y 		: out signed(15 downto 0);
				 edge	  		: out unsigned(15 downto 0);
				 grad_ready : out std_logic
			 );
end core;

architecture Behavioral of core is
	signal address_mem0  : unsigned(18 downto 0) := (others => '0');
	signal address_mem1  : unsigned(18 downto 0) := (others => '0');
	-- 9 byte 3x3 sobel matrix
	signal fifo  				 : std_logic_vector(71 downto 0) := (others => '0'); 
	signal ptr					 : integer range 0 to 2 := 0;
	signal row					 : unsigned(15 downto 0) := (others => '0');
	signal col  				 : unsigned(15 downto 0) := (others => '0');
	signal grad     		 : unsigned(17 downto 0);
	signal test_cnt			 : unsigned(23 downto 0) := (others => '0');
	
begin

	addr_mem1 <= std_logic_vector(address_mem1);
	x <= col;
	y <= row;
		
	process(clk12)	

		variable gx				: signed(17 downto 0);
		variable gy				: signed(17 downto 0);
		variable dx, dy   : unsigned(17 downto 0);

	begin
		if rising_edge(clk12) then
			if address_mem0 >= to_unsigned(frame_c - w640_c * 2, address_mem0'length) then -- 640x480 - last 3 rows
				address_mem0 <= (others => '0');
				address_mem1 <= (others => '0');
				addr_mem0 <= (others => '0');
				row <= (others => '0');
				col <= (others => '0');
				sync <= '1';
				test_cnt <= x"000000";
			else
				sync <= '0';
				fifo <= fifo(63 downto 0) & din;
				case ptr is
					when 0 =>
						addr_mem0 <= std_logic_vector(address_mem0 + to_unsigned(w640_c, address_mem0'length)); -- 640
						clkcore <='0';
						ptr <= 1;
						if col = w640_c - 1 then
							col <= (others => '0');
							row <= row + 1;
						else
							col <= col + 1;
						end if;
						if grad > 0 then
							-- FIXME test catch only one grad for showing
							--if test_cnt = 10 then
								--grad_ready <= '1';
							--else
								--test_cnt <= test_cnt + 1;
							--end if;						

							if btnd = '0' then
								grad_ready <= '1';
							else
								grad_ready <= '0';
							end if;
						end if;
					when 1 =>
						addr_mem0 <= std_logic_vector(address_mem0 + to_unsigned(w640_c * 2, address_mem0'length)); -- 640*2
						address_mem1 <= address_mem1 + 1;
						grad_ready <= '0';
						ptr <= 2;

					when others =>
						addr_mem0 <= std_logic_vector(address_mem0); -- 0

						-- show right half of screen
						gx := signed('0' & fifo(23 downto 16)) + 2*signed('0' & fifo(15 downto 8)) + signed('0' & fifo(7 downto 0)) -
						signed('0' & fifo(71 downto 64)) - 2*signed('0' & fifo(63 downto 56)) - signed('0' & fifo(55 downto 48));
						gy :=	signed('0' & fifo(55 downto 48)) - signed('0' & fifo(71 downto 64)) - 2*signed('0' & fifo(47 downto 40)) +
						2*signed('0' & fifo(31 downto 24)) - signed('0' & fifo(23 downto 16)) + signed('0' & fifo(7 downto 0));

						-- test data for simulation only
						if test_core_c = '1' then
							gx := (others => '0');
							gy := (others => '0');
							if test_gr1_c = '1' and row = test_row1_c and col = test_col1_c then
								gx := signed(test_gx1_c);
								gy := signed(test_gy1_c);
							end if;	
							if test_gr2_c = '1' and row = test_row2_c and col = test_col2_c then
								gx := - signed(test_gx2_c);
								gy := signed(test_gy2_c);
							end if;	
						end if;

						grad_x <= gx(17 downto 2);
						grad_y <= gy(17 downto 2);

						if gx > 0 then
							dx := unsigned(gx);
						else
							dx := unsigned(-gx);
						end if;
						if gy > 0 then
							dy := unsigned(gy);
						else
							dy := unsigned(-gy);
						end if;
						grad <= dx + dy;
						-- TODO every 2nd row and col avoid Sobel box overlapped

						address_mem0 <= address_mem0 + 1;
						clkcore <='1';
						ptr <= 0;

						-- original frame
						dout <= fifo(71 downto 68);
						-- contoured frame
						edge <= grad(17 downto 2);

				end case;

			end if; -- mem0 
		end if; -- edge
	end process;

end Behavioral;
