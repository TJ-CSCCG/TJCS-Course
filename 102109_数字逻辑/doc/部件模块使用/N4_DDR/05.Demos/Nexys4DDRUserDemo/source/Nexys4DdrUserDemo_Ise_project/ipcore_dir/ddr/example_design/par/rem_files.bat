::****************************************************************************
:: (c) Copyright 2009 - 2011 Xilinx, Inc. All rights reserved.
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
::  /   /         Filename              : rem_files.bat
:: /___/   /\     Date Last Modified    : $Date: 2011/06/02 08:31:14 $
:: \   \  /  \    Date Created          : Fri Oct 14 2011
::  \___\/\___\
::
:: Device            : 7 Series
:: Design Name       : DDR2 SDRAM
:: Purpose           : Batch file to remove files generated from ISE
:: Reference         :
:: Revision History  :
::****************************************************************************

@echo off
IF EXIST "../synth/__projnav" rmdir /S /Q "../synth/__projnav"
IF EXIST "../synth/xst" rmdir /S /Q "../synth/xst"
IF EXIST "../synth/_ngo" rmdir /S /Q "../synth/_ngo"

IF EXIST tmp rmdir /S /Q tmp
IF EXIST _xmsgs rmdir /S /Q _xmsgs

IF EXIST xst rmdir /S /Q xst
IF EXIST xlnx_auto_0_xdb rmdir /S /Q xlnx_auto_0_xdb

IF EXIST coregen.cgp del /F /Q coregen.cgp
IF EXIST coregen.cgc del /F /Q coregen.cgc
IF EXIST coregen.log del /F /Q coregen.log
IF EXIST stdout.log del /F /Q stdout.log

IF EXIST ise_flow_results.txt del /F /Q ise_flow_results.txt
IF EXIST example_top_vhdl.prj del /F /Q example_top_vhdl.prj
IF EXIST example_top.syr del /F /Q example_top.syr
IF EXIST example_top.ngc del /F /Q example_top.ngc
IF EXIST example_top.ngr del /F /Q example_top.ngr
IF EXIST example_top_xst.xrpt del /F /Q example_top_xst.xrpt
IF EXIST example_top.bld del /F /Q example_top.bld
IF EXIST example_top.ngd del /F /Q example_top.ngd
IF EXIST example_top_ngdbuild.xrpt del /F /Q  example_top_ngdbuild.xrpt
IF EXIST example_top_map.map del /F /Q  example_top_map.map
IF EXIST example_top_map.mrp del /F /Q  example_top_map.mrp
IF EXIST example_top_map.ngm del /F /Q  example_top_map.ngm
IF EXIST example_top.pcf del /F /Q  example_top.pcf
IF EXIST example_top_map.ncd del /F /Q  example_top_map.ncd
IF EXIST example_top_map.xrpt del /F /Q  example_top_map.xrpt
IF EXIST example_top_summary.xml del /F /Q  example_top_summary.xml
IF EXIST example_top_usage.xml del /F /Q  example_top_usage.xml
IF EXIST example_top.ncd del /F /Q  example_top.ncd
IF EXIST example_top.par del /F /Q  example_top.par
IF EXIST example_top.xpi del /F /Q  example_top.xpi
IF EXIST smartpreview.twr del /F /Q  smartpreview.twr
IF EXIST example_top.ptwx del /F /Q  example_top.ptwx
IF EXIST example_top.pad del /F /Q  example_top.pad
IF EXIST example_top.unroutes del /F /Q  example_top.unroutes
IF EXIST example_top_pad.csv del /F /Q  example_top_pad.csv
IF EXIST example_top_pad.txt del /F /Q  example_top_pad.txt
IF EXIST example_top_par.xrpt del /F /Q  example_top_par.xrpt
IF EXIST example_top.twx del /F /Q  example_top.twx
IF EXIST example_top.bgn del /F /Q  example_top.bgn
IF EXIST example_top.twr del /F /Q  example_top.twr
IF EXIST example_top.drc del /F /Q  example_top.drc
IF EXIST example_top_bitgen.xwbt del /F /Q  example_top_bitgen.xwbt
IF EXIST example_top.bit del /F /Q  example_top.bit
IF EXIST example_top.ngo del /F /Q  example_top.ngo
IF EXIST proj_1.prd del /F /Q  proj_1.prd
IF EXIST proj_1.prj del /F /Q  proj_1.prj
IF EXIST netlist.lst del /F /Q  netlist.lst

