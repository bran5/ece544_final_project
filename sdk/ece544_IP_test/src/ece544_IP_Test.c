
/**
*
* @file ece544periph_test.c
*
* @author Roy Kravitz (roy.kravitz@pdx.edu)
* @modified by Shrivatsa Yogendra (srivatsa@pdx.edu)
* @modified by Chetan Bornarkar (bchetan@pdx.edu)
* @copyright Portland State University, 2014-2015, 2016-2017
*
* This file implements a test program for the PmodOLEDrgb, PmodENC and Nexys4IO
* custom peripherals. The peripherals provides access to the Nexys4 pushbuttons
* and slide switches, the LEDs, the RGB LEDs, and the Seven Segment display
* on the Digilent Nexys4 board and the PmodOLED and PmodENC connected to the PMOD
* expansion connectors on the Nexys4
*
* The test is basic but covers all of the API functions:
*	o initialize the Nexys4IO driver
*	o Set the LED's to different values
*	o Checks that the duty cycles can be set for both RGB LEDs
*	o Write character codes to the digits of the seven segment display banks
*	o Checks that all of the switches and pushbuttons can be read
*	o Performs a basic test on the rotary encoder and LCD drivers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a	rhk	12/22/14	First release of test program.  Builds on the nx4io_test program.
* 2.00  sy	08/22/16	Modified the code and implemented the functionality of color wheel implementation
* 						and detection of the generated PWM for the RGB LED's
* </pre>
*
* @note
* The minimal hardware configuration for this test is a Microblaze-based system with at least 32KB of memory,
* an instance of Nexys4IO, an instance of the PMod544IOR2,  and an instance of the Xilinx
* UARTLite (used for xil_printf() console output)
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "platform.h"
#include "xparameters.h"
#include "xstatus.h"
#include "nexys4IO.h"
//#include "pmodENC.h"
//#include "xgpio.h"
#include "xintc.h"
#include "xtmrctr.h"
#include "PmodOLEDrgb.h"


/************************** Constant Definitions ****************************/
// Clock frequencies
#define CPU_CLOCK_FREQ_HZ		XPAR_CPU_CORE_CLOCK_FREQ_HZ
#define AXI_CLOCK_FREQ_HZ		XPAR_CPU_M_AXI_DP_FREQ_HZ

// AXI timer parameters
#define AXI_TIMER_DEVICE_ID		XPAR_AXI_TIMER_0_DEVICE_ID
#define AXI_TIMER_BASEADDR		XPAR_AXI_TIMER_0_BASEADDR
#define AXI_TIMER_HIGHADDR		XPAR_AXI_TIMER_0_HIGHADDR
#define TmrCtrNumber			0

// Definitions for peripheral NEXYS4IO
#define NX4IO_DEVICE_ID		XPAR_NEXYS4IO_0_DEVICE_ID
#define NX4IO_BASEADDR		XPAR_NEXYS4IO_0_S00_AXI_BASEADDR
#define NX4IO_HIGHADDR		XPAR_NEXYS4IO_0_S00_AXI_HIGHADDR

// Definitions for peripheral PMODOLEDRGB
#define RGBDSPLY_DEVICE_ID		XPAR_PMODOLEDRGB_0_DEVICE_ID
#define RGBDSPLY_GPIO_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_BASEADDR
#define RGBDSPLY_GPIO_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_HIGHADD
#define RGBDSPLY_SPI_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_BASEADDR
#define RGBDSPLY_SPI_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_HIGHADDR

// Definitions for peripheral PMODENC
#define PMODENC_DEVICE_ID		XPAR_PMODENC_0_DEVICE_ID
//#define PMODENC_BASEADDR		XPAR_PMODENC_0_S00_AXI_BASEADDR
//#define PMODENC_HIGHADDR		XPAR_PMODENC_0_S00_AXI_HIGHADDR

// Fixed Interval timer - 100 MHz input clock, 40KHz output clock
// FIT_COUNT_1MSEC = FIT_CLOCK_FREQ_HZ * .001
#define FIT_IN_CLOCK_FREQ_HZ	CPU_CLOCK_FREQ_HZ
#define FIT_CLOCK_FREQ_HZ		40000
#define FIT_COUNT				(FIT_IN_CLOCK_FREQ_HZ / FIT_CLOCK_FREQ_HZ)
#define FIT_COUNT_1MSEC			40

