`timescale 1ns / 1ps
// motor_driver_top.v - top level motor driver module for ECE 544 project 2
//
// Copyright Venkata Anil Viswanadha & Brandon Biodrowski , Portland State University, 2017
// 
// Date:		20-February-2017
// Version:		1.0
//
// Description:
// ------------
// Instantiates the pwm_gen.v, motor_dir_ctrl.v, and encoder_detection.v modules to drive a dc motor with 
// the VNH5019 motor driver.  
// 
//////////////////////////////////////////////////////////////////////


module motor_driver_top
(
    input clk,
    input reset,

    input motor_encoder_in,               //input from motor encoder
    input [31:0] on_set_point_in,         //desired speed of the motor 
    
    input motor_dir_in,                   //signal from Microblaze to indicate motor direction
    input motor_sw,                       //manual enable switch from FPGA board to ENA and ENB on motor driver board
	
    //motor control interface
    output  motor_dir_outa,              //assert high for clockwise direction, deassert when motor_dir_outb is high
    output  motor_dir_outb,              //assert high for counterclockwise direction, deassert when motor_dir_outa is high 
    output  motor_pwm_out,               //PWM output to motor driver
    output  motor_en_out,                //enable motor driver connected to ENA and ENB
    output  count_ready_out,             //signal to tell processor that count is ready to be used as calculation
    output  [31:0] count_high_out        //measured output from the encoder detection module
);

//set the period of PWM and clock used in motor control module
parameter PWM_PERIOD_FREQ_HZ = 10000;
parameter CLOCK_FREQ_HZ	= 100000000;


assign motor_en_out = motor_sw;  //enable motor for now
wire    motor_is_running;


//instantiate encoder detection block
encoder_detection   #(
	.PWM_PERIOD_FREQ_HZ(PWM_PERIOD_FREQ_HZ),
	.CLOCK_FREQ_HZ(CLOCK_FREQ_HZ))
    
    encoder_detection_0  (
    .clk(clk),
    .reset(reset),
    .motor_encoder_in(motor_encoder_in),               //*****need to change to motor_encoder_in
    .count_high(count_high_out),                //counts output to the processor  
    .count_low(),                               //not used since we know the pwm frequency
    .motor_is_running(motor_is_running),        //signal to indicate motor is running     
    .count_ready(count_ready_out)               //signal to indicate high count is ready
);

motor_dir_ctrl motor_dir_ctrl_0
(
    .clk(clk),
    .reset(reset),
    .motor_dir_in(motor_dir_in),
    .motor_dir_outa(motor_dir_outa),
    .motor_dir_outb(motor_dir_outb),
    .motor_is_running(motor_is_running)
);

pwm_gen  #(
	.PWM_PERIOD_FREQ_HZ(PWM_PERIOD_FREQ_HZ),
	.CLOCK_FREQ_HZ(CLOCK_FREQ_HZ))
	
	pwm_gen0 (
    .clk(clk),
    .reset(reset),
    .on_time_in(on_set_point_in),                             //on_set_point_in from the MB processor, from control loop
    .pwm_out(motor_pwm_out)
    );

endmodule