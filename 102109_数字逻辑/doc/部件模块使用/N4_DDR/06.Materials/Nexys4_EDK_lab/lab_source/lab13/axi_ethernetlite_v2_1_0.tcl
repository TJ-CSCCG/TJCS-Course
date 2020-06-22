###############################################################################
##    DISCLAIMER OF LIABILITY                     
##                                 
##    This file contains proprietary and confidential information of      
##    Xilinx, Inc. ("Xilinx"), that is distributed under a license        
##    from Xilinx, and may be used, copied and/or disclosed only       
##    pursuant to the terms of a valid license agreement with Xilinx.        
##                                 
##    XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION         
##    ("MATERIALS") "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER       
##    EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING WITHOUT           
##    LIMITATION, ANY WARRANTY WITH RESPECT TO NONINFRINGEMENT,        
##    MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE. Xilinx       
##    does not warrant that functions included in the Materials will      
##    meet the requirements of Licensee, or that the operation of the        
##    Materials will be uninterrupted or error-free, or that defects      
##    in the Materials will be corrected. Furthermore, Xilinx does        
##    not warrant or make any representations regarding use, or the       
##    results of the use, of the Materials in terms of correctness,       
##    accuracy, reliability or otherwise.               
##                                 
##    Xilinx products are not designed or intended to be fail-safe,       
##    or for use in any application requiring fail-safe performance,      
##    such as life-support or safety devices or systems, Class III        
##    medical devices, nuclear facilities, applications related to        
##    the deployment of airbags, or any other applications that could        
##    lead to death, personal injury or severe property or          
##    environmental damage (individually and collectively, "critical      
##    applications"). Customer assumes the sole risk and liability        
##    of any use of Xilinx products in critical applications,          
##    subject only to applicable laws and regulations governing        
##    limitations on product liability.                 
##                                 
##    Copyright 2005, 2008, 2009, 2010 Xilinx, Inc.                                
##    All rights reserved.                     
##                                 
##    This disclaimer and copyright notice must be retained as part       
##    of this file at all times.                  
##  
###############################################################################
##
## xps_ethernetlite_v2_1_0.tcl
##
###############################################################################


#***--------------------------------***-----------------------------------***
#
#             CORE_LEVEL_CONSTRAINTS
#
#***--------------------------------***-----------------------------------***

proc generate_corelevel_ucf {mhsinst} {

    set  filePath [xget_ncf_dir $mhsinst]

}