// GPIO parameters
//#define GPIO_0_DEVICE_ID			XPAR_AXI_GPIO_0_DEVICE_ID
//#define GPIO_0_INPUT_0_CHANNEL		1
//#define GPIO_0_OUTPUT_0_CHANNEL		2


// Interrupt Controller parameters
#define INTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define FIT_INTERRUPT_ID		XPAR_MICROBLAZE_0_AXI_INTC_FIT_TIMER_0_INTERRUPT_INTR

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
unsigned long timeStamp = 0;


/************************** Function Prototypes *****************************/
void usleep(u32 usecs);

void PMDIO_itoa(int32_t value, char *string, int32_t radix);
void PMDIO_puthex(PmodOLEDrgb* InstancePtr, uint32_t num);
void PMDIO_putnum(PmodOLEDrgb* InstancePtr, int32_t num, int32_t radix);
void RunTest1(void);
void RunTest2(void);
void RunTest3(void);
void RunTest4(void);
void RunTest5(void);
void FIT_Handler(void);										//

int do_init_nx4io(u32 BaseAddress);
int do_init_pmdio(u32 BaseAddress);
int AXI_Timer_initialize(void);
int do_init();

//PmodENC 	pmodENC_inst;				// PmodENC instance ref
PmodOLEDrgb	pmodOLEDrgb_inst;			// PmodOLED instance ref
//XGpio		GPIOInst0;					// GPIO instance
XIntc 		IntrptCtlrInst;				// Interrupt Controller instance
XTmrCtr		AXITimerInst;				// PWM timer instance


//XGpio		GPIOInst0,GPIOInst1,GPIOInst2,GPIOInst3;		// GPIO instance

uint8_t 	R,G,B;

// The following variables are shared between non-interrupt processing and
// interrupt processing such that they must be global(and declared volatile)
// These variables are controlled by the FIT timer interrupt handler
// "clkfit" toggles each time the FIT interrupt handler is called so its frequency will
// be 1/2 FIT_CLOCK_FREQ_HZ.  timestamp increments every 1msec and is used in delay_msecs()
volatile unsigned int	clkfit;					// clock signal is bit[0] (rightmost) of gpio 0 output port


volatile u32			gpio_in;				// GPIO input port
volatile u32			high_value_sw_red=0;	// Value of high count from software
volatile u32  			low_value_sw_red=0;		// Value of low count from software
volatile u32			high_value_sw_blue=0;	// Value of high count from software
volatile u32  			low_value_sw_blue=0;	// Value of low count from software
volatile u32			high_value_sw_green=0;	// Value of high count from software
volatile u32  			low_value_sw_green=0;	// Value of low count from software

volatile u32 			count_high_red=0;		// Initializing the count_high value to 0
volatile u32			count_low_red=0;		// Initializing the count_low value to 0
volatile u32			old_pwm_red=0;			// Store the old pwm value to enable detection of rising or falling edge
volatile u32 			count_high_blue=0;		// Initializing the count_high value to 0
volatile u32			count_low_blue=0;		// Initializing the count_low value to 0
volatile u32			old_pwm_blue=0;			// Store the old pwm value to enable detection of rising or falling edge
volatile u32 			count_high_green=0;		// Initializing the count_high value to 0
volatile u32			count_low_green=0;		// Initializing the count_low value to 0
volatile u32			old_pwm_green=0;		// Store the old pwm value to enable detection of rising or falling edge

volatile u32			pwm_red=0;				// Store the current red pwm value
volatile u32			pwm_blue=0;				// Store the current blue pwm value
volatile u32			pwm_green=0;			// Store the current green pwm value
u16 					sw = 0;
u16  					RotaryCnt;
uint64_t 				timestamp = 0L;			// used in delay msec

