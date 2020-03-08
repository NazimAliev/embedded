--
-- Copyright (c) by Nazim Aliev
-- All rights reserved.
--
-- nazim.ru@gmail.com
--

library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

Library UNISIM;
use UNISIM.vcomponents.all;

entity ov7670_top is
	Port ( 
				 clk100_zed   : in    std_logic;
				 OV7670_SIOC  : out   std_logic;
				 OV7670_SIOD  : inout std_logic;
				 OV7670_RESET : out   std_logic;
				 OV7670_PWDN  : out   std_logic;
				 OV7670_VSYNC : in    std_logic;
				 OV7670_HREF  : in    std_logic;
				 OV7670_PCLK  : in    std_logic;
				 OV7670_XCLK  : out   std_logic;
				 OV7670_D     : in    std_logic_vector(7 downto 0);

				 led          : out    std_logic_vector(7 downto 0);

				 vga_red      : out   std_logic_vector(3 downto 0);
				 vga_green    : out   std_logic_vector(3 downto 0);
				 vga_blue     : out   std_logic_vector(3 downto 0);
				 vga_hsync    : out   std_logic;
				 vga_vsync    : out   std_logic;

				 btn 		    	: in    std_logic;
				 btnl		    	: in    std_logic;
				 btnc		    	: in    std_logic;
				 btnr		    	: in    std_logic;
				 btnd		    	: in    std_logic
			 );
end ov7670_top;