:: Files and folders generated Coregen ChipScope Modules
IF EXIST ddr_icon.asy del /F /Q  ddr_icon.asy
IF EXIST ddr_icon.ngc del /F /Q  ddr_icon.ngc
IF EXIST ddr_icon.xco del /F /Q  ddr_icon.xco
IF EXIST ddr_icon_xmdf.tcl del /F /Q  ddr_icon_xmdf.tcl
IF EXIST ddr_icon.gise del /F /Q  ddr_icon.gise
IF EXIST ddr_icon.ise del /F /Q  ddr_icon.ise
IF EXIST ddr_icon.xise del /F /Q  ddr_icon.xise
IF EXIST ddr_icon_flist.txt del /F /Q  ddr_icon_flist.txt
IF EXIST ddr_icon_readme.txt del /F /Q  ddr_icon_readme.txt
IF EXIST ddr_icon.cdc del /F /Q  ddr_icon.cdc
IF EXIST ddr_icon_xdb rmdir /S /Q ddr_icon_xdb

IF EXIST ddr_ila_basic.asy del /F /Q  ddr_ila_basic.asy
IF EXIST ddr_ila_basic.ngc del /F /Q  ddr_ila_basic.ngc
IF EXIST ddr_ila_basic.constraints del /S /Q ddr_ila_basic.constraints
IF EXIST ddr_ila_basic.ncf del /F /Q  ddr_ila_basic.ncf
IF EXIST ddr_ila_basic.ucf del /F /Q  ddr_ila_basic.ucf
IF EXIST ddr_ila_basic.xdc del /F /Q  ddr_ila_basic.xdc
IF EXIST ddr_ila_basic.xco del /F /Q  ddr_ila_basic.xco
IF EXIST ddr_ila_basic_xmdf.tcl del /F /Q  ddr_ila_basic_xmdf.tcl
IF EXIST ddr_ila_basic.gise del /F /Q  ddr_ila_basic.gise
IF EXIST ddr_ila_basic.ise del /F /Q  ddr_ila_basic.ise
IF EXIST ddr_ila_basic.xise del /F /Q  ddr_ila_basic.xise
IF EXIST ddr_ila_basic_flist.txt del /F /Q  ddr_ila_basic_flist.txt
IF EXIST ddr_ila_basic_readme.txt del /F /Q  ddr_ila_basic_readme.txt
IF EXIST ddr_ila_basic.cdc del /F /Q  ddr_ila_basic.cdc
IF EXIST ddr_ila_basic_xdb rmdir /S /Q ddr_ila_basic_xdb

IF EXIST ddr_ila_wrpath.asy del /F /Q  ddr_ila_wrpath.asy
IF EXIST ddr_ila_wrpath.ngc del /F /Q  ddr_ila_wrpath.ngc
IF EXIST ddr_ila_wrpath.constraints del /S /Q ddr_ila_wrpath.constraints
IF EXIST ddr_ila_wrpath.ncf del /F /Q  ddr_ila_wrpath.ncf
IF EXIST ddr_ila_wrpath.ucf del /F /Q  ddr_ila_wrpath.ucf
IF EXIST ddr_ila_wrpath.xdc del /F /Q  ddr_ila_wrpath.xdc
IF EXIST ddr_ila_wrpath.xco del /F /Q  ddr_ila_wrpath.xco
IF EXIST ddr_ila_wrpath_xmdf.tcl del /F /Q  ddr_ila_wrpath_xmdf.tcl
IF EXIST ddr_ila_wrpath.gise del /F /Q  ddr_ila_wrpath.gise
IF EXIST ddr_ila_wrpath.ise del /F /Q  ddr_ila_wrpath.ise
IF EXIST ddr_ila_wrpath.xise del /F /Q  ddr_ila_wrpath.xise
IF EXIST ddr_ila_wrpath_flist.txt del /F /Q  ddr_ila_wrpath_flist.txt
IF EXIST ddr_ila_wrpath_readme.txt del /F /Q  ddr_ila_wrpath_readme.txt
IF EXIST ddr_ila_wrpath.cdc del /F /Q  ddr_ila_wrpath.cdc
IF EXIST ddr_ila_wrpath_xdb rmdir /S /Q ddr_ila_wrpath_xdb