/************************** MAIN PROGRAM ************************************/
int main()
{
	int sts;
    init_platform();

    sts = do_init();		// initialize the peripherals
    if (XST_SUCCESS != sts)
    {
    	exit(1);
    }

    microblaze_enable_interrupts();		// enable the interrupts

	xil_printf("ECE 544 Nexys4 Peripheral Test Program R2.0\r\n");
	xil_printf("By Dan Hammerstorm.  23-December 2016\r\n\n");


	// TEST 1 - Test the LD15..LD0 on the Nexys4
	RunTest1();
	// TEST 2 - Test RGB1 (LD16) and RGB2 (LD17) on the Nexys4
	RunTest2();
	// TEST 3 - test the seven segment display banks
	RunTest3();
	// TEST 4 - test the rotary encoder (PmodENC) and display (PmodOLED)
	RunTest4();

	// TEST 5 - The main act, at last
	// test the switches and pushbuttons
	// We will do this in a busy-wait loop
	// pressing BTN_C (the center button will
	// cause the loop to terminate
	timeStamp = 0;

	xil_printf("Starting Test 5...the buttons and switch test\r\n");
	xil_printf("Press the center pushbutton to exit\r\n");

	// blank the display digits and turn off the decimal points
	NX410_SSEG_setAllDigits(SSEGLO, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
	NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
	// loop the test until the user presses the center button
	while (1)
	{
		// Run an iteration of the test
		RunTest5();
		// check whether the center button is pressed.  If it is then
		// exit the loop.
		if (NX4IO_isPressed(BTNC))
		{
			// show the timestamp on the seven segment display and quit the loop
			NX4IO_SSEG_putU32Dec((u32) timeStamp, true);
			break;
		}
		else
		{
			// increment the timestamp and delay 100 msecs
			timeStamp += 100;
			usleep(00100000);
		}
	}

	xil_printf("\nThat's All Folks!\n\n");
	// display the message "Bye Bye" on the OLED screen
	OLEDrgb_Clear(&pmodOLEDrgb_inst);	// Clear the display
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 4, 2);	//
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst ,OLEDrgb_BuildRGB(0, 0, 255));  // blue font
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"BYE BYE");
	usleep(05000000);

	// Clear all the display digits and the OLED display at the end of the program
	NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_B, CC_LCY, CC_E, DP_NONE);
	NX410_SSEG_setAllDigits(SSEGLO, CC_B, CC_LCY, CC_E, CC_BLANK, DP_NONE);
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_end(&pmodOLEDrgb_inst);

    cleanup_platform();
    exit(0);
}


/**
 * Function Name: do_init()
 *
 * Return: XST_FAILURE or XST_SUCCESS
 *
 * Description: Initialize the AXI timer, gpio, interrupt, FIT timer, Encoder,
 * 				OLED display
 */
int do_init()
{
	int sts;
	int status;

	// initialize the Nexys4 driver and (some of)the devices
	status = (uint32_t) NX4IO_initialize(NX4IO_BASEADDR);
	if (sts == XST_FAILURE)
	{
		exit(1);
	}

	// initialize the PMod544IO driver and the PmodENC and PmodCLP
//	status = pmodENC_initialize(&pmodENC_inst, PMODENC_BASEADDR);
//	if (sts == XST_FAILURE)
//	{
//		exit(1);
//	}

	// initialize the GPIO instances
//	status = XGpio_Initialize(&GPIOInst0, GPIO_0_DEVICE_ID);
//	if (status != XST_SUCCESS)
//	{
//		return XST_FAILURE;
//	}
	
	// Initialize the AXI Timer 
	status = AXI_Timer_initialize();
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	// set all of the display digits to blanks and turn off
	// the decimal points using the "raw" set functions.
	// These registers are formatted according to the spec
	// and should remain unchanged when written to Nexys4IO...
	// something else to check w/ the debugger when we bring the
	// drivers up for the first time
	NX4IO_SSEG_setSSEG_DATA(SSEGHI, 0x0058E30E);
	NX4IO_SSEG_setSSEG_DATA(SSEGLO, 0x00144116);
	
	// Initialize the OLED display 
	OLEDrgb_begin(&pmodOLEDrgb_inst, RGBDSPLY_GPIO_BASEADDR, RGBDSPLY_SPI_BASEADDR);
	
	// initialize the GPIO instances
//	status = XGpio_Initialize(&GPIOInst0, GPIO_0_DEVICE_ID);
//	if (status != XST_SUCCESS)
//	{
//		return XST_FAILURE;
//	}
	// GPIO0 channel 1 is an 8-bit input port.
	// GPIO0 channel 2 is an 8-bit output port.
//	XGpio_SetDataDirection(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL, 0xFF);
//	XGpio_SetDataDirection(&GPIOInst0, GPIO_0_OUTPUT_0_CHANNEL, 0x00);

	// initialize the interrupt controller
	status = XIntc_Initialize(&IntrptCtlrInst, INTC_DEVICE_ID);
	if (status != XST_SUCCESS)
	{
	   return XST_FAILURE;
	}

	// connect the fixed interval timer (FIT) handler to the interrupt
	status = XIntc_Connect(&IntrptCtlrInst, FIT_INTERRUPT_ID,
						   (XInterruptHandler)FIT_Handler,
						   (void *)0);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;

	}

	// start the interrupt controller such that interrupts are enabled for
	// all devices that cause interrupts.
	status = XIntc_Start(&IntrptCtlrInst, XIN_REAL_MODE);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}

	// enable the FIT interrupt
	XIntc_Enable(&IntrptCtlrInst, FIT_INTERRUPT_ID);
	return XST_SUCCESS;
}

