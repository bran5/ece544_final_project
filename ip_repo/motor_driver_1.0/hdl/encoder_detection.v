`timescale 1ns / 1ps
// encoder_detection.v - module to measure 1 channel of an Hall effect encoder
//
// Copyright Venkata Anil Viswanadha & Brandon Biodrowski , Portland State University, 2017
// 
// Date:		20-February-2017
// Version:		1.0
//
// Description:
// ------------
// This module takes in an encoder input for a motor and measures the high and low period of the signal.
// With the known frequency of the PWM signal we can use the high count to calculate the duty cycle. 
// 
//////////////////////////////////////////////////////////////////////

module encoder_detection
(
    input clk,
    input reset,
    input motor_encoder_in,                     //signal from the motor encoder
    output reg [31:0] count_high,           //high count output to the controller
    output reg [31:0] count_low,
    output reg  motor_is_running,           //signal to indicate motor is running
    output reg  count_ready                 //indicates high count is ready
);


parameter PWM_PERIOD_FREQ_HZ = 2000;
parameter CLOCK_FREQ_HZ	= 100000000;

wire [31:0] max_count; 
wire rising_edge;
wire falling_edge;
reg motor_encoder_in_reg;

assign max_count = CLOCK_FREQ_HZ/PWM_PERIOD_FREQ_HZ;        //maximum count of the encoder based on the module clock and pwm frequency

//define state variables
reg [1:0] state;
localparam idle = 2'd0, high = 2'd1, low = 2'd2;

//edge detection of encoder signal through synchronizer
always@(posedge clk)
begin
    motor_encoder_in_reg <= motor_encoder_in;
end

//create rising edge signal 
assign rising_edge = (~motor_encoder_in_reg) & motor_encoder_in;

//create falling edge signal
assign falling_edge =  (~motor_encoder_in) & motor_encoder_in_reg;

        
//state machine to get the counts for the motor encoder
//idle state: counts are reset and next state taken depends on falling or rising edge of motor encoder
//high state: high count incremented until falling edge detected and count low reset
//low state: low count incremented until rising edge detected and count high reset
always@(posedge clk)
begin
    if (reset) 
    begin
        state <= idle;
        count_high <= 32'b0;
        count_low <= 32'b0;
    end
    else 
    begin
        case(state)
            idle: begin
                count_high <= 32'b0;
                count_low <= 32'b0;
                motor_is_running <= 0;
                count_ready <= 0;
                if (rising_edge)
                    state <= high;
                else if (falling_edge)
                    state <= low;
                else
                    state <= idle;
            end
            high: begin
            
                //increment high count in this state
                count_high <= count_high + 1'b1; 
                
                //set signal to tell us that motor is running
                motor_is_running <= 1;

                //next state logic
                if (falling_edge)                   //on a falling edge move to low count state
                begin
                    count_high <= count_high; //save count
                    count_ready <= 1;
                    state <= low;
                    count_low <= 32'b0;           //reset count
                end
                else 
                    state <= high;          //remain in this state until see a falling edge
                    
            end
            low: begin
   
                //determine if motor is idle
                if (count_low > max_count)
                    begin
                    state <= idle;
                    end
                else
                    begin
                    //increment low count in this state
                    count_low <= count_low + 1'b1;
                    
                    //set signal to tell us motor is running
                    motor_is_running <= 1;
      
                    //next state logic
                    if(rising_edge)                 //on a rising edge move to high count state
                    begin
                        state <= high;
                        count_low <= count_low;     //save count
                        count_high <= 32'b0;        //reset count
                        count_ready <= 0;
                    end
                    else 
                        state <= low;                   //remain in this state until see a falling edge
                    end
            end
            default: state <= idle;                 //begin in high count state
        endcase
    end
end
endmodule

        