architecture Behavioral of ov7670_top is
	
	component clk_wiz_0
		port
		(-- Clock in ports
		  clk_in_wiz         : in     std_logic;
		  -- Clock out ports
		  clk_50wiz          : out    std_logic;
		  clk_25wiz          : out    std_logic;
		  clk_12wiz          : out    std_logic;
		  clk_8wiz       	   : out    std_logic
		);
	end component; 
	
	component debounce
		Port(
					clk : in std_logic;
					i 	: in std_logic;          
					o 	: out std_logic
				);
	end component;

	component ov7670_capture
		Port(
					btnr	: in std_logic;
					pclk  : in std_logic;
					vsync : in std_logic;
					href  : in std_logic;
					din   : in std_logic_vector(7 downto 0);
					addr  : out std_logic_vector(18 downto 0);
					dout  : out std_logic_vector(7 downto 0);
					we    : out std_logic
				);
	end component;

	component blk_mem_gen_0
		Port (
					 clka  : in  std_logic;
					 wea   : in  std_logic_vector(0 downto 0);
					 addra : in  std_logic_vector(18 downto 0);
					 dina  : in  std_logic_vector(7 downto 0);
					 clkb  : in  std_logic;
					 addrb : in  std_logic_vector(18 downto 0);
					 doutb : out std_logic_vector(7 downto 0)
				 );
	end component;

	component core
		Port(
					btnd			: in std_logic;
					clk12    	: in std_logic;
					addr_mem0 : out std_logic_vector(18 downto 0);
					addr_mem1 : out std_logic_vector(18 downto 0);
					din 			: in  std_logic_vector(7 downto 0);
					dout 			: out  std_logic_vector(3 downto 0);
					clkcore		: out std_logic;
					sync			: out std_logic;
					x 				: out unsigned(15 downto 0);
					y 				: out unsigned(15 downto 0);
					grad_x 		: out signed(15 downto 0);
					grad_y 		: out signed(15 downto 0);
					grad_ready : out std_logic;
					edge		  : out unsigned(15 downto 0)
	);
	end component;

	component dummy is
		port (	
					 btnl				: in std_logic;
					 btnc				: in std_logic;
					 clkcore		: in std_logic;
					 clk100			: in std_logic;
					 sync 			: in std_logic;
					 addr_mem		: in std_logic_vector(18 downto 0);
					 addr_mem1	: out std_logic_vector(18 downto 0);
					 din				: in std_logic_vector(3 downto 0);
					 dout				: out std_logic_vector(3 downto 0);
					 x 					: in unsigned(15 downto 0);
					 y 					: in unsigned(15 downto 0);
					 grad_x 		: in signed(15 downto 0);
					 grad_y 		: in signed(15 downto 0);
					 edge			  : in unsigned(15 downto 0);
					 grad_ready : in std_logic
				 );
	end component;

	component blk_mem_gen_1
		Port (
					 clka  : in  std_logic;
					 wea   : in  std_logic_vector(0 downto 0);
					 addra : in  std_logic_vector(18 downto 0);
					 dina  : in  std_logic_vector(3 downto 0);
					 clkb  : in  std_logic;
					 addrb : in  std_logic_vector(18 downto 0);
					 doutb : out std_logic_vector(3 downto 0)
				 );
	end component;

	component vga
		Port(
					clk4     		: in std_logic;
					vga_red   	: out std_logic_vector(3 downto 0);
					vga_green 	: out std_logic_vector(3 downto 0);
					vga_blue  	: out std_logic_vector(3 downto 0);
					vga_hsync 	: out std_logic;
					vga_vsync 	: out std_logic;
					frame_addr	: out std_logic_vector(18 downto 0);
					frame_pixel : in  std_logic_vector(3 downto 0)
				);
	end component;

	component ov7670_controller
		Port(
					clk50 : in    std_logic;
					clk8	: in		std_logic;   
					resend: in    std_logic;    
					config_finished : out std_logic;
					siod  : inout std_logic;      
					sioc  : out   std_logic;
					reset : out   std_logic;
					pwdn  : out   std_logic;
					xclk  : out   std_logic;
					clk4	: out		std_logic
				);
	end component;

	-- clocks from wizard
	signal clk50        	: std_logic;
	signal clk25        	: std_logic;
	signal clk12        	: std_logic;
	signal clk8         	: std_logic;
	-- generated in controller
	signal clk4         	: std_logic;
	-- debounce to controller
	signal resend       	: std_logic;
	signal obtnl	       	: std_logic;
	signal obtnc	       	: std_logic;
	signal obtnr	       	: std_logic;
	signal obtnd	       	: std_logic;
	-- capture to mem_blk_0
	signal capture_addr 	: std_logic_vector(18 downto 0);
	signal capture_data 	: std_logic_vector(7 downto 0);
	signal capture_we   	: std_logic_vector(0 downto 0);
	-- mem_blk_0 -> core
	signal data_mem0_core	: std_logic_vector(7 downto 0);
	signal addr_core_mem0	: std_logic_vector(18 downto 0);
	--core -> dummy
	signal addr_core_dummy: std_logic_vector(18 downto 0);
	signal data_core_dummy: std_logic_vector(3 downto 0);
	signal x_core_dummy 	: unsigned(15 downto 0);
	signal y_core_dummy 	: unsigned(15 downto 0);
	signal grad_x_core_dummy	  : signed(15 downto 0);
	signal grad_y_core_dummy	  : signed(15 downto 0);
	signal clk_core_dummy		    : std_logic;
	signal sync						: std_logic;
	signal edge_core_dummy: unsigned(15 downto 0);
	signal grad_ready 		: std_logic;
	-- dummy -> mem_blk_1
	signal wea_mem1	: std_logic_vector(0 downto 0);
	signal addr_dummy_mem1: std_logic_vector(18 downto 0);
	signal data_dummy_mem1: std_logic_vector(3 downto 0);
	signal clk_dummy_mem1	: std_logic;
	-- mem_blk_1 to vga
	signal frame_addr			: std_logic_vector(18 downto 0);
	signal frame_pixel		: std_logic_vector(3 downto 0);
	-- controller to LED
	signal config_finished: std_logic;

-- ========================================================================
begin

	led <= resend & "0000" & obtnl & obtnc & config_finished;
	wea_mem1(0) <= '1';
	clk_dummy_mem1 <= not clk_core_dummy;
			