/*
 * AXI timer initializes it to generate out a 4Khz signal, Which is given to the Nexys4IO module as clock input.
 * DO NOT MODIFY
 */
int AXI_Timer_initialize(void){

	uint32_t status;				// status from Xilinx Lib calls
	u32		ctlsts;		// control/status register or mask

	status = XTmrCtr_Initialize(&AXITimerInst,AXI_TIMER_DEVICE_ID);
		if (status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	status = XTmrCtr_SelfTest(&AXITimerInst, TmrCtrNumber);
		if (status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	ctlsts = XTC_CSR_AUTO_RELOAD_MASK | XTC_CSR_EXT_GENERATE_MASK | XTC_CSR_LOAD_MASK |XTC_CSR_DOWN_COUNT_MASK ;
	XTmrCtr_SetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber,ctlsts);

	//Set the value that is loaded into the timer counter and cause it to be loaded into the timer counter
	XTmrCtr_SetLoadReg(AXI_TIMER_BASEADDR, TmrCtrNumber, 24998);
	XTmrCtr_LoadTimerCounterReg(AXI_TIMER_BASEADDR, TmrCtrNumber);
	ctlsts = XTmrCtr_GetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber);
	ctlsts &= (~XTC_CSR_LOAD_MASK);
	XTmrCtr_SetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber, ctlsts);

	ctlsts = XTmrCtr_GetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber);
	ctlsts |= XTC_CSR_ENABLE_TMR_MASK;
	XTmrCtr_SetControlStatusReg(AXI_TIMER_BASEADDR, TmrCtrNumber, ctlsts);

	XTmrCtr_Enable(AXI_TIMER_BASEADDR, TmrCtrNumber);
	return XST_SUCCESS;

}

/************************ TEST FUNCTIONS ************************************/

