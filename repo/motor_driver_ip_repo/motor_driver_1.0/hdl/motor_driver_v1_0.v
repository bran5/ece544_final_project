
`timescale 1 ns / 1 ps

	module motor_driver_v1_0 #
	(
		// Users to add parameters here
        parameter PWM_PERIOD_FREQ_HZ = 2000,
        parameter CLOCK_FREQ_HZ	= 100000000,
		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S00_AXI
		parameter integer C_S00_AXI_DATA_WIDTH	= 32,
		parameter integer C_S00_AXI_ADDR_WIDTH	= 5
	)
	(
		// Users to add ports here
        input               motor_encoder_in,
        input               motor_sw,
        output              motor_dir_outa,
        output              motor_dir_outb,
        output              motor_pwm_out,
        output				motor_ena_out,
        output              motor_enb_out,
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S00_AXI
		input wire  s00_axi_aclk,
		input wire  s00_axi_aresetn,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_awaddr,
		input wire [2 : 0] s00_axi_awprot,
		input wire  s00_axi_awvalid,
		output wire  s00_axi_awready,
		input wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_wdata,
		input wire [(C_S00_AXI_DATA_WIDTH/8)-1 : 0] s00_axi_wstrb,
		input wire  s00_axi_wvalid,
		output wire  s00_axi_wready,
		output wire [1 : 0] s00_axi_bresp,
		output wire  s00_axi_bvalid,
		input wire  s00_axi_bready,
		input wire [C_S00_AXI_ADDR_WIDTH-1 : 0] s00_axi_araddr,
		input wire [2 : 0] s00_axi_arprot,
		input wire  s00_axi_arvalid,
		output wire  s00_axi_arready,
		output wire [C_S00_AXI_DATA_WIDTH-1 : 0] s00_axi_rdata,
		output wire [1 : 0] s00_axi_rresp,
		output wire  s00_axi_rvalid,
		input wire  s00_axi_rready
	);
    
    //declare interconnect wires
    wire [31:0] on_set_point_in;
    wire        motor_dir_in;
    wire        count_ready_out;
    wire [31:0] count_high_out;
    
    
// Instantiation of Axi Bus Interface S00_AXI
	motor_driver_v1_0_S00_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S00_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S00_AXI_ADDR_WIDTH)
	) motor_driver_v1_0_S00_AXI_inst (
        
        //Peripheral-specific signals (to/from register data)
        .on_set_point_in(on_set_point_in),
        .motor_dir_in(motor_dir_in),
        .count_ready_out(count_ready_out),
        .count_high_out(count_high_out),
    
        //AXI Lite Signals
		.S_AXI_ACLK(s00_axi_aclk),
		.S_AXI_ARESETN(s00_axi_aresetn),
		.S_AXI_AWADDR(s00_axi_awaddr),
		.S_AXI_AWPROT(s00_axi_awprot),
		.S_AXI_AWVALID(s00_axi_awvalid),
		.S_AXI_AWREADY(s00_axi_awready),
		.S_AXI_WDATA(s00_axi_wdata),
		.S_AXI_WSTRB(s00_axi_wstrb),
		.S_AXI_WVALID(s00_axi_wvalid),
		.S_AXI_WREADY(s00_axi_wready),
		.S_AXI_BRESP(s00_axi_bresp),
		.S_AXI_BVALID(s00_axi_bvalid),
		.S_AXI_BREADY(s00_axi_bready),
		.S_AXI_ARADDR(s00_axi_araddr),
		.S_AXI_ARPROT(s00_axi_arprot),
		.S_AXI_ARVALID(s00_axi_arvalid),
		.S_AXI_ARREADY(s00_axi_arready),
		.S_AXI_RDATA(s00_axi_rdata),
		.S_AXI_RRESP(s00_axi_rresp),
		.S_AXI_RVALID(s00_axi_rvalid),
		.S_AXI_RREADY(s00_axi_rready)
	);

	// Add user logic here
    motor_driver_top #(
        .PWM_PERIOD_FREQ_HZ(PWM_PERIOD_FREQ_HZ),
        .CLOCK_FREQ_HZ(CLOCK_FREQ_HZ))
        
     motor_driver_top_0
    (
        .clk(s00_axi_aclk),                         //logic clock
        .reset(~s00_axi_aresetn),                   //global reset asserted high   
        
        //motor control input interface
        .motor_encoder_in(motor_encoder_in),        //from motor encoder
        .motor_sw(motor_sw),	                    //input for hard on/off switch
        
        //input from MB
        .on_set_point_in(on_set_point_in),          //control loop output from MB
        .motor_dir_in(motor_dir_in),                //directional signal from microblaze from MB rotary encoder switch(maybe connect through axi bus)
        
        
        //motor control output interface                   
        .motor_dir_outa(motor_dir_outa),            //to INA pin; assert high for clockwise direction, deassert when motor_dir_outb is high
        .motor_dir_outb(motor_dir_outb),            //to INA pin; assert high for counterclockwise direction, deassert when motor_dir_outa is high
        .motor_pwm_out(motor_pwm_out),              //pwm to motor driver board
        .motor_en_out(motor_en_out),                //hard on/off switch
        
        //output to MB
        .count_ready_out(count_ready_out),          //maybe interrupt driven (maybe connected through axi bus)
        .count_high_out(count_high_out)             //measured count of high period from motor encoder (connect to axi bus)

    );
    
    //enable both bridge A and bridge B on motor driver VNH5019
    assign motor_ena_out = motor_en_out;
    assign motor_enb_out = motor_en_out;
    
	// User logic ends

	endmodule