proc generate_corelevel_ucf_1 {mhsinst} {

    set  filePath [xget_ncf_dir $mhsinst]

    file mkdir    $filePath

    # specify file name
    set    instname   [xget_hw_parameter_value $mhsinst "INSTANCE"]
    set    name_lower [string      tolower    $instname]
    set    fileName   $name_lower
    append fileName   "_wrapper.ucf"
    append filePath   $fileName

    set    duplex       [xget_value $mhsinst   "parameter" "C_DUPLEX"]
    set    constraints  [xget_value $mhsinst   "parameter" "C_INCLUDE_PHY_CONSTRAINTS"]

    if { $constraints == 1 } {

        # Open a file for writing
        set    outputFile [open $filePath "w"]


        # Send a line to the output file
      #  puts   $outputFile "NET \"PHY_rx_clk\" TNM_NET = \"RXCLK_GRP_$instname\";"
         puts   $outputFile "NET \"PHY_tx_clk\" TNM_NET = \"TXCLK_GRP_$instname\";"
         puts   $outputFile "TIMESPEC \"TSTXOUT_$instname\" = FROM \"TXCLK_GRP_$instname\" TO \"PADS\" 10 ns;"
      #  puts   $outputFile "TIMESPEC \"TSRXIN_$instname\" = FROM \"PADS\" TO \"RXCLK_GRP_$instname\" 6 ns;"

         ########################################################
         ### OFFSET IN and OFFSET OUT constraints on PHY Clocks
         ########################################################

#          set tx_clk         [ xget_connected_global_ports $mhsinst "PHY_tx_clk"]
#          set rx_clk         [ xget_connected_global_ports $mhsinst "PHY_rx_clk"]
#
#          set global_tx_clk_name [ xget_hw_name [lindex $tx_clk 0] ]
#          set global_rx_clk_name [ xget_hw_name [lindex $rx_clk 0] ]
#
#          #puts   $outputFile "OFFSET = OUT 10.000 AFTER  \"$global_tx_clk_name\";"
#          puts   $outputFile "OFFSET = IN 6.000 BEFORE  \"$global_rx_clk_name\";"

         #########################################################



        set target_family [ xget_hw_parameter_value $mhsinst "C_FAMILY" ]

        # family based constraints
        # if virtex family
        if { [xcheck_virtex_device $target_family] == 1 } {

          puts   $outputFile "NET \"PHY_rx_clk\" USELOWSKEWLINES;"
          puts   $outputFile "NET \"PHY_tx_clk\" USELOWSKEWLINES;"

        }

        # for spartan3 maxskew constraint is 5.0 ns
        set maxskew_value "MAXSKEW= 5.0 ns"
        # for devices other than spartan3 maxskew constraint is 6.0 ns
        if { [string match -nocase {spartan3*} $target_family] == 0 } {
          set maxskew_value "MAXSKEW= 6.0 ns" 
        }

        puts   $outputFile "NET \"PHY_tx_clk\" $maxskew_value;"
        puts   $outputFile "NET \"PHY_rx_clk\" $maxskew_value;"    

        puts   $outputFile "NET \"PHY_rx_clk\" PERIOD = 40 ns HIGH 14 ns;"
        puts   $outputFile "NET \"PHY_tx_clk\" PERIOD = 40 ns HIGH 14 ns;"

        # if C_FAMILY == V4, V5 or SPARTAN3E, use IOBDELAY=NONE and 
        # NODELAY for everything else
        ## set delay "NODELAY"
        set delay "IOBDELAY=NONE"

        if { [string compare -nocase $target_family "virtex"] == 0 } {
          set delay "NODELAY"
        } 

        puts   $outputFile "NET \"PHY_rx_data<3>\" $delay;"
        puts   $outputFile "NET \"PHY_rx_data<2>\" $delay;"
        puts   $outputFile "NET \"PHY_rx_data<1>\" $delay;"
        puts   $outputFile "NET \"PHY_rx_data<0>\" $delay;"
        puts   $outputFile "NET \"PHY_dv\" $delay;"
        puts   $outputFile "NET \"PHY_rx_er\" $delay;"
        puts   $outputFile "NET \"PHY_crs\" $delay;"

        if { $duplex == 0 } {

            puts   $outputFile "NET \"PHY_col\" $delay;"
        }

        ## False Paths ##
         puts   $outputFile "NET \"S_AXI_ACLK\" TNM_NET = \"S_AXI_ACLK_$instname\";"
         puts   $outputFile "NET \"PHY_rx_clk\" TNM_NET = \"RXCLK_GRP_$instname\";"
         puts   $outputFile "TIMESPEC \"TS_AXI_TX_FP_$instname\" = FROM \"S_AXI_ACLK_$instname\" TO \"TXCLK_GRP_$instname\" TIG;"
         puts   $outputFile "TIMESPEC \"TS_TX_AXI_FP_$instname\" = FROM \"TXCLK_GRP_$instname\" TO \"S_AXI_ACLK_$instname\" TIG;"
         puts   $outputFile "TIMESPEC \"TS_AXI_RX_FP_$instname\" = FROM \"S_AXI_ACLK_$instname\" TO \"RXCLK_GRP_$instname\" TIG;"
         puts   $outputFile "TIMESPEC \"TS_RX_AXI_FP_$instname\" = FROM \"RXCLK_GRP_$instname\" TO \"S_AXI_ACLK_$instname\" TIG;"

        ###
        # Close the file
        close  $outputFile

        puts   [xget_ncf_loc_info $mhsinst]

    }

    set  xdc_filePath [xget_ncf_dir $mhsinst]

    file mkdir    $xdc_filePath

    # specify file name
    set    xdc_instname   [xget_hw_parameter_value $mhsinst "INSTANCE"]
    set    xdc_name_lower [string      tolower    $instname]
    set    xdc_fileName   $name_lower
    append xdc_fileName   "_wrapper.xdc"
    append xdc_filePath   $xdc_fileName

    set    duplex       [xget_value $mhsinst   "parameter" "C_DUPLEX"]
    set    constraints  [xget_value $mhsinst   "parameter" "C_INCLUDE_PHY_CONSTRAINTS"]

    if { $constraints == 1 } {

        # Open a file for writing
        set xdc_outputFile [open $xdc_filePath "w"]

#        set tx_clk         [ xget_connected_global_ports $mhsinst "PHY_tx_clk"]
#        set rx_clk         [ xget_connected_global_ports $mhsinst "PHY_rx_clk"]

        set tx_data        [ xget_connected_global_ports $mhsinst "PHY_tx_data"]
        set tx_en          [ xget_connected_global_ports $mhsinst "PHY_tx_en"]
        set rx_data        [ xget_connected_global_ports $mhsinst "PHY_rx_data"]
        set crs            [ xget_connected_global_ports $mhsinst "PHY_crs"]
        set dv             [ xget_connected_global_ports $mhsinst "PHY_dv"]
        set col            [ xget_connected_global_ports $mhsinst "PHY_col"]
        set rx_er          [ xget_connected_global_ports $mhsinst "PHY_rx_er"]

#        set global_tx_clk_name [ xget_hw_name [lindex $tx_clk 0] ]
#        set global_rx_clk_name [ xget_hw_name [lindex $rx_clk 0] ]

        set global_tx_data     [ xget_hw_name [lindex $tx_data 0] ]
        set global_tx_en       [ xget_hw_name [lindex $tx_en 0] ]
        set global_rx_data     [ xget_hw_name [lindex $rx_data 0] ]
        set global_crs         [ xget_hw_name [lindex $crs 0] ]
        set global_dv          [ xget_hw_name [lindex $dv 0] ]
        set global_col         [ xget_hw_name [lindex $col 0] ]
        set global_rx_er       [ xget_hw_name [lindex $rx_er 0] ]


         puts $xdc_outputFile "create_clock \[get_ports {$global_tx_clk_name}\] -name $global_tx_clk_name -period 40 -waveform {0 14}"
         puts $xdc_outputFile "create_clock \[get_ports {$global_rx_clk_name}\] -name $global_rx_clk_name -period 40 -waveform {0 14}"
         puts $xdc_outputFile "set_output_delay 10 \[get_ports {$global_tx_data $global_tx_en}\] -clock {$global_tx_clk_name}"

        set target_family [ xget_hw_parameter_value $mhsinst "C_FAMILY" ]
        # for spartan3 maxskew constraint is 5.0 ns
        set maxskew_value "5"
        # for devices other than spartan3 maxskew constraint is 6.0 ns
        if { [string match -nocase {spartan3*} $target_family] == 0 } {
          set maxskew_value "6" 
        }

         puts $xdc_outputFile "set_input_delay 34 \[get_ports {$global_rx_data $global_crs $global_dv $global_col $global_rx_er}\] -clock {$global_rx_clk_name}"
         puts $xdc_outputFile "set_clock_uncertainty -setup $maxskew_value \[get_clocks {$global_tx_clk_name $global_rx_clk_name}\]"

         ## False Paths
         puts $xdc_outputFile "set_false_path -from \[get_pins $instname/S_AXI_ACLK\] -to \[get_pins $instname/PHY_tx_clk\]"
         puts $xdc_outputFile "set_false_path -from \[get_pins $instname/PHY_tx_clk\] -to \[get_pins $instname/S_AXI_ACLK\]"
         puts $xdc_outputFile "set_false_path -from \[get_pins $instname/S_AXI_ACLK\] -to \[get_pins $instname/PHY_rx_clk\]"
         puts $xdc_outputFile "set_false_path -from \[get_pins $instname/PHY_rx_clk\] -to \[get_pins $instname/S_AXI_ACLK\]"
        # Close the file
        close  $xdc_outputFile

        puts   [xget_ncf_loc_info $mhsinst]

    }
}

#***--------------------------------***-----------------------------------***
#
#                          SYSLEVEL_DRC_PROC (IP)
#
#***--------------------------------***-----------------------------------***

proc check_syslevel_drcs {mhsinst} {
   set  instname           [xget_hw_parameter_value $mhsinst "INSTANCE"]
   set  include_mdio  [xget_hw_parameter_value $mhsinst "C_INCLUDE_MDIO"]
   set  nhandle [xget_hw_parameter_value $mhsinst PHY_MDIO]
   #puts "this is handle :$nhandle ,"

   if { $include_mdio == 1 } {

   if {[xget_hw_port_value $mhsinst PHY_MDIO] != ""} {
     puts "Port present in $instname"
   } elseif {[xget_hw_port_value $mhsinst PHY_MDIO_I] != "" && [xget_hw_port_value $mhsinst PHY_MDIO_O] != "" && [xget_hw_port_value $mhsinst PHY_MDIO_T] != ""} {
     puts "Port present MDIO_MDIO (I,O,T) in $instname"

   } else {
     error "Port PHY_MDIO or PHY_MDIO_I/_O/_T not present in $instname; User must specify a connection to this port in the MHS when MDIO interface is included in the design." "" "mdt_error"
     

   }
   }

}