/****************************************************************************/
/**
* Test 1 - Test the LEDS (LD15..LD0)
*
* Checks the functionality of the LEDs API with some constant patterns and
* a sliding patter.  Determine Pass/Fail by observing the LEDs on the Nexys4
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunTest1(void)
{
	u16 ledvalue;

	xil_printf("Starting Test 1...the LED test\n");
	// test the LEDS (LD15..LD0) with some constant patterns
	NX4IO_setLEDs(0x00005555);
	usleep(02000000);
	NX4IO_setLEDs(0x0000AAAA);
	usleep(02000000);
	NX4IO_setLEDs(0x0000FF00);
	usleep(02000000);
	NX4IO_setLEDs(0x000000FF);
	usleep(02000000);

	// shift a 1 through all of the leds
	ledvalue = 0x0001;
	do
	{
		NX4IO_setLEDs(ledvalue);
		usleep(01000000);
		ledvalue = ledvalue << 1;
	} while (ledvalue != 0);
	return;
}


/****************************************************************************/
/**
* Test 2 - Test the RGB LEDS (LD17..LD16)
*
* Checks the functionality of the RGB LEDs API with a fixed duty cycle.
* Determine Pass/Fail by observing the RGB LEDs on the Nexys4.
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunTest2(void)
{
	// Test the RGB LEDS (LD17..LD16)
	xil_printf("Starting Test 2...the RGB LED test\n");

	// For RGB1 turn on only the blue LED (e.g. Red and Green duty cycles
	// are set to 0 but enable all three PWM channels
	NX4IO_RGBLED_setChnlEn(RGB1, true, true, true);
	NX4IO_RGBLED_setDutyCycle(RGB1, 0, 0, 16);
	usleep(03000000);

	// For RGB2, only write a non-zero duty cycle to the green channel
	NX4IO_RGBLED_setChnlEn(RGB2, true, true, true);
	NX4IO_RGBLED_setDutyCycle(RGB2, 0, 32, 0);
	usleep(03000000);

	// Next make RGB1 red. This time we'll only enable the red PWM channel
	NX4IO_RGBLED_setChnlEn(RGB1, true, false, false);
	NX4IO_RGBLED_setDutyCycle(RGB1, 64, 64, 64);
	usleep(05000000);

	// Next make RGB2 BRIGHTpurple-ish by only changing the duty cycle
	NX4IO_RGBLED_setDutyCycle(RGB2, 255, 255, 255);
	usleep(03000000);

	// Finish by turning the both LEDs off
	// We'll do this by disabling all of the channels without changing
	// the duty cycles
	NX4IO_RGBLED_setDutyCycle(RGB1, 0, 0, 0);
	NX4IO_RGBLED_setDutyCycle(RGB2, 0, 0, 0);

return;
}

/****************************************************************************/
/**
* Test 3 - Test the seven segment display
*
* Checks the seven segment display by displaying DEADBEEF and lighting all of
* the decimal points. Determine Pass/Fail by observing the seven segment
* display on the Nexys4.
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunTest3(void)
{
	xil_printf("Starting Test 3...The seven segment display test\n");

	// show the message 
	NX4IO_SSEG_putU32Hex(0xBEEFDEAD);
	// set the decimal points 
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT7, true);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT6, true);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT5, true);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT4, true);
	usleep(05000000);
	// show the new message 
	NX4IO_SSEG_putU32Hex(0xDEADBEEF);
	// set the remaining decimal points 
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT3, true);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT2, true);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT1, true);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT0, true);
	usleep(05000000);
	// clear all the decimal points 
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT7, false);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT6, false);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT5, false);
	NX4IO_SSEG_setDecPt(SSEGLO, DIGIT4, false);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT3, false);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT2, false);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT1, false);
	NX4IO_SSEG_setDecPt(SSEGHI, DIGIT0, false);
	return;
}


/****************************************************************************/
/**
* Test 4 - Test the PmodENC and PmodCLP
*
* Performs some basic tests on the PmodENC and PmodCLP.  Includes the following tests
* 	1.	check the rotary encoder by displaying the rotary encoder
* 		count in decimal and hex on the LCD display.  Rotate the knob
* 		to change the values up or down.  The pushbuttons can be used
* 		as follows:
* 			o 	press the rotary encoder pushbutton to exit
* 			o 	press BtnUp to clear the count
* 			o 	press BtnR to change rotary encoder
* 				mode to "stop at zero".  This does not appear
* 				to be reversible - not sure why.
* 			o 	press BTNL to change the increment/decrement
* 				value.  Use sw[3:0] to set the new value
*	6.	display the string "357#&CFsw" on the LCD display.  These values
* 		were chosen to check that the bit order is correct.  The screen will
* 		clear in about 5 seconds.
* 	7.	display "Exiting Test 4" on the LCD.  The screen will clear
* 		in about 5 seconds.
*
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void RunTest4(void)
{
	u16 sw;
	int  RotaryIncr;

	bool RotaryNoNeg;
	bool dispSetFlg = false;			// used to reset the display row of the encoder value
	char s[] = " Exiting Test 4 ";

	xil_printf("Starting Test 4...The PmodCLP and PmodENC Test\n");
	xil_printf("Turn PmodENC shaft.  Rotary Encoder count is displayed\n");
	xil_printf("BTNU - clear count, BNTR - Toggle NoNeg flag, BTNL - Inc/Dec count is set to sw[3:0]\n");
	xil_printf("Press Rotary encoder shaft or BTNC to exit\n");

	// test the rotary encoder functions
	RotaryIncr = 1;
	RotaryNoNeg = false;
	
	// Initialize the rotary encoder 
	// clear the counter of the encoder if initialized to garbage value on power on
//	pmodENC_init(&pmodENC_inst, RotaryIncr, RotaryNoNeg);
//	pmodENC_clear_count(&pmodENC_inst);


	// Set up the display output
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst,OLEDrgb_BuildRGB(200, 12, 44));
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 1);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"Enc:");
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 4);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"Hex:");

	while(1)
	{
		// check if the rotary encoder pushbutton or BTNC is pressed
		// exit the loop if either one is pressed.
//		if ( pmodENC_is_button_pressed(&pmodENC_inst) )
//		{
//			break;
//		}

		if (NX4IO_isPressed(BTNC))
		{
			break;
		}

		// check the buttons and perform the appropriate action
		if (NX4IO_isPressed(BTNU))	// clear the rotary count
		{
//			pmodENC_clear_count(&pmodENC_inst);
			dispSetFlg = false;			// clear the row of the display which shows the value of Enc in decimal
		}  // end clear the rotary count
		else if (NX4IO_isPressed(BTNR))		// toggle no-neg flag (may not be reliable)
		{
			if (RotaryNoNeg)	//No Neg was enabled
			{  
//				pmodENC_init(&pmodENC_inst, RotaryIncr, false);
				RotaryNoNeg = false;
			}
			else	// No Neg was disabled
			{  
//				pmodENC_init(&pmodENC_inst, RotaryIncr, true);
				RotaryNoNeg = true;		// enable the no negative mode of the rotary encoder to stop at 0 
			}
		}
		// Load the rotary encoder value from the switches [3:0]
		else if (NX4IO_isPressed(BTNL))
		{ 
			sw = NX4IO_getSwitches();	// load Inc/Dec count from switches
			RotaryIncr = sw & 0xF;
//			pmodENC_init(&pmodENC_inst, RotaryIncr, RotaryNoNeg);
		}

		// read the new value from the rotary encoder and show it on the display 
//		pmodENC_read_count(&pmodENC_inst, &RotaryCnt);
		OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 4, 1);
	
		if((RotaryCnt == 0) && (!dispSetFlg))		// cleared by pressing the UP button
		{// The dispSetFlg is used to avoid the flickering of the display when showing 0 value
			OLEDrgb_PutString(&pmodOLEDrgb_inst, "         ");	// used to clear the row of the display
			dispSetFlg = true;
		}
		OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 4, 1);		// reset the cursor to the location after "Enc:"
		PMDIO_putnum(&pmodOLEDrgb_inst, RotaryCnt, 10);	// show the number in decimal form
		OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 2, 5);
		PMDIO_puthex(&pmodOLEDrgb_inst, RotaryCnt);		// show the number in hex format 

		// display the count on the LEDs and seven segment display, too
		NX4IO_setLEDs(RotaryCnt);
		NX4IO_SSEG_putU32Dec(RotaryCnt, true);
	} // rotary button has been pressed - exit the loop
	xil_printf("\nPmodENC test completed\n");

	// Write some character to the screen to check the ASCII translation
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 7);
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '3');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '5');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '7');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '#');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, '&');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 'C');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 'F');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 's');
	OLEDrgb_PutChar(&pmodOLEDrgb_inst, 'w');
	usleep(05000000);

	// Write one final string
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 4);
	OLEDrgb_PutString(&pmodOLEDrgb_inst, s);	// Display "Exiting Test 4"
	return;
}



/****************************************************************************/
/**
* Test 5 - Test the pushbuttons and switches
*
* Copies the slide switch values to LEDs and the pushbuttons to the decimal
* points in SSEGLO.  Also shows shows the value of the switches on SSEGLO.
* Doing this not only tests the putU16HEX() function but also lets the user
* try all of the character codes (they are displayed on DIGIT7. Determine
* Pass/Fail by flipping switches and pressing
* buttons and seeing if the results are reflected in the LEDs and decimal points
*
* @param	*NONE*
*
* @return	*NONE*
*
* @note
* This function does a single iteration. It should be inclosed in a loop
* if you want to repeat the test
*
*****************************************************************************/
void RunTest5()
{
	u16 ledvalue;
	u8 btnvalue;
	u32 regvalue;
	u32 ssegreg;

	// read the switches and write them to the LEDs and SSEGLO
	ledvalue = NX4IO_getSwitches();
	NX4IO_setLEDs(ledvalue);
	NX4IO_SSEG_putU16Hex(SSEGLO, ledvalue);

	// write sw[4:0] as a character code to digit 7 so we can
	// check that all of the characters are displayed correctly
	NX4IO_SSEG_setDigit(SSEGHI, DIGIT7, (ledvalue & 0x1F));

	// read the buttons and write them to the decimal points on SSEGHI
	// use the raw get and put functions for the seven segment display
	// to test them and to keep the functionality simple.  We want to
	// ignore the center button so mask that out while we're at it.
	btnvalue = NX4IO_getBtns() & 0x01F;
	ssegreg = NX4IO_SSEG_getSSEG_DATA(SSEGHI);

	// shift the button value to bits 27:24 of the SSEG_DATA register
	// these are the bits that light the decimal points.
	regvalue = ssegreg & ~NEXYS4IO_SSEG_DECPTS_MASK;
	regvalue  |=  btnvalue << 24;

	// write the SSEG_DATA register with the new decimal point values
	NX4IO_SSEG_setSSEG_DATA(SSEGHI, regvalue);
	return;
}


