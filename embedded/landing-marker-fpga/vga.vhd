----------------------------------------------------------------------------------
-- Engineer: Mike Field <hamster@snap.net.nz>
-- 
-- Description: Generate analog 640x480 VGA, double-doublescanned from 19200 bytes of RAM
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity vga is
	Port ( 
				 clk4       : in  std_logic;
				 vga_red     : out std_logic_vector(3 downto 0);
				 vga_green   : out std_logic_vector(3 downto 0);
				 vga_blue    : out std_logic_vector(3 downto 0);
				 vga_hsync   : out std_logic;
				 vga_vsync   : out std_logic;
				 frame_addr  : out std_logic_vector(18 downto 0);
				 frame_pixel : in  std_logic_vector(3 downto 0)
			 );
end vga;

architecture Behavioral of vga is
	-- Timing constants
	constant hRez       : natural := 640;
	constant hStartSync : natural := hRez+16;
	constant hEndSync   : natural := hRez+16+96;
	constant hMaxCount  : natural := 800;

	constant vRez       : natural := 480;
	constant vStartSync : natural := vRez+10;
	constant vEndSync   : natural := vRez+10+2;
	constant vMaxCount  : natural := vRez+10+2+33;

	constant hsync_active : std_logic := '0';
	constant vsync_active : std_logic := '0';

	signal hCounter : unsigned( 9 downto 0) := (others => '0');
	signal vCounter : unsigned( 9 downto 0) := (others => '0');
	signal address  : unsigned(18 downto 0) := (others => '0');
	signal blank    : std_logic := '1';

begin

	frame_addr <= std_logic_vector(address);

	process(clk4)
	begin
		if rising_edge(clk4) then
			-- Count the lines and rows      
			if hCounter = hMaxCount-1 then
				hCounter <= (others => '0');
				if vCounter = vMaxCount-1 then
					vCounter <= (others => '0');
				else
					vCounter <= vCounter+1;
				end if;
			else
				hCounter <= hCounter+1;
			end if;

			if blank = '0' then
				vga_red   <= frame_pixel;
				vga_green <= frame_pixel;
				vga_blue  <= frame_pixel;
			else
				vga_red   <= (others => '0');
				vga_green <= (others => '0');
				vga_blue  <= (others => '0');
			end if;

			if vCounter  >= vRez then --480
				address <= (others => '0');
				blank <= '1';
			else 
				if hCounter  < hRez then --640
					blank <= '0';
					address <= address+1;
				else
					blank <= '1';
				end if;
			end if;

			-- Are we in the hSync pulse? (one has been added to include frame_buffer_latency)
			if hCounter > hStartSync and hCounter <= hEndSync then
				vga_hSync <= hsync_active;
			else
				vga_hSync <= not hsync_active;
			end if;

			-- Are we in the vSync pulse?
			if vCounter >= vStartSync and vCounter < vEndSync then
				vga_vSync <= vsync_active;
			else
				vga_vSync <= not vsync_active;
			end if;
		end if;
	end process;
end Behavioral;
