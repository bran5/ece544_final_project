`timescale 1ns / 1ps
// motor_dir_ctrl.v - controls the direction of the motor for use with the VNH5019 Motor Driver
//
// Copyright Venkata Anil Viswanadha & Brandon Biodrowski , Portland State University, 2017
// 
// Date:		20-February-2017
// Version:		1.0
//
// Description:
// ------------
// Controls the direction of a motor driven by the VNH5019 motor driver.  Outputs a high or low 
// on separate channels connected to the INA or INB pins on the driver's board.  The motor_dir_in
// input designates the either high or low for INA or INB 
//////////////////////////////////////////////////////////////////////

module motor_dir_ctrl
(
    input clk,
    input reset,
    input motor_dir_in,             //signal from rotary encoder to indicate direction of motor
    output reg motor_dir_outa,      //assert high for clockwise direction
    output reg motor_dir_outb,      //assert high for counterclockwise direction
    input motor_is_running          //signal to indicate 
);

always@(posedge clk)
begin

    if (reset)
    begin
        motor_dir_outa <= 1'b1;     //begin default state in clockwise motion
        motor_dir_outb <= 1'b0;
    end
    else
    begin
        if (motor_dir_in == 1 && motor_is_running == 0)
			begin
            motor_dir_outa <= 1;
			motor_dir_outb <= 0;
			end
        else if (motor_dir_in == 0 && motor_is_running == 0)
			begin
            motor_dir_outb <= 1;
			motor_dir_outa <= 0;
			end
    end
end    
endmodule