IF EXIST ddr_ila_rdpath.asy del /F /Q  ddr_ila_rdpath.asy
IF EXIST ddr_ila_rdpath.ngc del /F /Q  ddr_ila_rdpath.ngc
IF EXIST ddr_ila_rdpath.constraints del /S /Q ddr_ila_rdpath.constraints
IF EXIST ddr_ila_rdpath.ncf del /F /Q  ddr_ila_rdpath.ncf
IF EXIST ddr_ila_rdpath.ucf del /F /Q  ddr_ila_rdpath.ucf
IF EXIST ddr_ila_rdpath.xdc del /F /Q  ddr_ila_rdpath.xdc
IF EXIST ddr_ila_rdpath.xco del /F /Q  ddr_ila_rdpath.xco
IF EXIST ddr_ila_rdpath_xmdf.tcl del /F /Q  ddr_ila_rdpath_xmdf.tcl
IF EXIST ddr_ila_rdpath.gise del /F /Q  ddr_ila_rdpath.gise
IF EXIST ddr_ila_rdpath.ise del /F /Q  ddr_ila_rdpath.ise
IF EXIST ddr_ila_rdpath.xise del /F /Q  ddr_ila_rdpath.xise
IF EXIST ddr_ila_rdpath_flist.txt del /F /Q  ddr_ila_rdpath_flist.txt
IF EXIST ddr_ila_rdpath_readme.txt del /F /Q  ddr_ila_rdpath_readme.txt
IF EXIST ddr_ila_rdpath.cdc del /F /Q  ddr_ila_rdpath.cdc
IF EXIST ddr_ila_rdpath_xdb rmdir /S /Q ddr_ila_rdpath_xdb

IF EXIST ddr_vio_sync_async_out72.asy del /F /Q  ddr_vio_sync_async_out72.asy
IF EXIST ddr_vio_sync_async_out72.ngc del /F /Q  ddr_vio_sync_async_out72.ngc
IF EXIST ddr_vio_sync_async_out72.constraints del /S /Q ddr_vio_sync_async_out72.constraints
IF EXIST ddr_vio_sync_async_out72.ncf del /F /Q  ddr_vio_sync_async_out72.ncf
IF EXIST ddr_vio_sync_async_out72.ucf del /F /Q  ddr_vio_sync_async_out72.ucf
IF EXIST ddr_vio_sync_async_out72.xdc del /F /Q  ddr_vio_sync_async_out72.xdc
IF EXIST ddr_vio_sync_async_out72.xco del /F /Q  ddr_vio_sync_async_out72.xco
IF EXIST ddr_vio_sync_async_out72_xmdf.tcl del /F /Q  ddr_vio_sync_async_out72_xmdf.tcl
IF EXIST ddr_vio_sync_async_out72.gise del /F /Q  ddr_vio_sync_async_out72.gise
IF EXIST ddr_vio_sync_async_out72.ise del /F /Q  ddr_vio_sync_async_out72.ise
IF EXIST ddr_vio_sync_async_out72.xise del /F /Q  ddr_vio_sync_async_out72.xise
IF EXIST ddr_vio_sync_async_out72_flist.txt del /F /Q  ddr_vio_sync_async_out72_flist.txt
IF EXIST ddr_vio_sync_async_out72_readme.txt del /F /Q  ddr_vio_sync_async_out72_readme.txt
IF EXIST ddr_vio_sync_async_out72.cdc del /F /Q  ddr_vio_sync_async_out72.cdc
IF EXIST ddr_vio_sync_async_out72_xdb rmdir /S /Q ddr_vio_sync_async_out72_xdb

