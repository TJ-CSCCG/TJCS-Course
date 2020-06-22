##############################################################################
## Filename:          J:\Hebin\system/drivers/ip_vga_v1_00_a/data/ip_vga_v2_1_0.tcl
## Description:       Microprocess Driver Command (tcl)
## Date:              Mon May 27 21:40:58 2013 (by Create and Import Peripheral Wizard)
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
  xdefine_include_file $drv_handle "xparameters.h" "ip_vga" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" 
}