/*********************** HELPER FUNCTIONS ***********************************/

/****************************************************************************/
/**
* insert delay (in microseconds) between instructions.
*
* This function should be in libc but it seems to be missing.  This emulation implements
* a delay loop with (really) approximate timing; not perfect but it gets the job done.
*
* @param	usec is the requested delay in microseconds
*
* @return	*NONE*
*
* @note
* This emulation assumes that the microblaze is running @ 100MHz and takes 15 clocks
* per iteration - this is probably totally bogus but it's a start.
*
*****************************************************************************/

static const u32	DELAY_1US_CONSTANT	= 15;	// constant for 1 microsecond delay

void usleep(u32 usec)
{
	volatile u32 i, j;

	for (i = 0; i < usec; i++)
	{
		for (j = 0; j < DELAY_1US_CONSTANT; j++);
	}
	return;
}


/****************************************************************************/
/**
* initialize the Nexys4 LEDs and seven segment display digits
*
* Initializes the NX4IO driver, turns off all of the LEDs and blanks the seven segment display
*
* @param	BaseAddress is the memory mapped address of the start of the Nexys4 registers
*
* @return	XST_SUCCESS if initialization succeeds.  XST_FAILURE otherwise
*
* @note
* The NX4IO_initialize() function calls the NX4IO self-test.  This could
* cause the program to hang if the hardware was not configured properly
*
*****************************************************************************/
int do_init_nx4io(u32 BaseAddress)
{
	int sts;

	// initialize the NX4IO driver
	sts = NX4IO_initialize(BaseAddress);
	if (sts == XST_FAILURE)
		return XST_FAILURE;

	// turn all of the LEDs off using the "raw" set functions
	// functions should mask out the unused bits..something to check w/
	// the debugger when we bring the drivers up for the first time
	NX4IO_setLEDs(0xFFF0000);
	NX4IO_RGBLED_setRGB_DATA(RGB1, 0xFF000000);
	NX4IO_RGBLED_setRGB_DATA(RGB2, 0xFF000000);
	NX4IO_RGBLED_setRGB_CNTRL(RGB1, 0xFFFFFFF0);
	NX4IO_RGBLED_setRGB_CNTRL(RGB2, 0xFFFFFFFC);

	// set all of the display digits to blanks and turn off
	// the decimal points using the "raw" set functions.
	// These registers are formatted according to the spec
	// and should remain unchanged when written to Nexys4IO...
	// something else to check w/ the debugger when we bring the
	// drivers up for the first time
	NX4IO_SSEG_setSSEG_DATA(SSEGHI, 0x0058E30E);
	NX4IO_SSEG_setSSEG_DATA(SSEGLO, 0x00144116);

	return XST_SUCCESS;

}