IF EXIST ddr_vio_async_in_sync_out.asy del /F /Q  ddr_vio_async_in_sync_out.asy
IF EXIST ddr_vio_async_in_sync_out.ngc del /F /Q  ddr_vio_async_in_sync_out.ngc
IF EXIST ddr_vio_async_in_sync_out.constraints del /S /Q ddr_vio_async_in_sync_out.constraints
IF EXIST ddr_vio_async_in_sync_out.ncf del /F /Q  ddr_vio_async_in_sync_out.ncf
IF EXIST ddr_vio_async_in_sync_out.ucf del /F /Q  ddr_vio_async_in_sync_out.ucf
IF EXIST ddr_vio_async_in_sync_out.xdc del /F /Q  ddr_vio_async_in_sync_out.xdc
IF EXIST ddr_vio_async_in_sync_out.xco del /F /Q  ddr_vio_async_in_sync_out.xco
IF EXIST ddr_vio_async_in_sync_out_xmdf.tcl del /F /Q  ddr_vio_async_in_sync_out_xmdf.tcl
IF EXIST ddr_vio_async_in_sync_out.gise del /F /Q  ddr_vio_async_in_sync_out.gise
IF EXIST ddr_vio_async_in_sync_out.ise del /F /Q  ddr_vio_async_in_sync_out.ise
IF EXIST ddr_vio_async_in_sync_out.xise del /F /Q  ddr_vio_async_in_sync_out.xise
IF EXIST ddr_vio_async_in_sync_out_flist.txt del /F /Q  ddr_vio_async_in_sync_out_flist.txt
IF EXIST ddr_vio_async_in_sync_out_readme.txt del /F /Q  ddr_vio_async_in_sync_out_readme.txt
IF EXIST ddr_vio_async_in_sync_out.cdc del /F /Q  ddr_vio_async_in_sync_out.cdc
IF EXIST ddr_vio_async_in_sync_out_xdb rmdir /S /Q ddr_vio_async_in_sync_out_xdb

:: Files and folders generated by create ise
IF EXIST test_xdb rmdir /S /Q test_xdb
IF EXIST _xmsgs rmdir /S /Q _xmsgs
IF EXIST test.gise del /F /Q test.gise
IF EXIST test.xise del /F /Q test.xise
IF EXIST test.xise del /F /Q test.xise

:: Files and folders generated by ISE through GUI mode
IF EXIST _ngo rmdir /S /Q _ngo
IF EXIST xst rmdir /S /Q xst
IF EXIST example_top.cmd_log del /F /Q example_top.cmd_log
IF EXIST example_top.lso del /F /Q example_top.lso
IF EXIST example_top.prj del /F /Q example_top.prj
IF EXIST example_top.stx del /F /Q example_top.stx
IF EXIST example_top.ut del /F /Q example_top.ut
IF EXIST example_top.xst del /F /Q example_top.xst
IF EXIST example_top_guide.ncd del /F /Q example_top_guide.ncd
IF EXIST example_top_prev_built.ngd del /F /Q example_top_prev_built.ngd
IF EXIST example_top_summary.html del /F /Q example_top_summary.html
IF EXIST par_usage_statistics.html del /F /Q par_usage_statistics.html
IF EXIST usage_statistics_webtalk.html del /F /Q usage_statistics_webtalk.html
IF EXIST webtalk.log del /F /Q webtalk.log
IF EXIST device_usage_statistics.html del /F /Q device_usage_statistics.html
IF EXIST test.ntrc_log del /F /Q test.ntrc_log

@echo on