clkwiz : clk_wiz_0
		   port map ( 
		   -- Clock in ports
		   clk_in_wiz => clk100_zed,
		  -- Clock out ports  
		   clk_50wiz	=> clk50,
		   clk_25wiz	=> clk25,
		   clk_12wiz	=> clk12,
		   clk_8wiz		=> clk8
		 );

	dbn: debounce
	port map(
						clk => clk50,
						i   => btn,
						o   => resend
					);

	dbnl: debounce
	port map(
						clk => clk50,
						i   => btnl,
						o   => obtnl 
					);
					
					
	dbnc: debounce
	port map(
						clk => clk50,
						i   => btnc,
						o   => obtnc 
					);
					
	dbnr: debounce
	port map(
					clk => clk50,
					i   => btnr,
					o   => obtnr 
				);
									
	dbnd: debounce
	port map(
						clk => clk50,
						i   => btnd,
						o   => obtnd 
					);
					
	capture: ov7670_capture
	port map(
						btnr	=> obtnr,
						pclk  => OV7670_PCLK,
						vsync => OV7670_VSYNC,
						href  => OV7670_HREF,
						din   => OV7670_D,
						addr  => capture_addr,
						dout  => capture_data,
						we    => capture_we(0)
					);

	fb1 : blk_mem_gen_0
	port map (
						 clka  => OV7670_PCLK,
						 wea   => capture_we,
						 addra => capture_addr,
						 dina  => capture_data,

						 clkb  => clk25, -- =core clk12x2 - latency
						 addrb => addr_core_mem0,
						 doutb => data_mem0_core 
					 );

	icore: core
	port map(
						btnd			=> obtnd,
						clk12			=> clk12,
						addr_mem0	=> addr_core_mem0,
						din				=> data_mem0_core,
						addr_mem1	=> addr_core_dummy,
						dout			=> data_core_dummy,
						clkcore		=> clk_core_dummy,
						sync			=> sync,
	 					x					=> x_core_dummy, 
	 					y					=> y_core_dummy, 
						grad_x		=> grad_x_core_dummy,
						grad_y		=> grad_y_core_dummy,
						edge 			=> edge_core_dummy,
						grad_ready	=> grad_ready
					);

	idummy: dummy 
	port map(
						btnl			=> 	obtnl,
						btnc			=>	obtnc,
						sync 			=> 	sync,			
						addr_mem	=> addr_core_dummy,
						addr_mem1	=> addr_dummy_mem1, 
						din				=> data_core_dummy,
						edge 			=> edge_core_dummy,
						dout			=> data_dummy_mem1,
						clkcore		=> clk_core_dummy,
						clk100		=>	clk100_zed,
						x					=> x_core_dummy,
						y					=> y_core_dummy,
						grad_x		=> grad_x_core_dummy,
						grad_y		=> grad_y_core_dummy,
						grad_ready=> grad_ready
					);
					
	fb2 : blk_mem_gen_1
	port map (
						 clka  => clk_dummy_mem1,
						 wea   => wea_mem1,
						 addra => addr_dummy_mem1,
						 dina  => data_dummy_mem1,

							-- FIXME
						 clkb  => clk50, -- =vga clk4x2 - latency
						 addrb => frame_addr,
						 doutb => frame_pixel
					 );      

	display: vga
	port map(
						-- FIXME
						clk4     		=> clk25,
						vga_red     => vga_red,
						vga_green   => vga_green,
						vga_blue    => vga_blue,
						vga_hsync   => vga_hsync,
						vga_vsync   => vga_vsync,
						frame_addr 	=> frame_addr,
						frame_pixel => frame_pixel
					);

	controller: ov7670_controller
	port map(
						clk50 => clk50,
						clk8	=> clk8,
						sioc  => ov7670_sioc,
						resend => resend,
						config_finished => config_finished,
						siod  => ov7670_siod,
						pwdn  => OV7670_PWDN,
						reset => OV7670_RESET,
						xclk  => OV7670_XCLK,
						clk4	=> clk4
					);

end Behavioral;
