`timescale 1ns / 1ps
// pwm_gen.v - pwm generator for motor control module
//
// Copyright Venkata Anil Viswanadha & Brandon Biodrowski , Portland State University, 2017
// 
// Date:		20-February-2017
// Version:		1.0
//
// Description:
// ------------
// Generates a PWM signal with a 16 bit count.  Takes in account of PWM Frequency and
// motor control module clock speed to create a counter.  
// 
//////////////////////////////////////////////////////////////////////
module pwm_gen(
    input clk,
    input reset,
    input [31:0] on_time_in,
    output reg pwm_out
    );
	
parameter PWM_PERIOD_FREQ_HZ = 2000;
parameter CLOCK_FREQ_HZ	= 100000000;

reg [31:0] highcount,lowcount;
wire [31:0] off;

assign off = CLOCK_FREQ_HZ/PWM_PERIOD_FREQ_HZ - on_time_in;

always @(posedge clk) begin
	if (reset) 
	begin
		pwm_out <= 1'b0;
		lowcount <= 32'b0;
		highcount <= 32'b0;
	end
	else
	begin
		case(pwm_out)
			1'b1: begin
				lowcount <= 32'b0;
				
				if(highcount <= on_time_in) begin
					pwm_out <= 1'b1;
					highcount <= highcount + 1'b1;			
				end
				else begin
					pwm_out <= 1'b0;
					highcount <= 32'b0;
				end
			end
			1'b0: begin
				highcount <= 32'b0;
				
				if(lowcount <= off) begin
					pwm_out <= 1'b0;
					lowcount <= lowcount + 1'b1;	
				end
				else begin
					pwm_out <= 1'b1;
					lowcount <= 32'b0;
				end
			end
		endcase
	end
		
end

endmodule
