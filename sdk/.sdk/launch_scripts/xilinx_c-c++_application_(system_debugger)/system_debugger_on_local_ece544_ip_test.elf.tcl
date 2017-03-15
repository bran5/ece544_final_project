connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292747175A"} -index 0
loadhw C:/Users/biodrowski/Documents/ECE544/final_project/sdk/n4fpga_hw_platform_0/system.hdf
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292747175A"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "microblaze*#0" && bscan=="USER2"  && jtag_cable_name =~ "Digilent Nexys4DDR 210292747175A"} -index 0
dow C:/Users/biodrowski/Documents/ECE544/final_project/sdk/ece544_IP_test/Debug/ece544_IP_test.elf
bpadd -addr &main
