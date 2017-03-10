`timescale 1ns / 1ps
// pwm_detection.v - PWM Detection module
//
// Copyright Brandon Biodrowski, Portland State University, 2016
// 
// Created by : Brandon Biodrowski
// Date:		26-January-2017
// Version:		1.0
//
// Description:
// ------------
// This module implements a state machine to count the high and low times triggered by a 
// rising and falling edge.  The high count and low counts are output. 
// 
// 
//////////////////////////////////////////////////////////////////////
module pwm_detection
(
    input clk,
    input reset,
    input pwm_in,
    output reg [31:0] count_high_pwm,           //outputs the high count of the PWM signal
    output reg [31:0] count_low_pwm,            //outputs the low count of the PWM signal
    output hready_intr,
    output lready_intr
);

wire pwm_rising_edge;
wire pwm_falling_edge;
reg pwm_in_reg;
reg count_hready_reg;
reg count_lready_reg;
reg count_hready;
reg count_lready;

//define state variables
reg [1:0] state;
localparam idle = 2'd0, high = 2'd1, low = 2'd2;

//edge detection of pwm signals 
always@(posedge clk)
begin
    pwm_in_reg <= pwm_in;
    count_hready_reg <= count_hready;
    count_lready_reg <= count_lready;
end

//create rising edge signal 
assign pwm_rising_edge = (~pwm_in_reg) & pwm_in;
assign hready_intr = (~count_hready_reg) & count_hready;
assign lready_intr = (~count_lready_reg) & count_lready;

//create falling edge signal
assign pwm_falling_edge =  (~pwm_in) & pwm_in_reg;

        
//state machine to get the counts for the pwm
//idle state: counts are reset and next state taken depends on falling or rising edge of pwm
//high state: high count incremented until falling edge detected and count low reset
//low state: low count incremented until rising edge detected and count high reset
always@(posedge clk)
begin
    if (reset) 
    begin
        state <= idle;
        count_high_pwm <= 32'b0;
        count_low_pwm <= 32'b0;
    end
    else 
    begin
        case(state)
            idle: begin
                count_high_pwm <= 32'b0;
                count_low_pwm <= 32'b0;
                count_hready <= 1'b0;
                count_lready <= 1'b0;
                if (pwm_rising_edge)
                    state <= high;
                else if (pwm_falling_edge)
                    state <= low;
                else
                    state <= idle;
            end
            high: begin
            
                //increment high count in this state
                count_high_pwm <= count_high_pwm + 1'b1; 
                
                //next state logic
                if (pwm_falling_edge)                   //on a falling edge move to low count state
                begin
                    count_high_pwm <= count_high_pwm; //save count
                    count_hready <= 1'b1;         //bit indicates count is ready
                    count_lready <= 1'b0;
                    state <= low;
                    count_low_pwm <= 32'b0;           //reset count
                end
                else 
                    state <= high;          //remain in this state until see a falling edge
            end
            low: begin
                //increment low count in this state
                count_low_pwm <= count_low_pwm + 1'b1;
                
                //next state logic
                if(pwm_rising_edge)                 //on a rising edge move to high count state
                begin
                    state <= high;
                    count_low_pwm <= count_low_pwm; //save count
                    count_hready <= 1'b0;            //bit indicates count is cleared
                    count_lready <= 1'b1;
                    count_high_pwm <= 32'b0;        //reset count
                end
                else 
                    state <= low;                   //remain in this state until see a falling edge
            end
            default: state <= idle;                 //begin in high count state
        endcase
    end
end
endmodule

        

