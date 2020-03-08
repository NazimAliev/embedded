--
-- Copyright (c) by Nazim Aliev
-- All rights reserved.
--
-- nazim.ru@gmail.com
--

library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

package cv_package is

	--constant scale_c	 				 : natural := 1;
	--constant w640_c						 : natural := 24;
	--constant h480_c						 : natural := 12;
	--constant test_col1_c				 : natural := 2;
	--constant test_row1_c				 : natural := 2;
	--constant test_col2_c				 : natural := 12;
	--constant test_row2_c				 : natural := 2;

	constant scale_c	 				 : natural := 16;
	constant w640_c						 : natural := 640;
	constant h480_c						 : natural := 480;
	

	constant frame_c					 : natural := w640_c * h480_c;

	-----------------------------------------------------------------------
	-- fixed diagonal b/w image instead of real image
	-- ov7670_capture.vhd
	constant test_capture_c		 : std_logic := '0';

	-----------------------------------------------------------------------
	-- use two fixed gradients instead of real image or instead of fixed image
	-- cv_core.vhd
	constant test_core_c	 		 : std_logic := '0';
	
	-- col/row and real/image of fixed gradients

	constant test_gr1_c				 : std_logic := '0';
	constant test_col1_c			 : natural := 128;
	constant test_row1_c			 : natural := 64;
	constant test_gx1_c				 : unsigned(17 downto 0) := ("00" & x"0fff");
	constant test_gy1_c				 : unsigned(17 downto 0) := ("00" & x"0b88");

	constant test_gr2_c				 : std_logic := '0';
	constant test_col2_c			 : natural := 550;
	constant test_row2_c			 : natural := 32;
	constant test_gx2_c				 : unsigned(17 downto 0) := ("00" & x"0044");
	constant test_gy2_c				 : unsigned(17 downto 0) := ("00" & x"0055");

	-----------------------------------------------------------------------
	-- show acc instead of real/edge image 
	-- cv_dummy.vhd
	constant test_dummy_c	 		 : std_logic := '0';

	-----------------------------------------------------------------------
end package cv_package;

package body cv_package is

end package body cv_package;
