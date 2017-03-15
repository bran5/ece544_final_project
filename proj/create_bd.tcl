
################################################################
# This is a generated script based on design: embsys
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2016.2
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   catch {common::send_msg_id "BD_TCL-109" "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source embsys_script.tcl

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xc7a100tcsg324-1
   set_property BOARD_PART digilentinc.com:nexys4_ddr:part0:1.1 [current_project]
}


# CHANGE DESIGN NAME HERE
set design_name embsys

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_msg_id "BD_TCL-001" "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_msg_id "BD_TCL-002" "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_msg_id "BD_TCL-003" "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_msg_id "BD_TCL-004" "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_msg_id "BD_TCL-005" "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_msg_id "BD_TCL-114" "ERROR" $errMsg}
   return $nRet
}

##################################################################
# DESIGN PROCs
##################################################################


# Hierarchical cell: microblaze_0_local_memory
proc create_hier_cell_microblaze_0_local_memory { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_msg_id "BD_TCL-102" "ERROR" create_hier_cell_microblaze_0_local_memory() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode MirroredMaster -vlnv xilinx.com:interface:lmb_rtl:1.0 DLMB
  create_bd_intf_pin -mode MirroredMaster -vlnv xilinx.com:interface:lmb_rtl:1.0 ILMB

  # Create pins
  create_bd_pin -dir I -type clk LMB_Clk
  create_bd_pin -dir I -from 0 -to 0 -type rst SYS_Rst

  # Create instance: dlmb_bram_if_cntlr, and set properties
  set dlmb_bram_if_cntlr [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:4.0 dlmb_bram_if_cntlr ]
  set_property -dict [ list \
CONFIG.C_ECC {0} \
 ] $dlmb_bram_if_cntlr

  # Create instance: dlmb_v10, and set properties
  set dlmb_v10 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_v10:3.0 dlmb_v10 ]

  # Create instance: ilmb_bram_if_cntlr, and set properties
  set ilmb_bram_if_cntlr [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:4.0 ilmb_bram_if_cntlr ]
  set_property -dict [ list \
CONFIG.C_ECC {0} \
 ] $ilmb_bram_if_cntlr

  # Create instance: ilmb_v10, and set properties
  set ilmb_v10 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_v10:3.0 ilmb_v10 ]

  # Create instance: lmb_bram, and set properties
  set lmb_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:8.3 lmb_bram ]
  set_property -dict [ list \
CONFIG.Memory_Type {True_Dual_Port_RAM} \
CONFIG.use_bram_block {BRAM_Controller} \
 ] $lmb_bram

  # Create interface connections
  connect_bd_intf_net -intf_net microblaze_0_dlmb [get_bd_intf_pins DLMB] [get_bd_intf_pins dlmb_v10/LMB_M]
  connect_bd_intf_net -intf_net microblaze_0_dlmb_bus [get_bd_intf_pins dlmb_bram_if_cntlr/SLMB] [get_bd_intf_pins dlmb_v10/LMB_Sl_0]
  connect_bd_intf_net -intf_net microblaze_0_dlmb_cntlr [get_bd_intf_pins dlmb_bram_if_cntlr/BRAM_PORT] [get_bd_intf_pins lmb_bram/BRAM_PORTA]
  connect_bd_intf_net -intf_net microblaze_0_ilmb [get_bd_intf_pins ILMB] [get_bd_intf_pins ilmb_v10/LMB_M]
  connect_bd_intf_net -intf_net microblaze_0_ilmb_bus [get_bd_intf_pins ilmb_bram_if_cntlr/SLMB] [get_bd_intf_pins ilmb_v10/LMB_Sl_0]
  connect_bd_intf_net -intf_net microblaze_0_ilmb_cntlr [get_bd_intf_pins ilmb_bram_if_cntlr/BRAM_PORT] [get_bd_intf_pins lmb_bram/BRAM_PORTB]

  # Create port connections
  connect_bd_net -net SYS_Rst_1 [get_bd_pins SYS_Rst] [get_bd_pins dlmb_bram_if_cntlr/LMB_Rst] [get_bd_pins dlmb_v10/SYS_Rst] [get_bd_pins ilmb_bram_if_cntlr/LMB_Rst] [get_bd_pins ilmb_v10/SYS_Rst]
  connect_bd_net -net microblaze_0_Clk [get_bd_pins LMB_Clk] [get_bd_pins dlmb_bram_if_cntlr/LMB_Clk] [get_bd_pins dlmb_v10/LMB_Clk] [get_bd_pins ilmb_bram_if_cntlr/LMB_Clk] [get_bd_pins ilmb_v10/LMB_Clk]

  # Restore current instance
  current_bd_instance $oldCurInst
}


# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_msg_id "BD_TCL-100" "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_msg_id "BD_TCL-101" "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set BT_out [ create_bd_intf_port -mode Master -vlnv digilentinc.com:interface:pmod_rtl:1.0 BT_out ]
  set PmodOLEDrgb_out [ create_bd_intf_port -mode Master -vlnv digilentinc.com:interface:pmod_rtl:1.0 PmodOLEDrgb_out ]
  set uart_rtl [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:uart_rtl:1.0 uart_rtl ]

  # Create ports
  set RGB1_Blue [ create_bd_port -dir O RGB1_Blue ]
  set RGB1_Green [ create_bd_port -dir O RGB1_Green ]
  set RGB1_Red [ create_bd_port -dir O RGB1_Red ]
  set RGB2_Blue [ create_bd_port -dir O RGB2_Blue ]
  set RGB2_Green [ create_bd_port -dir O RGB2_Green ]
  set RGB2_Red [ create_bd_port -dir O RGB2_Red ]
  set an [ create_bd_port -dir O -from 7 -to 0 an ]
  set btnC [ create_bd_port -dir I btnC ]
  set btnD [ create_bd_port -dir I btnD ]
  set btnL [ create_bd_port -dir I btnL ]
  set btnR [ create_bd_port -dir I btnR ]
  set btnU [ create_bd_port -dir I btnU ]
  set dp [ create_bd_port -dir O dp ]
  set led [ create_bd_port -dir O -from 15 -to 0 led ]
  set motor_pwm_out [ create_bd_port -dir O motor_pwm_out ]
  set motor_pwm_out_1 [ create_bd_port -dir O motor_pwm_out_1 ]
  set motor_sw [ create_bd_port -dir I motor_sw ]
  set motor_sw_1 [ create_bd_port -dir I motor_sw_1 ]
  set seg [ create_bd_port -dir O -from 6 -to 0 seg ]
  set sw [ create_bd_port -dir I -from 15 -to 0 sw ]
  set sysclk [ create_bd_port -dir I -type clk sysclk ]
  set sysreset_n [ create_bd_port -dir I -type rst sysreset_n ]
  set_property -dict [ list \
CONFIG.POLARITY {ACTIVE_LOW} \
 ] $sysreset_n
  set uart_rx [ create_bd_port -dir I uart_rx ]
  set vauxn3 [ create_bd_port -dir I vauxn3 ]
  set vauxp3 [ create_bd_port -dir I vauxp3 ]

  # Create instance: PmodBT2_0, and set properties
  set PmodBT2_0 [ create_bd_cell -type ip -vlnv digilentinc.com:IP:PmodBT2:1.0 PmodBT2_0 ]
  set_property -dict [ list \
CONFIG.PMOD {jb} \
CONFIG.USE_BOARD_FLOW {true} \
 ] $PmodBT2_0

  # Create instance: PmodOLEDrgb_0, and set properties
  set PmodOLEDrgb_0 [ create_bd_cell -type ip -vlnv digilentinc.com:IP:PmodOLEDrgb:1.0 PmodOLEDrgb_0 ]

  # Create instance: axi_timer_0, and set properties
  set axi_timer_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_timer:2.0 axi_timer_0 ]

  # Create instance: axi_uartlite_0, and set properties
  set axi_uartlite_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_uartlite:2.0 axi_uartlite_0 ]
  set_property -dict [ list \
CONFIG.C_BAUDRATE {115200} \
CONFIG.C_S_AXI_ACLK_FREQ_HZ {100000000} \
CONFIG.UARTLITE_BOARD_INTERFACE {Custom} \
CONFIG.USE_BOARD_FLOW {true} \
 ] $axi_uartlite_0

  # Need to retain value_src of defaults
  set_property -dict [ list \
CONFIG.C_S_AXI_ACLK_FREQ_HZ.VALUE_SRC {DEFAULT} \
 ] $axi_uartlite_0

  # Create instance: axi_uartlite_1, and set properties
  set axi_uartlite_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_uartlite:2.0 axi_uartlite_1 ]
  set_property -dict [ list \
CONFIG.C_S_AXI_ACLK_FREQ_HZ {100000000} \
 ] $axi_uartlite_1

  # Need to retain value_src of defaults
  set_property -dict [ list \
CONFIG.C_S_AXI_ACLK_FREQ_HZ.VALUE_SRC {DEFAULT} \
 ] $axi_uartlite_1

  # Create instance: clk_wiz_1, and set properties
  set clk_wiz_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:clk_wiz:5.3 clk_wiz_1 ]
  set_property -dict [ list \
CONFIG.CLKOUT1_JITTER {130.958} \
CONFIG.CLKOUT1_PHASE_ERROR {98.575} \
CONFIG.CLKOUT2_JITTER {151.636} \
CONFIG.CLKOUT2_PHASE_ERROR {98.575} \
CONFIG.CLKOUT2_REQUESTED_OUT_FREQ {50} \
CONFIG.CLKOUT2_USED {true} \
CONFIG.CLK_IN1_BOARD_INTERFACE {Custom} \
CONFIG.CLK_IN2_BOARD_INTERFACE {Custom} \
CONFIG.MMCM_CLKFBOUT_MULT_F {10.000} \
CONFIG.MMCM_CLKIN1_PERIOD {10.0} \
CONFIG.MMCM_CLKIN2_PERIOD {10.0} \
CONFIG.MMCM_CLKOUT0_DIVIDE_F {10.000} \
CONFIG.MMCM_CLKOUT1_DIVIDE {20} \
CONFIG.MMCM_COMPENSATION {ZHOLD} \
CONFIG.MMCM_DIVCLK_DIVIDE {1} \
CONFIG.NUM_OUT_CLKS {2} \
CONFIG.PRIM_SOURCE {Single_ended_clock_capable_pin} \
CONFIG.RESET_BOARD_INTERFACE {Custom} \
CONFIG.RESET_PORT {reset} \
CONFIG.RESET_TYPE {ACTIVE_HIGH} \
CONFIG.USE_BOARD_FLOW {true} \
CONFIG.USE_RESET {false} \
 ] $clk_wiz_1

  # Need to retain value_src of defaults
  set_property -dict [ list \
CONFIG.CLKOUT1_JITTER.VALUE_SRC {DEFAULT} \
CONFIG.CLKOUT1_PHASE_ERROR.VALUE_SRC {DEFAULT} \
CONFIG.MMCM_CLKFBOUT_MULT_F.VALUE_SRC {DEFAULT} \
CONFIG.MMCM_CLKIN1_PERIOD.VALUE_SRC {DEFAULT} \
CONFIG.MMCM_CLKIN2_PERIOD.VALUE_SRC {DEFAULT} \
CONFIG.MMCM_CLKOUT0_DIVIDE_F.VALUE_SRC {DEFAULT} \
CONFIG.MMCM_COMPENSATION.VALUE_SRC {DEFAULT} \
 ] $clk_wiz_1

  # Create instance: fit_timer_0, and set properties
  set fit_timer_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:fit_timer:2.0 fit_timer_0 ]
  set_property -dict [ list \
CONFIG.C_NO_CLOCKS {2500} \
 ] $fit_timer_0

  # Create instance: mdm_1, and set properties
  set mdm_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:mdm:3.2 mdm_1 ]

  # Create instance: microblaze_0, and set properties
  set microblaze_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:microblaze:9.6 microblaze_0 ]
  set_property -dict [ list \
CONFIG.C_DEBUG_ENABLED {1} \
CONFIG.C_D_AXI {1} \
CONFIG.C_D_LMB {1} \
CONFIG.C_I_LMB {1} \
 ] $microblaze_0

  # Create instance: microblaze_0_axi_intc, and set properties
  set microblaze_0_axi_intc [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_intc:4.1 microblaze_0_axi_intc ]
  set_property -dict [ list \
CONFIG.C_HAS_FAST {1} \
 ] $microblaze_0_axi_intc

  # Create instance: microblaze_0_axi_periph, and set properties
  set microblaze_0_axi_periph [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 microblaze_0_axi_periph ]
  set_property -dict [ list \
CONFIG.NUM_MI {12} \
 ] $microblaze_0_axi_periph

  # Create instance: microblaze_0_local_memory
  create_hier_cell_microblaze_0_local_memory [current_bd_instance .] microblaze_0_local_memory

  # Create instance: microblaze_0_xlconcat, and set properties
  set microblaze_0_xlconcat [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 microblaze_0_xlconcat ]

  # Create instance: motor_driver_0, and set properties
  set motor_driver_0 [ create_bd_cell -type ip -vlnv garmin.com:user:motor_driver:1.0 motor_driver_0 ]

  # Create instance: motor_driver_1, and set properties
  set motor_driver_1 [ create_bd_cell -type ip -vlnv garmin.com:user:motor_driver:1.0 motor_driver_1 ]

  # Create instance: nexys4IO_0, and set properties
  set nexys4IO_0 [ create_bd_cell -type ip -vlnv xilinx.com:user:nexys4IO:2.0 nexys4IO_0 ]

  # Create instance: rst_clk_wiz_1_100M, and set properties
  set rst_clk_wiz_1_100M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_clk_wiz_1_100M ]
  set_property -dict [ list \
CONFIG.RESET_BOARD_INTERFACE {Custom} \
CONFIG.USE_BOARD_FLOW {true} \
 ] $rst_clk_wiz_1_100M

  # Create instance: xadc_wiz_0, and set properties
  set xadc_wiz_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xadc_wiz:3.3 xadc_wiz_0 ]
  set_property -dict [ list \
CONFIG.ADC_CONVERSION_RATE {1000} \
CONFIG.CHANNEL_ENABLE_VAUXP10_VAUXN10 {false} \
CONFIG.CHANNEL_ENABLE_VAUXP11_VAUXN11 {false} \
CONFIG.CHANNEL_ENABLE_VAUXP2_VAUXN2 {false} \
CONFIG.CHANNEL_ENABLE_VAUXP3_VAUXN3 {true} \
CONFIG.CHANNEL_ENABLE_VP_VN {true} \
CONFIG.DCLK_FREQUENCY {100} \
CONFIG.ENABLE_RESET {false} \
CONFIG.EXTERNAL_MUX_CHANNEL {VP_VN} \
CONFIG.INTERFACE_SELECTION {Enable_AXI} \
CONFIG.OT_ALARM {false} \
CONFIG.SEQUENCER_MODE {Continuous} \
CONFIG.SINGLE_CHANNEL_SELECTION {TEMPERATURE} \
CONFIG.USER_TEMP_ALARM {false} \
CONFIG.VCCAUX_ALARM {false} \
CONFIG.VCCINT_ALARM {false} \
CONFIG.XADC_STARUP_SELECTION {channel_sequencer} \
 ] $xadc_wiz_0

  # Need to retain value_src of defaults
  set_property -dict [ list \
CONFIG.ADC_CONVERSION_RATE.VALUE_SRC {DEFAULT} \
CONFIG.DCLK_FREQUENCY.VALUE_SRC {DEFAULT} \
CONFIG.ENABLE_RESET.VALUE_SRC {DEFAULT} \
CONFIG.INTERFACE_SELECTION.VALUE_SRC {DEFAULT} \
 ] $xadc_wiz_0

  # Create interface connections
  connect_bd_intf_net -intf_net PmodBT2_0_Pmod_out [get_bd_intf_ports BT_out] [get_bd_intf_pins PmodBT2_0/Pmod_out]
  connect_bd_intf_net -intf_net PmodOLEDrgb_0_PmodOLEDrgb_out [get_bd_intf_ports PmodOLEDrgb_out] [get_bd_intf_pins PmodOLEDrgb_0/PmodOLEDrgb_out]
  connect_bd_intf_net -intf_net axi_uartlite_0_UART [get_bd_intf_ports uart_rtl] [get_bd_intf_pins axi_uartlite_0/UART]
  connect_bd_intf_net -intf_net microblaze_0_axi_dp [get_bd_intf_pins microblaze_0/M_AXI_DP] [get_bd_intf_pins microblaze_0_axi_periph/S00_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M01_AXI [get_bd_intf_pins microblaze_0_axi_periph/M01_AXI] [get_bd_intf_pins nexys4IO_0/S00_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M02_AXI [get_bd_intf_pins PmodOLEDrgb_0/AXI_LITE_GPIO] [get_bd_intf_pins microblaze_0_axi_periph/M02_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M03_AXI [get_bd_intf_pins PmodOLEDrgb_0/AXI_LITE_SPI] [get_bd_intf_pins microblaze_0_axi_periph/M03_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M04_AXI [get_bd_intf_pins microblaze_0_axi_periph/M04_AXI] [get_bd_intf_pins motor_driver_1/S00_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M05_AXI [get_bd_intf_pins axi_timer_0/S_AXI] [get_bd_intf_pins microblaze_0_axi_periph/M05_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M06_AXI [get_bd_intf_pins axi_uartlite_0/S_AXI] [get_bd_intf_pins microblaze_0_axi_periph/M06_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M07_AXI [get_bd_intf_pins microblaze_0_axi_periph/M07_AXI] [get_bd_intf_pins xadc_wiz_0/s_axi_lite]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M08_AXI [get_bd_intf_pins PmodBT2_0/AXI_LITE_UART] [get_bd_intf_pins microblaze_0_axi_periph/M08_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M09_AXI [get_bd_intf_pins PmodBT2_0/AXI_LITE_GPIO] [get_bd_intf_pins microblaze_0_axi_periph/M09_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M10_AXI [get_bd_intf_pins microblaze_0_axi_periph/M10_AXI] [get_bd_intf_pins motor_driver_0/S00_AXI]
  connect_bd_intf_net -intf_net microblaze_0_axi_periph_M11_AXI [get_bd_intf_pins axi_uartlite_1/S_AXI] [get_bd_intf_pins microblaze_0_axi_periph/M11_AXI]
  connect_bd_intf_net -intf_net microblaze_0_debug [get_bd_intf_pins mdm_1/MBDEBUG_0] [get_bd_intf_pins microblaze_0/DEBUG]
  connect_bd_intf_net -intf_net microblaze_0_dlmb_1 [get_bd_intf_pins microblaze_0/DLMB] [get_bd_intf_pins microblaze_0_local_memory/DLMB]
  connect_bd_intf_net -intf_net microblaze_0_ilmb_1 [get_bd_intf_pins microblaze_0/ILMB] [get_bd_intf_pins microblaze_0_local_memory/ILMB]
  connect_bd_intf_net -intf_net microblaze_0_intc_axi [get_bd_intf_pins microblaze_0_axi_intc/s_axi] [get_bd_intf_pins microblaze_0_axi_periph/M00_AXI]
  connect_bd_intf_net -intf_net microblaze_0_interrupt [get_bd_intf_pins microblaze_0/INTERRUPT] [get_bd_intf_pins microblaze_0_axi_intc/interrupt]

  # Create port connections
  connect_bd_net -net axi_timer_0_generateout0 [get_bd_pins axi_timer_0/generateout0] [get_bd_pins nexys4IO_0/Clock]
  connect_bd_net -net axi_timer_0_interrupt [get_bd_pins axi_timer_0/interrupt] [get_bd_pins microblaze_0_xlconcat/In1]
  connect_bd_net -net btnC_1 [get_bd_ports btnC] [get_bd_pins nexys4IO_0/btnC]
  connect_bd_net -net btnD_1 [get_bd_ports btnD] [get_bd_pins nexys4IO_0/btnD]
  connect_bd_net -net btnL_1 [get_bd_ports btnL] [get_bd_pins nexys4IO_0/btnL]
  connect_bd_net -net btnR_1 [get_bd_ports btnR] [get_bd_pins nexys4IO_0/btnR]
  connect_bd_net -net btnU_1 [get_bd_ports btnU] [get_bd_pins nexys4IO_0/btnU]
  connect_bd_net -net clk_in1_1 [get_bd_ports sysclk] [get_bd_pins clk_wiz_1/clk_in1]
  connect_bd_net -net clk_wiz_1_clk_out2 [get_bd_pins PmodOLEDrgb_0/ext_spi_clk] [get_bd_pins clk_wiz_1/clk_out2]
  connect_bd_net -net clk_wiz_1_locked [get_bd_pins clk_wiz_1/locked] [get_bd_pins rst_clk_wiz_1_100M/dcm_locked]
  connect_bd_net -net fit_timer_0_Interrupt [get_bd_pins fit_timer_0/Interrupt] [get_bd_pins microblaze_0_xlconcat/In0]
  connect_bd_net -net mdm_1_debug_sys_rst [get_bd_pins mdm_1/Debug_SYS_Rst] [get_bd_pins rst_clk_wiz_1_100M/mb_debug_sys_rst]
  connect_bd_net -net microblaze_0_Clk [get_bd_pins PmodBT2_0/s_axi_aclk] [get_bd_pins PmodOLEDrgb_0/s_axi_aclk] [get_bd_pins PmodOLEDrgb_0/s_axi_aclk2] [get_bd_pins axi_timer_0/s_axi_aclk] [get_bd_pins axi_uartlite_0/s_axi_aclk] [get_bd_pins axi_uartlite_1/s_axi_aclk] [get_bd_pins clk_wiz_1/clk_out1] [get_bd_pins fit_timer_0/Clk] [get_bd_pins microblaze_0/Clk] [get_bd_pins microblaze_0_axi_intc/processor_clk] [get_bd_pins microblaze_0_axi_intc/s_axi_aclk] [get_bd_pins microblaze_0_axi_periph/ACLK] [get_bd_pins microblaze_0_axi_periph/M00_ACLK] [get_bd_pins microblaze_0_axi_periph/M01_ACLK] [get_bd_pins microblaze_0_axi_periph/M02_ACLK] [get_bd_pins microblaze_0_axi_periph/M03_ACLK] [get_bd_pins microblaze_0_axi_periph/M04_ACLK] [get_bd_pins microblaze_0_axi_periph/M05_ACLK] [get_bd_pins microblaze_0_axi_periph/M06_ACLK] [get_bd_pins microblaze_0_axi_periph/M07_ACLK] [get_bd_pins microblaze_0_axi_periph/M08_ACLK] [get_bd_pins microblaze_0_axi_periph/M09_ACLK] [get_bd_pins microblaze_0_axi_periph/M10_ACLK] [get_bd_pins microblaze_0_axi_periph/M11_ACLK] [get_bd_pins microblaze_0_axi_periph/S00_ACLK] [get_bd_pins microblaze_0_local_memory/LMB_Clk] [get_bd_pins motor_driver_0/s00_axi_aclk] [get_bd_pins motor_driver_1/s00_axi_aclk] [get_bd_pins nexys4IO_0/s00_axi_aclk] [get_bd_pins rst_clk_wiz_1_100M/slowest_sync_clk] [get_bd_pins xadc_wiz_0/s_axi_aclk]
  connect_bd_net -net microblaze_0_intr [get_bd_pins microblaze_0_axi_intc/intr] [get_bd_pins microblaze_0_xlconcat/dout]
  connect_bd_net -net motor_driver_0_motor_pwm_out [get_bd_ports motor_pwm_out] [get_bd_pins motor_driver_0/motor_pwm_out]
  connect_bd_net -net motor_driver_1_motor_pwm_out [get_bd_ports motor_pwm_out_1] [get_bd_pins motor_driver_1/motor_pwm_out]
  connect_bd_net -net motor_sw_1 [get_bd_ports motor_sw] [get_bd_pins motor_driver_0/motor_sw]
  connect_bd_net -net motor_sw_1_1 [get_bd_ports motor_sw_1] [get_bd_pins motor_driver_1/motor_sw]
  connect_bd_net -net nexys4IO_0_RGB1_Blue [get_bd_ports RGB1_Blue] [get_bd_pins nexys4IO_0/RGB1_Blue]
  connect_bd_net -net nexys4IO_0_RGB1_Green [get_bd_ports RGB1_Green] [get_bd_pins nexys4IO_0/RGB1_Green]
  connect_bd_net -net nexys4IO_0_RGB1_Red [get_bd_ports RGB1_Red] [get_bd_pins nexys4IO_0/RGB1_Red]
  connect_bd_net -net nexys4IO_0_RGB2_Blue [get_bd_ports RGB2_Blue] [get_bd_pins nexys4IO_0/RGB2_Blue]
  connect_bd_net -net nexys4IO_0_RGB2_Green [get_bd_ports RGB2_Green] [get_bd_pins nexys4IO_0/RGB2_Green]
  connect_bd_net -net nexys4IO_0_RGB2_Red [get_bd_ports RGB2_Red] [get_bd_pins nexys4IO_0/RGB2_Red]
  connect_bd_net -net nexys4IO_0_an [get_bd_ports an] [get_bd_pins nexys4IO_0/an]
  connect_bd_net -net nexys4IO_0_dp [get_bd_ports dp] [get_bd_pins nexys4IO_0/dp]
  connect_bd_net -net nexys4IO_0_led [get_bd_ports led] [get_bd_pins nexys4IO_0/led]
  connect_bd_net -net nexys4IO_0_seg [get_bd_ports seg] [get_bd_pins nexys4IO_0/seg]
  connect_bd_net -net reset_1 [get_bd_ports sysreset_n] [get_bd_pins rst_clk_wiz_1_100M/ext_reset_in]
  connect_bd_net -net rst_clk_wiz_1_100M_bus_struct_reset [get_bd_pins microblaze_0_local_memory/SYS_Rst] [get_bd_pins rst_clk_wiz_1_100M/bus_struct_reset]
  connect_bd_net -net rst_clk_wiz_1_100M_interconnect_aresetn [get_bd_pins microblaze_0_axi_periph/ARESETN] [get_bd_pins rst_clk_wiz_1_100M/interconnect_aresetn]
  connect_bd_net -net rst_clk_wiz_1_100M_mb_reset [get_bd_pins microblaze_0/Reset] [get_bd_pins microblaze_0_axi_intc/processor_rst] [get_bd_pins rst_clk_wiz_1_100M/mb_reset]
  connect_bd_net -net rst_clk_wiz_1_100M_peripheral_aresetn [get_bd_pins PmodBT2_0/s_axi_aresetn] [get_bd_pins PmodOLEDrgb_0/s_axi_aresetn] [get_bd_pins axi_timer_0/s_axi_aresetn] [get_bd_pins axi_uartlite_0/s_axi_aresetn] [get_bd_pins axi_uartlite_1/s_axi_aresetn] [get_bd_pins microblaze_0_axi_intc/s_axi_aresetn] [get_bd_pins microblaze_0_axi_periph/M00_ARESETN] [get_bd_pins microblaze_0_axi_periph/M01_ARESETN] [get_bd_pins microblaze_0_axi_periph/M02_ARESETN] [get_bd_pins microblaze_0_axi_periph/M03_ARESETN] [get_bd_pins microblaze_0_axi_periph/M04_ARESETN] [get_bd_pins microblaze_0_axi_periph/M05_ARESETN] [get_bd_pins microblaze_0_axi_periph/M06_ARESETN] [get_bd_pins microblaze_0_axi_periph/M07_ARESETN] [get_bd_pins microblaze_0_axi_periph/M08_ARESETN] [get_bd_pins microblaze_0_axi_periph/M09_ARESETN] [get_bd_pins microblaze_0_axi_periph/M10_ARESETN] [get_bd_pins microblaze_0_axi_periph/M11_ARESETN] [get_bd_pins microblaze_0_axi_periph/S00_ARESETN] [get_bd_pins motor_driver_0/s00_axi_aresetn] [get_bd_pins motor_driver_1/s00_axi_aresetn] [get_bd_pins nexys4IO_0/s00_axi_aresetn] [get_bd_pins rst_clk_wiz_1_100M/peripheral_aresetn] [get_bd_pins xadc_wiz_0/s_axi_aresetn]
  connect_bd_net -net rst_clk_wiz_1_100M_peripheral_reset [get_bd_pins fit_timer_0/Rst] [get_bd_pins rst_clk_wiz_1_100M/peripheral_reset]
  connect_bd_net -net rx_1 [get_bd_ports uart_rx] [get_bd_pins axi_uartlite_1/rx]
  connect_bd_net -net sw_1 [get_bd_ports sw] [get_bd_pins nexys4IO_0/sw]
  connect_bd_net -net vauxn3_1 [get_bd_ports vauxn3] [get_bd_pins xadc_wiz_0/vauxn3]
  connect_bd_net -net vauxp3_1 [get_bd_ports vauxp3] [get_bd_pins xadc_wiz_0/vauxp3]

  # Create address segments
  create_bd_addr_seg -range 0x00002000 -offset 0x00020000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs PmodBT2_0/AXI_LITE_UART/Reg0] SEG_PmodBT2_0_Reg0
  create_bd_addr_seg -range 0x00001000 -offset 0x00030000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs PmodBT2_0/AXI_LITE_GPIO/Reg0] SEG_PmodBT2_0_Reg01
  create_bd_addr_seg -range 0x00010000 -offset 0x44A10000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs PmodOLEDrgb_0/AXI_LITE_GPIO/AXI_LITE_GPIO_reg] SEG_PmodOLEDrgb_0_AXI_LITE_GPIO_reg
  create_bd_addr_seg -range 0x00010000 -offset 0x44A20000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs PmodOLEDrgb_0/AXI_LITE_SPI/AXI_LITE_SPI_reg] SEG_PmodOLEDrgb_0_AXI_LITE_SPI_reg
  create_bd_addr_seg -range 0x00010000 -offset 0x41C00000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs axi_timer_0/S_AXI/Reg] SEG_axi_timer_0_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x40600000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs axi_uartlite_0/S_AXI/Reg] SEG_axi_uartlite_0_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x40610000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs axi_uartlite_1/S_AXI/Reg] SEG_axi_uartlite_1_Reg
  create_bd_addr_seg -range 0x00020000 -offset 0x00000000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs microblaze_0_local_memory/dlmb_bram_if_cntlr/SLMB/Mem] SEG_dlmb_bram_if_cntlr_Mem
  create_bd_addr_seg -range 0x00020000 -offset 0x00000000 [get_bd_addr_spaces microblaze_0/Instruction] [get_bd_addr_segs microblaze_0_local_memory/ilmb_bram_if_cntlr/SLMB/Mem] SEG_ilmb_bram_if_cntlr_Mem
  create_bd_addr_seg -range 0x00010000 -offset 0x41200000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs microblaze_0_axi_intc/S_AXI/Reg] SEG_microblaze_0_axi_intc_Reg
  create_bd_addr_seg -range 0x00010000 -offset 0x44A40000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs motor_driver_0/S00_AXI/S00_AXI_reg] SEG_motor_driver_0_S00_AXI_reg
  create_bd_addr_seg -range 0x00010000 -offset 0x44A50000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs motor_driver_1/S00_AXI/S00_AXI_reg] SEG_motor_driver_1_S00_AXI_reg
  create_bd_addr_seg -range 0x00010000 -offset 0x44A00000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs nexys4IO_0/S00_AXI/S00_AXI_reg] SEG_nexys4IO_0_S00_AXI_reg
  create_bd_addr_seg -range 0x00010000 -offset 0x44A30000 [get_bd_addr_spaces microblaze_0/Data] [get_bd_addr_segs xadc_wiz_0/s_axi_lite/Reg] SEG_xadc_wiz_0_Reg

  # Perform GUI Layout
  regenerate_bd_layout -layout_string {
   guistr: "# # String gsaved with Nlview 6.5.12  2016-01-29 bk=1.3547 VDI=39 GEI=35 GUI=JA:1.6
#  -string -flagsOSRD
preplace port motor_pwm_out_1 -pg 1 -y 340 -defaultsOSRD
preplace port uart_rtl -pg 1 -y 650 -defaultsOSRD
preplace port BT_out -pg 1 -y 960 -defaultsOSRD
preplace port motor_sw_1 -pg 1 -y 340 -defaultsOSRD
preplace port btnL -pg 1 -y 120 -defaultsOSRD
preplace port vauxp3 -pg 1 -y 1600 -defaultsOSRD
preplace port RGB1_Blue -pg 1 -y 100 -defaultsOSRD
preplace port btnC -pg 1 -y 140 -defaultsOSRD
preplace port vauxn3 -pg 1 -y 1580 -defaultsOSRD
preplace port btnD -pg 1 -y 80 -defaultsOSRD
preplace port dp -pg 1 -y 200 -defaultsOSRD
preplace port btnR -pg 1 -y 100 -defaultsOSRD
preplace port sysclk -pg 1 -y 1480 -defaultsOSRD
preplace port RGB2_Blue -pg 1 -y 160 -defaultsOSRD
preplace port sysreset_n -pg 1 -y 1410 -defaultsOSRD
preplace port motor_sw -pg 1 -y 490 -defaultsOSRD
preplace port RGB1_Red -pg 1 -y 60 -defaultsOSRD
preplace port motor_pwm_out -pg 1 -y 500 -defaultsOSRD
preplace port btnU -pg 1 -y 60 -defaultsOSRD
preplace port RGB2_Red -pg 1 -y 120 -defaultsOSRD
preplace port RGB2_Green -pg 1 -y 140 -defaultsOSRD
preplace port uart_rx -pg 1 -y 1170 -defaultsOSRD
preplace port RGB1_Green -pg 1 -y 80 -defaultsOSRD
preplace port PmodOLEDrgb_out -pg 1 -y 810 -defaultsOSRD
preplace portBus sw -pg 1 -y 180 -defaultsOSRD
preplace portBus an -pg 1 -y 220 -defaultsOSRD
preplace portBus led -pg 1 -y 40 -defaultsOSRD
preplace portBus seg -pg 1 -y 180 -defaultsOSRD
preplace inst xadc_wiz_0 -pg 1 -lvl 8 -y 1480 -defaultsOSRD
preplace inst PmodBT2_0 -pg 1 -lvl 8 -y 970 -defaultsOSRD
preplace inst microblaze_0_axi_periph -pg 1 -lvl 7 -y 830 -defaultsOSRD
preplace inst fit_timer_0 -pg 1 -lvl 3 -y 1340 -defaultsOSRD
preplace inst motor_driver_0 -pg 1 -lvl 8 -y 500 -defaultsOSRD
preplace inst microblaze_0_xlconcat -pg 1 -lvl 4 -y 1340 -defaultsOSRD
preplace inst axi_timer_0 -pg 1 -lvl 8 -y 1270 -defaultsOSRD
preplace inst PmodOLEDrgb_0 -pg 1 -lvl 8 -y 810 -defaultsOSRD
preplace inst motor_driver_1 -pg 1 -lvl 8 -y 340 -defaultsOSRD
preplace inst microblaze_0_axi_intc -pg 1 -lvl 5 -y 1310 -defaultsOSRD
preplace inst mdm_1 -pg 1 -lvl 5 -y 1530 -defaultsOSRD
preplace inst nexys4IO_0 -pg 1 -lvl 8 -y 130 -defaultsOSRD
preplace inst microblaze_0 -pg 1 -lvl 6 -y 1340 -defaultsOSRD
preplace inst axi_uartlite_0 -pg 1 -lvl 8 -y 660 -defaultsOSRD
preplace inst axi_uartlite_1 -pg 1 -lvl 8 -y 1120 -defaultsOSRD
preplace inst rst_clk_wiz_1_100M -pg 1 -lvl 2 -y 1440 -defaultsOSRD
preplace inst microblaze_0_local_memory -pg 1 -lvl 7 -y 1350 -defaultsOSRD
preplace inst clk_wiz_1 -pg 1 -lvl 1 -y 1480 -defaultsOSRD
preplace netloc PmodOLEDrgb_0_PmodOLEDrgb_out 1 8 1 NJ
preplace netloc rx_1 1 0 9 NJ 1170 NJ 1170 NJ 1170 NJ 1170 NJ 1170 NJ 1170 NJ 1170 NJ 1050 2410
preplace netloc vauxn3_1 1 0 8 NJ 1580 NJ 1580 NJ 1460 NJ 1460 NJ 1460 NJ 1460 NJ 1460 NJ
preplace netloc microblaze_0_axi_periph_M08_AXI 1 7 1 2070
preplace netloc microblaze_0_axi_periph_M04_AXI 1 7 1 2020
preplace netloc vauxp3_1 1 0 8 NJ 1600 NJ 1600 NJ 1470 NJ 1470 NJ 1470 NJ 1470 NJ 1470 NJ
preplace netloc motor_sw_1 1 0 8 NJ 490 NJ 490 NJ 490 NJ 490 NJ 490 NJ 490 NJ 490 NJ
preplace netloc motor_sw_1_1 1 0 8 NJ 340 NJ 340 NJ 340 NJ 340 NJ 340 NJ 340 NJ 340 NJ
preplace netloc motor_driver_1_motor_pwm_out 1 8 1 NJ
preplace netloc microblaze_0_intr 1 4 1 930
preplace netloc nexys4IO_0_led 1 8 1 NJ
preplace netloc microblaze_0_Clk 1 1 7 200 1350 560 1400 NJ 1400 950 1410 1170 1240 1660 440 2030
preplace netloc clk_in1_1 1 0 1 NJ
preplace netloc btnD_1 1 0 8 NJ 80 NJ 80 NJ 80 NJ 80 NJ 80 NJ 80 NJ 80 NJ
preplace netloc btnC_1 1 0 8 NJ 140 NJ 140 NJ 140 NJ 140 NJ 140 NJ 140 NJ 140 NJ
preplace netloc microblaze_0_interrupt 1 5 1 N
preplace netloc microblaze_0_intc_axi 1 4 4 950 480 NJ 480 NJ 480 1990
preplace netloc microblaze_0_axi_periph_M06_AXI 1 7 1 2010
preplace netloc microblaze_0_axi_periph_M03_AXI 1 7 1 N
preplace netloc nexys4IO_0_RGB2_Blue 1 8 1 NJ
preplace netloc btnU_1 1 0 8 NJ 60 NJ 60 NJ 60 NJ 60 NJ 60 NJ 60 NJ 60 NJ
preplace netloc btnL_1 1 0 8 NJ 120 NJ 120 NJ 120 NJ 120 NJ 120 NJ 120 NJ 120 NJ
preplace netloc microblaze_0_ilmb_1 1 6 1 N
preplace netloc microblaze_0_axi_periph_M10_AXI 1 7 1 2060
preplace netloc nexys4IO_0_RGB1_Blue 1 8 1 NJ
preplace netloc microblaze_0_axi_periph_M05_AXI 1 7 1 2020
preplace netloc microblaze_0_axi_dp 1 6 1 1650
preplace netloc nexys4IO_0_dp 1 8 1 NJ
preplace netloc nexys4IO_0_RGB1_Red 1 8 1 NJ
preplace netloc fit_timer_0_Interrupt 1 3 1 750
preplace netloc axi_timer_0_generateout0 1 7 2 2080 1370 2410
preplace netloc rst_clk_wiz_1_100M_interconnect_aresetn 1 2 5 NJ 590 NJ 590 NJ 590 NJ 590 N
preplace netloc rst_clk_wiz_1_100M_bus_struct_reset 1 2 5 N 1420 NJ 1420 NJ 1420 NJ 1260 NJ
preplace netloc nexys4IO_0_RGB2_Green 1 8 1 NJ
preplace netloc microblaze_0_axi_periph_M01_AXI 1 7 1 2000
preplace netloc rst_clk_wiz_1_100M_peripheral_aresetn 1 2 6 NJ 1280 NJ 1280 930 1110 NJ 1110 1640 470 2040
preplace netloc rst_clk_wiz_1_100M_mb_reset 1 2 4 530 1260 NJ 1260 940 1210 NJ
preplace netloc clk_wiz_1_locked 1 1 1 N
preplace netloc axi_uartlite_0_UART 1 8 1 NJ
preplace netloc clk_wiz_1_clk_out2 1 1 7 NJ 460 NJ 460 NJ 460 NJ 460 NJ 460 NJ 460 2050
preplace netloc microblaze_0_dlmb_1 1 6 1 N
preplace netloc microblaze_0_axi_periph_M07_AXI 1 7 1 2000
preplace netloc microblaze_0_axi_periph_M02_AXI 1 7 1 N
preplace netloc PmodBT2_0_Pmod_out 1 8 1 NJ
preplace netloc sw_1 1 0 8 NJ 180 NJ 180 NJ 180 NJ 180 NJ 180 NJ 180 NJ 180 NJ
preplace netloc nexys4IO_0_an 1 8 1 NJ
preplace netloc microblaze_0_debug 1 5 1 1200
preplace netloc rst_clk_wiz_1_100M_peripheral_reset 1 2 1 570
preplace netloc reset_1 1 0 2 NJ 1410 NJ
preplace netloc motor_driver_0_motor_pwm_out 1 8 1 NJ
preplace netloc mdm_1_debug_sys_rst 1 1 5 210 1610 NJ 1610 NJ 1610 NJ 1610 1180
preplace netloc btnR_1 1 0 8 NJ 100 NJ 100 NJ 100 NJ 100 NJ 100 NJ 100 NJ 100 NJ
preplace netloc microblaze_0_axi_periph_M11_AXI 1 7 1 1990
preplace netloc microblaze_0_axi_periph_M09_AXI 1 7 1 2010
preplace netloc nexys4IO_0_seg 1 8 1 NJ
preplace netloc nexys4IO_0_RGB2_Red 1 8 1 NJ
preplace netloc nexys4IO_0_RGB1_Green 1 8 1 NJ
preplace netloc axi_timer_0_interrupt 1 3 6 750 1590 NJ 1590 NJ 1590 NJ 1590 NJ 1590 2400
levelinfo -pg 1 0 100 370 660 840 1060 1420 1840 2250 2430 -top 0 -bot 1620
",
}

  # Restore current instance
  current_bd_instance $oldCurInst

  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


