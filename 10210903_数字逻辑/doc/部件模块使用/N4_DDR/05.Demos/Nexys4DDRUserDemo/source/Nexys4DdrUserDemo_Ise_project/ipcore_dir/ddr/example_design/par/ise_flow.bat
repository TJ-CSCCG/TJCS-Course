::*****************************************************************************
:: (c) Copyright 2009 - 2012 Xilinx, Inc. All rights reserved.
::
:: This file contains confidential and proprietary information
:: of Xilinx, Inc. and is protected under U.S. and
:: international copyright and other intellectual property
:: laws.
::
:: DISCLAIMER
:: This disclaimer is not a license and does not grant any
:: rights to the materials distributed herewith. Except as
:: otherwise provided in a valid license issued to you by
:: Xilinx, and to the maximum extent permitted by applicable
:: law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
:: WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
:: AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
:: BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
:: INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
:: (2) Xilinx shall not be liable (whether in contract or tort,
:: including negligence, or under any other theory of
:: liability) for any loss or damage of any kind or nature
:: related to, arising under or in connection with these
:: materials, including for any direct, or any indirect,
:: special, incidental, or consequential loss or damage
:: (including loss of data, profits, goodwill, or any type of
:: loss or damage suffered as a result of any action brought
:: by a third party) even if such damage or loss was
:: reasonably foreseeable or Xilinx had been advised of the
:: possibility of the same.
::
:: CRITICAL APPLICATIONS
:: Xilinx products are not designed or intended to be fail-
:: safe, or for use in any application requiring fail-safe
:: performance, such as life-support or safety devices or
:: systems, Class III medical devices, nuclear facilities,
:: applications related to the deployment of airbags, or any
:: other applications that could lead to death, personal
:: injury, or severe property or environmental damage
:: (individually and collectively, "Critical
:: Applications"). Customer assumes the sole risk and
:: liability of any use of Xilinx products in Critical
:: Applications, subject only to applicable laws and
:: regulations governing limitations on product liability.
::
:: THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
:: PART OF THIS FILE AT ALL TIMES.
::
::****************************************************************************
::   ____  ____
::  /   /\/   /
:: /___/  \  /    Vendor                : Xilinx
:: \   \   \/     Version               : 1.9
::  \   \         Application           : MIG
::  /   /         Filename              : ise_flow.bat
:: /___/   /\     Date Last Modified    : $Date: 2011/06/02 08:31:14 $
:: \   \  /  \    Date Created          : Tue Sept 21 2010
::  \___\/\___\
::
:: Device            : 7 Series
:: Design Name       : DDR2 SDRAM
:: Purpose           : Batch file to run Synthesis and PAR through ISE in batch mode
:: Reference         :
:: Revision History  :
::****************************************************************************

call rem_files.bat

echo Synthesis Tool: XST

mkdir "../synth/__projnav" > ise_flow_results.txt
mkdir "../synth/xst" >> ise_flow_results.txt
mkdir "../synth/xst/work" >> ise_flow_results.txt

xst -ifn xst_options.txt -ofn example_top.syr -intstyle ise >> ise_flow_results.txt
if not exist "example_top.ngc" ( 
  echo Failed in SYNTHESIS 
  goto END )

echo Implementation Tool: ISE


echo Running TRANSLATE
ngdbuild -intstyle ise -dd ../synth/_ngo -nt timestamp -uc example_top.ucf -p xc7a100t-csg324-1 example_top.ngc example_top.ngd >> ise_flow_results.txt
if not exist "example_top.ngd" ( 
  echo Failed in TRANSLATE
  goto END )

echo Running MAP
map -intstyle ise -p xc7a100t-csg324-1 -w -ol high -xe n -register_duplication off -ir off -pr off -lc off -power off -o example_top_map.ncd example_top.ngd example_top.pcf >> ise_flow_results.txt
if not exist "example_top_map.ncd" ( 
  echo Failed in MAP
  goto END )

echo Running PAR
par -w -intstyle ise -ol high -xe n example_top_map.ncd example_top.ncd example_top.pcf >> ise_flow_results.txt
if not exist "example_top.ncd" ( 
  echo Failed in PAR
  goto END )


echo Running TRACE 
trce -intstyle ise -v 3 -fastpaths -xml example_top example_top.ncd -o example_top.twr example_top.pcf >> ise_flow_results.txt
echo Running BITGEN
bitgen -intstyle ise example_top.ncd >> ise_flow_results.txt

if not exist "example_top.bit" ( 
  echo Failed in BITGEN
  goto END )

echo IMPLEMENTATION SUCCESSFUL
:END
