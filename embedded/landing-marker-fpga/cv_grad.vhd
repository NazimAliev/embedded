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

entity grad is
	Port ( 
				 clk100	: in  std_logic;
				 x			: in unsigned(15 downto 0);
				 y			: in unsigned(15 downto 0);
				 grad_x	: in signed(15 downto 0);
				 grad_y	: in signed(15 downto 0);
				 grad_ready : in std_logic;
				 -- line starts from x,y
				 line_x	: out integer;
				 line_y	: out integer;
				 en			:	out std_logic
			 );
end grad;

architecture Behavioral of grad is

	-- 20 clock freq 100 MHz  after grad_ready
	constant cnt_max_c	: integer := 20;

	-- line coordinates => line_x, line_y
	signal xb			: integer := 0;
	signal yb			: integer := 0;

	signal en1		: std_logic := '0';
	signal cnt		: integer := 0;
	signal err		: signed(15 downto 0) := (others => '0');

begin

	en <= en1;
	line_x <= xb;
	line_y <= yb;

	process(clk100)

		variable err_fix_v	: unsigned(15 downto 0);
		variable err2_v			: signed(15 downto 0); 
		variable dx_v				: unsigned(15 downto 0); 
		variable dy_v				: unsigned(15 downto 0); 
		variable tmp_v			: unsigned(15 downto 0); 
		variable sx_v				: integer range -1 to 1 := 0;
		variable sy_v				: integer range -1 to 1 := 0;
		variable inv_v			:	std_logic := '0';

	begin
		if rising_edge(clk100) then
			if grad_ready = '1' and en1 = '0' then
				-- 1st clock, run once, then en1 = 1 and this block will be skipped
				-- prepare Bresenham algorinv_vhm data
				if grad_x > 0 then
					dx_v := unsigned(grad_x);
					sx_v := 1;	
				else
					dx_v := unsigned(-grad_x);
					sx_v := -1;
				end if;
				if grad_y > 0 then
					dy_v := unsigned(grad_y);
					sy_v := 1;	
				else
					dy_v := unsigned(-grad_y);
					sy_v := -1;
				end if;
				if dy_v > dx_v then
					tmp_v := dx_v;
					dx_v := dy_v;
					dy_v := tmp_v;
					inv_v := '1';
				else
					inv_v := '0';
				end if;	
				err_fix_v := dy_v + dy_v;
				err <= signed(err_fix_v) - signed(dx_v);
				err2_v := signed(err_fix_v) - signed(dx_v) - signed(dx_v);

				cnt <= 0;
				xb <= to_integer(x/scale_c);
				yb <= to_integer(y/scale_c);
				en1 <= '1';
			end if; --grad

			-- ====================================================
			if en1 = '1' then
				-- main Bresenham algorinv_vhm loop
				-- current line point (xb,yb) is here
				if err < 0 then
					if inv_v = '1' then
						yb <= yb + sy_v;
					else
						xb <= xb + sx_v;
					end if; -- inv_v
				err <= err + signed(err_fix_v);
				else
					yb <= yb + sy_v;
					xb <= xb + sx_v;
					err <= err + err2_v;
				end if; -- err

				cnt <= cnt + 1;
				-- stop when line lenght reach 24 points
				-- clk100 pulses = 24 between clk4 pulses
				-- alg for i in range(dx_v+1):
				if cnt >= cnt_max_c or xb <= 0 or yb <= 0 or xb >= w640_c/scale_c - 1 or yb >= h480_c/scale_c - 1 then
					-- line is ready, next cycle will start when grad_ready = 1
					en1 <= '0';
				end if;
			end if; -- en 
			-- ====================================================
		end if;	-- edge
	end process;

end Behavioral;
