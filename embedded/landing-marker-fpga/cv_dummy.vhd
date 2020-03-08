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

entity dummy is
	port ( 
				 -- data for display camera and edge data
				 -- addr_mem1 <= addr_mem 
				 -- dout <= din
				 btnl				: in  std_logic;
				 btnc				: in  std_logic;
				 sync       : in  std_logic;
				 addr_mem		: in std_logic_vector(18 downto 0);
				 din				: in  std_logic_vector(3 downto 0);
				 edge			  : in unsigned(15 downto 0);
				 addr_mem1  : out std_logic_vector(18 downto 0);
				 dout				: out  std_logic_vector(3 downto 0);
				 clkcore		: in std_logic;
				 -- data for grad component
				 clk100     : in  std_logic;
				 x					: in unsigned(15 downto 0);
				 y					: in unsigned(15 downto 0);
				 grad_x			: in signed(15 downto 0);
				 grad_y			: in signed(15 downto 0);
				 grad_ready : in std_logic
			 );
end dummy;

architecture Behavioral of dummy is

	component grad
		port
		(
			clk100		: in std_logic;
			x					: in unsigned(15 downto 0);
			y					: in unsigned(15 downto 0);
			grad_x		: in signed(15 downto 0);
			grad_y		: in signed(15 downto 0);
			grad_ready: in std_logic;
			line_x		: out integer;
			line_y		: out integer;
			-- take line_x, line_y by clk100 when en=1
			en				: out std_logic
		);
	end component;

	constant cols_acc_c	: integer := w640_c/scale_c;
	constant rows_acc_c	: integer := h480_c/scale_c;

	--type acc_t is array (0 to 39, 0 to 29) of unsigned(7 downto 0);
	type acc_t is array (0 to cols_acc_c-1, 0 to rows_acc_c-1) of unsigned(7 downto 0);
	signal acc		: acc_t := (others => (others => x"00"));
	signal acc_cpy: acc_t := (others => (others => x"00"));
	signal acc_max: unsigned(7 downto 0) := (others => '0');
	
	signal en			: std_logic := '0';
	signal line_x : integer;
	signal line_y : integer;
	-- marker coordinates
	signal x_max	: integer := 0;
	signal y_max	: integer := 0;

begin

	addr_mem1 <= addr_mem;

	igrad : grad
	port map(
						clk100		=> clk100,
						x					=> x,
						y					=> y,
						grad_x		=> grad_x,
						grad_y		=> grad_y,
						grad_ready => grad_ready,
						line_x		=> line_x,
						line_y		=> line_y,
						en				=> en	
					);

	process(clk100)
	begin
		if(rising_edge(clk100)) then
			-- FIXME remove mod
			if en = '1' and sync = '0' then
				if btnl = '0' then
					acc(line_x,line_y) <= acc(line_x,line_y) + 1;
				end if;
			
			end if; --en;
			-- FIXME
			if sync = '1' then
				--FIXME
				if btnc = '0' then
					acc <= (others => (others => x"00"));
				end if;
			end if;

		end if;	-- edge clk100
	end process;

	process(sync)
	begin
		-- reload acc when new frame by signal from other process
		if rising_edge(sync) then
			acc_cpy <= acc;
			--acc_max <= (others => '0');
		end if;
				
	end process;

	process(clkcore)

		variable acc2_v		: unsigned(7 downto 0) := (others => '0');

	begin

		if(rising_edge(clkcore)) then

			-- acc cell not empty, show point of gradient line?
			acc2_v := acc_cpy(to_integer(x)/scale_c,to_integer(y)/scale_c);
			
			-- check acc max value on crossed gradient lines
			if acc2_v > acc_max then
				-- fix coordinates for max acc
				x_max <= line_x;
				y_max <= line_y;
				-- refresh max acc value
				--acc_max <= acc2_v;
			end if;
			
			--if x_max = to_integer(x)/scale_c and y_max = to_integer(y)/scale_c then
				--dout <= x"f";
			--end if;
			if test_dummy_c = '1' then
				dout <= std_logic_vector(acc2_v(3 downto 0));
			else
				-- show original image and edge
				if x < 320 then
					-- left half of screen: translate input picture to output
					dout <= din;
				else
					-- right half of screen: show edge
					dout <= std_logic_vector(edge(8 downto 5));
				end if; --x
			end if; -- test_dummy_c

		end if; --edge

	end process;


end Behavioral;