/*********************** DISPLAY-RELATED FUNCTIONS ***********************************/

/****************************************************************************/
/**
* Converts an integer to ASCII characters
*
* algorithm borrowed from ReactOS system libraries
*
* Converts an integer to ASCII in the specified base.  Assumes string[] is
* long enough to hold the result plus the terminating null
*
* @param 	value is the integer to convert
* @param 	*string is a pointer to a buffer large enough to hold the converted number plus
*  			the terminating null
* @param	radix is the base to use in conversion,
*
* @return  *NONE*
*
* @note
* No size check is done on the return string size.  Make sure you leave room
* for the full string plus the terminating null in string
*****************************************************************************/
void PMDIO_itoa(int32_t value, char *string, int32_t radix)
{
	char tmp[33];
	char *tp = tmp;
	int32_t i;
	uint32_t v;
	int32_t  sign;
	char *sp;

	if (radix > 36 || radix <= 1)
	{
		return;
	}

	sign = ((10 == radix) && (value < 0));
	if (sign)
	{
		v = -value;
	}
	else
	{
		v = (uint32_t) value;
	}

  	while (v || tp == tmp)
  	{
		i = v % radix;
		v = v / radix;
		if (i < 10)
		{
			*tp++ = i+'0';
		}
		else
		{
			*tp++ = i + 'a' - 10;
		}
	}
	sp = string;

	if (sign)
		*sp++ = '-';

	while (tp > tmp)
		*sp++ = *--tp;
	*sp = 0;

  	return;
}


