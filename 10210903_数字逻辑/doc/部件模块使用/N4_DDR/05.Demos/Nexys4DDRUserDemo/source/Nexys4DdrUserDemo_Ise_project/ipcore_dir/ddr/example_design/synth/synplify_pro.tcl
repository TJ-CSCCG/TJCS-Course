project -new
#project files
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_afifo.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_cmd_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_cmd_prbs_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_data_prbs_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_init_mem_pattern_ctr.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_memc_flow_vcontrol.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_memc_traffic_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_rd_data_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_read_data_path.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_read_posted_fifo.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_s7ven_data_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_tg_prbs_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_tg_status.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_traffic_gen_top.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_vio_init_pattern_bram.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_wr_data_gen.v"
add_file -verilog "../rtl/traffic_gen/mig_7series_v1_9_write_data_path.v"
add_file -vhdl "../rtl/example_top.vhd"
add_file -verilog "../../user_design/rtl/clocking/mig_7series_v1_9_clk_ibuf.v"
add_file -verilog "../../user_design/rtl/clocking/mig_7series_v1_9_infrastructure.v"
add_file -verilog "../../user_design/rtl/clocking/mig_7series_v1_9_iodelay_ctrl.v"
add_file -verilog "../../user_design/rtl/clocking/mig_7series_v1_9_tempmon.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_arb_mux.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_arb_row_col.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_arb_select.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_bank_cntrl.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_bank_common.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_bank_compare.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_bank_mach.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_bank_queue.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_bank_state.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_col_mach.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_mc.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_rank_cntrl.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_rank_common.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_rank_mach.v"
add_file -verilog "../../user_design/rtl/controller/mig_7series_v1_9_round_robin_arb.v"
add_file -verilog "../../user_design/rtl/ecc/mig_7series_v1_9_ecc_buf.v"
add_file -verilog "../../user_design/rtl/ecc/mig_7series_v1_9_ecc_dec_fix.v"
add_file -verilog "../../user_design/rtl/ecc/mig_7series_v1_9_ecc_gen.v"
add_file -verilog "../../user_design/rtl/ecc/mig_7series_v1_9_ecc_merge_enc.v"
add_file -verilog "../../user_design/rtl/ip_top/mig_7series_v1_9_mem_intfc.v"
add_file -verilog "../../user_design/rtl/ip_top/mig_7series_v1_9_memc_ui_top_std.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_byte_group_io.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_byte_lane.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_calib_top.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_if_post_fifo.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_mc_phy.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_mc_phy_wrapper.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_of_pre_fifo.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_4lanes.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_dqs_found_cal.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_dqs_found_cal_hr.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_init.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_oclkdelay_cal.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_prbs_rdlvl.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_rdlvl.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_tempmon.v"
add_file -vhdl "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_top.vhd"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_wrcal.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_phy_wrlvl_off_delay.v"
add_file -verilog "../../user_design/rtl/phy/mig_7series_v1_9_ddr_prbs_gen.v"
add_file -verilog "../../user_design/rtl/ui/mig_7series_v1_9_ui_cmd.v"
add_file -verilog "../../user_design/rtl/ui/mig_7series_v1_9_ui_rd_data.v"
add_file -verilog "../../user_design/rtl/ui/mig_7series_v1_9_ui_top.v"
add_file -verilog "../../user_design/rtl/ui/mig_7series_v1_9_ui_wr_data.v"
add_file -vhdl "../../user_design/rtl/ddr.vhd"

#implementation: "rev_1"
impl -add rev_1 -type fpga

#device options
set_option -technology artix7
set_option -part xc7a100t
set_option -package csg324
set_option -speed_grade -1
set_option -part_companion ""

#compilation/mapping options
set_option -top_module "example_top"

# mapper_options
set_option -frequency 300.03
set_option -write_verilog 0
set_option -write_vhdl 0

#set result format/file last
project -result_file "../synth/rev_1/example_top.edf"

#implementation attributes
set_option -vlog_std v2001
set_option -project_relative_includes 1
impl -active "../synth/rev_1"
project -run
project -save
