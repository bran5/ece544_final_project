`timescale 1ns / 1ps

module pwm_detection_tb();

reg clk_t;
reg red_pwm_t;
reg reset_t;

initial
begin
    clk_t = 1'b0;
    red_pwm_t = 1'b0;
    reset_t = 1'b1;
end

always begin
    #2 clk_t = !clk_t;
end

always begin
    #100 reset_t = 1'b0;
    // #125000 red_pwm_t = !red_pwm_t;
    #31250 red_pwm_t = 1'b1;
    #93750 red_pwm_t = 1'b0;

end
 

    
pwm_detection u1 (
    .clk(clk_t),
    .reset(reset_t),
    .pwm_in(red_pwm_t),
    .count_high_pwm(),
    .count_low_pwm(),
    .hready_intr(),
    .lready_intr()

    );
endmodule