/****************************************************************************/
/**
* Write a 32-bit unsigned hex number to PmodOLEDrgb in Hex
*
* Writes  32-bit unsigned number to the pmodOLEDrgb display starting at the current
* cursor position.
*
* @param num is the number to display as a hex value
*
* @return  *NONE*
*
* @note
* No size checking is done to make sure the string will fit into a single line,
* or the entire display, for that matter.  Watch your string sizes.
*****************************************************************************/
void PMDIO_puthex(PmodOLEDrgb* InstancePtr, uint32_t num)
{
  char  buf[9];
  int32_t   cnt;
  char  *ptr;
  int32_t  digit;

  ptr = buf;
  for (cnt = 7; cnt >= 0; cnt--) {
    digit = (num >> (cnt * 4)) & 0xF;

    if (digit <= 9)
	{
      *ptr++ = (char) ('0' + digit);
	}
    else
	{
      *ptr++ = (char) ('a' - 10 + digit);
	}
  }

  *ptr = (char) 0;
  OLEDrgb_PutString(InstancePtr,buf);

  return;
}


/****************************************************************************/
/**
* Write a 32-bit number in Radix "radix" to LCD display
*
* Writes a 32-bit number to the LCD display starting at the current
* cursor position. "radix" is the base to output the number in.
*
* @param num is the number to display
*
* @param radix is the radix to display number in
*
* @return *NONE*
*
* @note
* No size checking is done to make sure the string will fit into a single line,
* or the entire display, for that matter.  Watch your string sizes.
*****************************************************************************/
void PMDIO_putnum(PmodOLEDrgb* InstancePtr, int32_t num, int32_t radix)
{
  char  buf[16];

  PMDIO_itoa(num, buf, radix);
  OLEDrgb_PutString(InstancePtr,buf);

  return;
}


/**************************** INTERRUPT HANDLERS ******************************/

/****************************************************************************/
/**
* Fixed interval timer interrupt handler
*
* Reads the GPIO port which reads back the hardware generated PWM wave for the RGB Leds
*
* @note
* ECE 544 students - When you implement your software solution for pulse width detection in
* Project 1 this could be a reasonable place to do that processing.
 *****************************************************************************/

void FIT_Handler(void)
{
	// Read the GPIO port to read back the generated PWM signal for RGB led's
//	gpio_in = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);

}


