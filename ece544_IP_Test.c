
/**
*
* @file ece544periph_test.c
*
* @author Roy Kravitz (roy.kravitz@pdx.edu)
* @modified by Shrivatsa Yogendra (srivatsa@pdx.edu)
* @modified by Chetan Bornarkar (bchetan@pdx.edu)
* @modified by Randon Stasney (rstasney@pdx.edu)
* @copyright Portland State University, 2014-2015, 2016-2017
*
* This file implements a pulse width modulation program which demonstrates the 
* functionality of the PmodOLEDrgb, PmodENC and Nexys4IO custom peripherals.
* The Nexys4IO provides access to the Nexys4 pushbuttons and slide switches, 
* the LEDs, the RGB LEDs, and the Seven Segment display on the Digilent Nexys4
* board and the PmodOLED provides a method for display and PmodENC provides a 
* means for changing the Hue of the created color.
*
* The new program covers much of the API functions:
*	o initialize the Nexys4IO driver
*	o Set the SSEG (seven segment display) to display the detected 
*		PWM (pulse width modulation) for the RGB values
*	o (SW) switch 0 changes whether the display is from the software 
*		detection or the hardware detection
*	o The switch LED on SW 0 is a visual representation of detection method
*		with lit or 1 being the hardware
*	o The on board RGB LEDs represent the color with RGB2 (LD17) taking the 
*		built in function HSV to RGB function which gives it's final value
*		as a R(5) G(6) B(5) bit value. RGB1 (LD16) recieves it's values from
*		the MIT hsvtorgb function which translates the 8 bit values from 
*		hue saturation and value to full 8 bit RGB values.  This signal is 
*		then what is detected with the pulse width detection
*	o The left and right buttons change the "Saturation" in a 0-255 range
*	o The up and down buttons change the "Value" in a 0-255 range
*	o The rotary encoder selects the "Hue" in a 0-255 range
*	o The splash screen is exited by the center button or the rotary button
*
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a	rhk	12/22/14	First release of test program.  Builds on the nx4io_test program.
* 2.00  sy	08/22/16	Modified the code and implemented the functionality of color wheel implementation
* 						and detection of the generated PWM for the RGB LED's
* 3.00	rs	02/01/17	Modified to detect pulse width modulation in SW and display HW results
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
#include "xuartlite.h"
#include "xsysmon.h"
#include "IPhb3.h"
#include "xgpio.h"
#include "xintc.h"
#include "xtmrctr.h"
#include "PmodOLEDrgb.h"
#include "bitmap.h"
#include "PmodBT2.h"

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

// Definitions definitions for peripheral AXI_UARTLITE
#define UARTLITE_1_DEVICE_ID  	XPAR_UARTLITE_1_DEVICE_ID 
#define UARTLITE_1_BASEADDR		XPAR_UARTLITE_1_BASEADDR 


// Definitions for peripheral PMODOLEDRGB
#define RGBDSPLY_DEVICE_ID		XPAR_PMODOLEDRGB_0_DEVICE_ID
#define RGBDSPLY_GPIO_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_BASEADDR
#define RGBDSPLY_GPIO_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_HIGHADD
#define RGBDSPLY_SPI_BASEADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_BASEADDR
#define RGBDSPLY_SPI_HIGHADDR	XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_HIGHADDR

// Definitions for peripheral PMODBT2
#define PMODBT2_DEVICE_ID 	XPAR_PMODBT2_0_DEVICE_ID
#define PMODBT2_UART_BASEADDR				XPAR_PMODBT2_0_AXI_LITE_UART_BASEADDR 
#define PMODBT2_UART_HIGHADDR				XPAR_PMODBT2_0_AXI_LITE_UART_HIGHADDR 
#define PMODBT2_GPIO_BASEADDR				XPAR_PMODBT2_0_AXI_LITE_GPIO_BASEADDR 
#define PMODBT2_GPIO_HIGHADDR				XPAR_PMODBT2_0_AXI_LITE_GPIO_HIGHADDR 


// Definitions for peripheral IPHB3_0 
#define IPHB3_DEVICE_ID			XPAR_IPHB3_0_DEVICE_ID 
#define IPHB3_BASEADDR 			XPAR_IPHB3_0_S00_AXI_BASEADDR
#define IPHB3_HIGHADDR 			XPAR_IPHB3_0_S00_AXI_HIGHADDR 
// Definitions for peripheral IPHB3_1
#define IPHB3_DEVICE_ID			XPAR_IPHB3_1_DEVICE_ID
#define IPHB3_BASEADDR 			XPAR_IPHB3_1_S00_AXI_BASEADDR
#define IPHB3_HIGHADDR 			XPAR_IPHB3_1_S00_AXI_HIGHADDR

// Fixed Interval timer - 100 MHz input clock, 5KHz output clock
#define FIT_IN_CLOCK_FREQ_HZ	CPU_CLOCK_FREQ_HZ
#define FIT_CLOCK_FREQ_HZ		5000
#define FIT_COUNT				(FIT_IN_CLOCK_FREQ_HZ / FIT_CLOCK_FREQ_HZ)
#define FIT_COUNT_1MSEC			5

// GPIO parameters
#define GPIO_0_DEVICE_ID			XPAR_AXI_GPIO_0_DEVICE_ID
#define GPIO_0_INPUT_0_CHANNEL		1
#define GPIO_0_OUTPUT_0_CHANNEL		2

// adc
#define SYSMON_DEVICE_ID 	XPAR_SYSMON_0_DEVICE_ID
// Interrupt Controller parameters
#define INTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
#define FIT_INTERRUPT_ID		XPAR_MICROBLAZE_0_AXI_INTC_FIT_TIMER_0_INTERRUPT_INTR


// Mask for HB3
#define IPHB3_SA_MASK				0x0000FFFF  
#define IPHB3_SB_MASK				0xFFFF0000
#define IPHB3_DC_MASK				0x000000FF
#define IPHB3_EN_MASK 				0x00000100
#define IPHB3_DIR_MASK				0x00000200
#define IPHB3_MOT_MASK				0x00000100
#define IPHB3_SWSA_MASK				0x00000001  
#define IPHB3_SWSB_MASK				0x00000002
#define XOR_MASK					0xFFFFFFFF

// Mask for switch[0]
#define SW_0 						0x00000001
#define SW_1						0x00000002
#define SW_2						0x00000004
#define SW_3						0x00000008
//#define
/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/


/************************** Function Prototypes *****************************/
void usleep(u32 usecs);

void PMDIO_itoa(int32_t value, char *string, int32_t radix);
void PMDIO_puthex(PmodOLEDrgb* InstancePtr, uint32_t num);
void PMDIO_putnum(PmodOLEDrgb* InstancePtr, int32_t num, int32_t radix);

void FIT_Handler(void);										
int do_init_nx4io(u32 BaseAddress);
int do_init_pmdio(u32 BaseAddress);
int AXI_Timer_initialize(void);
int do_init();
static int SysMonFractionToInt(float FloatNum);

XUartLite	UART_inst, MAX_inst;
PmodOLEDrgb	pmodOLEDrgb_inst;			// PmodOLED instance ref
PmodBT2		myDevice;
XGpio		GPIOInst0;					// GPIO instance
XIntc 		IntrptCtlrInst;				// Interrupt Controller instance
XTmrCtr		AXITimerInst;				// PWM timer instance
static XSysMon SysMonInst;  




// storage for conversion from hex into bcd for display to SSEG
unsigned char rpm_SA[10];
unsigned char KP_disp[10];
unsigned char DATAPTR[10];


// The following variables are shared between non-interrupt processing and
// interrupt processing such that they must be global(and declared volatile)
// These variables are controlled by the FIT timer interrupt handler

XSysMon_Config *ConfigPtr;
u32 aux3Raw;
float aux3Data;
XSysMon *SysMonInstPtr = &SysMonInst;

volatile u32			HB_rpm;				// GPIO input port
volatile u32			motor_out;				// GPIO input port
volatile u32			HB_SW;				// GPIO input port
volatile u32			gpio_in;				// GPIO input port

volatile u32			SA_count = 0;
volatile u32			SA_freq = 20;

u16 					sw = 0;					// storage to read in the switches

volatile u32			MOT_CONR = 0;
volatile u32			MOT_CONL = 0;

volatile int			MRPM = 0;
volatile u32			MOT_CNTRL = 0;

volatile uint8_t		Screen = 0;
volatile uint8_t        Flag = 0;
volatile uint8_t		DirSafe = 0;
volatile uint8_t		sensor = 0;

/************************** MAIN PROGRAM ************************************/
int main()
{
	int sts;
	xil_printf("inprogram\r\n");
    init_platform();

    sts = do_init();		// initialize the peripherals
  //  if (XST_SUCCESS != sts)
   // {
   // 	exit(1);
   // }

 //   microblaze_enable_interrupts();		// enable the interrupts

	xil_printf("ECE 544 PWMDet\r\n");
	xil_printf("By Randon Stasney.  01-February 2017\r\n\n");

	while (1) {
	// Detect PWM duty cycle and display
	PWMdetection();

	// blank the display digits and turn off the decimal points
	NX410_SSEG_setAllDigits(SSEGLO, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);
	NX410_SSEG_setAllDigits(SSEGHI, CC_BLANK, CC_BLANK, CC_BLANK, CC_BLANK, DP_NONE);

	// Clear all the display digits and the OLED display at the end of the program
	OLEDrgb_Clear(&pmodOLEDrgb_inst);
//	OLEDrgb_end(&pmodOLEDrgb_inst);
	IPHB3_mWriteReg(IPHB3_BASEADDR, IPHB3_S00_AXI_SLV_REG0_OFFSET, 0x00000000);
//    cleanup_platform();
	}
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
	//int sts;
	int status;

	// initialize the Nexys4 driver and (some of)the devices
	status = (uint32_t) NX4IO_initialize(NX4IO_BASEADDR);
	if (status == XST_FAILURE)
	{
		exit(1);
	}
	
	xil_printf("Starting BT2 Init...\r\n");
	BT2_begin(&myDevice, XPAR_PMODBT2_0_AXI_LITE_GPIO_BASEADDR, XPAR_PMODBT2_0_AXI_LITE_UART_BASEADDR);
	BT2_changeBaud(&myDevice, 9600);


	//status = XUartLite_Initialize(&UART_inst, XPAR_AXI_UARTLITE_1_DEVICE_ID);
	//if (status != XST_SUCCESS)
	//{
	//	return XST_FAILURE;
	//}

	
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
	status = XGpio_Initialize(&GPIOInst0, GPIO_0_DEVICE_ID);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	// GPIO0 channel 1 is an 32-bit input port.
	// GPIO0 channel 2 is an 32-bit output port.
	XGpio_SetDataDirection(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL, 0xFFFFFFFF);
	XGpio_SetDataDirection(&GPIOInst0, GPIO_0_OUTPUT_0_CHANNEL, 0x00000000);

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
	xil_printf("Start Interupts Init\r\n");
	// start the interrupt controller such that interrupts are enabled for
	// all devices that cause interrupts.
	status = XIntc_Start(&IntrptCtlrInst, XIN_REAL_MODE);
	if (status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	xil_printf("1\r\n");
	// enable the FIT interrupt
	//XIntc_Enable(&IntrptCtlrInst, FIT_INTERRUPT_ID);
//	return XST_SUCCESS;

	xil_printf("2\r\n");
	/*
	 * Initialize the SysMon driver.
	 */
	ConfigPtr = XSysMon_LookupConfig(SYSMON_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XSysMon_CfgInitialize(SysMonInstPtr, ConfigPtr,
				ConfigPtr->BaseAddress);
	xil_printf("3\r\n");
	/*
	 * Self Test the System Monitor/ADC device
	 */
	status = XSysMon_SelfTest(SysMonInstPtr);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("4\r\n");
	/*
	 * Set the ADCCLK frequency equal to 1/32 of System clock for the System
	 * Monitor/ADC in the Configuration Register 2.
	 */
	XSysMon_SetAdcClkDivisor(SysMonInstPtr, 32);
	xil_printf("5\r\n");
	/*
	 * Wait till the End of Sequence occurs
	 */
	XSysMon_GetStatus(SysMonInstPtr); /* Clear the old status */
		xil_printf("done xmos\r\n");
	
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



/****************************************************************************/
/**
* PWMdetection - Test to detect PWM and test board interface through MicroBlaze
*
* Performs the functions required by the assignment
*			o 	press the rotary encoder pushbutton or center button to start
* 			o   the rotary encoder changes the Hue
* 			o 	press BtnUp and BtnD change value
* 			o 	press BtnR and BtnL change value
* 			o 	sw[0] determines whether hardware or software PWM dettection shown
*			o   duty cycle displayed on seven segement display left to right 
*			o  	Red Green Blue
*			o 	press the rotary encoder pushbutton or center button to exit the program
*
*
* @param	*NONE*
*
* @return	*NONE*
*
*****************************************************************************/
void PWMdetection(void)
{	
	int i;
	
	u32 hard_soft;						// hold value fro HW or SW detection desired
	u32 rot_inc;
	u32	KD_KI;
	u32	KP_or;
	u32 Old_SW;
	char s[] = "Exiting Test 4";
	int str_size = 15;

	int rest = 0;
//	char bit;
//	bool work;
	int len;
	//	char s[] = "Exiting Test 4";
	//int i;
	char output = "\n";


	xil_printf("Starting PWMdetection\n");
	xil_printf("Turn PmodENC shaft to change hue\n");
	xil_printf("BTNU - increase value, BNTD - decrease value\n");
	xil_printf("BTNR - increase saturation, BNTL - decrease saturation\n");
	xil_printf("Press Rotary encoder shaft or BTNC to exit\n");



	NX4IO_setLEDs(0x00000000);
	NX4IO_RGBLED_setChnlEn(RGB1, true, true, true);
	NX4IO_RGBLED_setChnlEn(RGB2, true, true, true);	
	NX4IO_RGBLED_setDutyCycle(RGB1, 0, 0, 0);
	NX4IO_RGBLED_setDutyCycle(RGB2, 0, 0, 0);
	
	IPHB3_mWriteReg(IPHB3_BASEADDR, IPHB3_S00_AXI_SLV_REG0_OFFSET, 0x00000000);

	// display Amelie splash screen for intro before detection
	OLEDrgb_DrawBitmap(&pmodOLEDrgb_inst,0,0,95,63, (u8*)tommy);
	usleep(0100000);//Wait 1 seconds
	while(1)
	{
		sw = NX4IO_getSwitches();							// get switch info
		hard_soft = (sw & SW_0);							// mask to just get sw 0
		
		if (hard_soft){
			break;
		}
	}


	// Set up the display output
	OLEDrgb_Clear(&pmodOLEDrgb_inst);


	while(1)
	{
		// check if the rotary encoder pushbutton or BTNC is pressed
		// exit the loop if either one is pressed.

		if (NX4IO_isPressed(BTNC))
		{
			break;
		}
		//	xil_printf("try stuff Init\r\n");

	while ((XSysMon_GetStatus(SysMonInstPtr) & XSM_SR_EOS_MASK) !=
			XSM_SR_EOS_MASK);
				
		//xil_printf("Smade past\r\n");
	aux3Raw = XSysMon_GetAdcData(SysMonInstPtr, XSM_CH_AUX_MIN+3);
	aux3Data = XSysMon_RawToVoltage(aux3Raw);
	if (aux3Data < .15){
		sensor = 1;
	//xil_printf("The aux3Data is %0d.%03d Volts. \r\n\r\n",
	//			(int)(aux3Data), SysMonFractionToInt(aux3Data));
	}
	else {
		sensor = 0;

	}
	//xil_printf("Exiting the SysMon Polled Example. \r\n");
	
	
		if((len=BT2_getData(&myDevice, 600))){
			//echo back to sender
		output = myDevice.recv[0];}
			switch (output){
				case '0'	:
				if (sensor){
				MOT_CONR = 0;
					MOT_CONL = 0;
					DirSafe = 1;
				}
				else{	
				switch (gpio_in){
					xil_printf("update gpio %d\r\n",gpio_in);
					case 0 :  
					MOT_CONR = 0;
					MOT_CONL = 0;
					DirSafe = 1;
					break;
					case 1 :	// turn right 	
					MOT_CONR = 0;
					MOT_CONL = 0x00000164;
					DirSafe = 0;
					break;
					case 2 :	// turn left 	
					MOT_CONR = 0x00000164;
					MOT_CONL = 0;
					DirSafe = 0;
					break;
					case 3 :	// slow 	
					MOT_CONR = 0x00000164;
					MOT_CONL = 0x00000164;
					DirSafe = 0;
					break;
					case 4 :	// fast	
					MOT_CONR = 0x000001F0;
					MOT_CONL = 0x000001F0;
					DirSafe = 0;
					break;
					case 5 :	// stop	
					MOT_CONR = 0x0;
					MOT_CONL = 0x0;	
					DirSafe = 1;
					break;
					case 6 :	// reverse	
					if (DirSafe){
					IPHB3_mWriteReg(XPAR_IPHB3_0_S00_AXI_BASEADDR, IPHB3_S00_AXI_SLV_REG3_OFFSET, 0x00000001);
					IPHB3_mWriteReg(XPAR_IPHB3_1_S00_AXI_BASEADDR, IPHB3_S00_AXI_SLV_REG3_OFFSET, 0x00000001);					
					MOT_CONR = 0x0;
					MOT_CONL = 0x0;	
					DirSafe = 1;
					}
					else {
					MOT_CONR = MOT_CONR;
					MOT_CONL = MOT_CONL;
					DirSafe = DirSafe;						
					}
					break;
					default :
					MOT_CONR = 0;
					MOT_CONL = 0;
					DirSafe = 1;
					break;
					
			//	}
				}
				break;
					case '1' :	// turn right 	
					MOT_CONR = 0;
					MOT_CONL = 0x00000164;
					DirSafe = 0;
					xil_printf("in case1\r\n");
					break;
					case '2' :	// turn left 	
					MOT_CONR = 0x00000164;
					MOT_CONL = 0;
					DirSafe = 0;
					break;
					case '3' :	// slow 	
					MOT_CONR = 0x00000164;
					MOT_CONL = 0x00000164;
					DirSafe = 0;
					break;
					case '4' :	// fast	
					MOT_CONR = 0x000001F0;
					MOT_CONL = 0x000001F0;
					DirSafe = 0;
					break;
					case '5' :	// stop	
					MOT_CONR = 0x0;
					MOT_CONL = 0x0;	
					DirSafe = 1;
					break;
					case '6' :	// reverse	
					if (DirSafe){
					IPHB3_mWriteReg(XPAR_IPHB3_0_S00_AXI_BASEADDR, IPHB3_S00_AXI_SLV_REG3_OFFSET, 0x00000001);
					IPHB3_mWriteReg(XPAR_IPHB3_1_S00_AXI_BASEADDR, IPHB3_S00_AXI_SLV_REG3_OFFSET, 0x00000001);	
					MOT_CONR = 0x0;
					MOT_CONL = 0x0;	
					DirSafe = 1;
					}
					else {
					MOT_CONR = MOT_CONR;
					MOT_CONL = MOT_CONL;
					DirSafe = DirSafe;						
					}
					break;
					default :
					MOT_CONR = MOT_CONR;
					MOT_CONL = MOT_CONL;
					DirSafe = DirSafe;

				
			}
		}



		IPHB3_mWriteReg(XPAR_IPHB3_0_S00_AXI_BASEADDR, IPHB3_S00_AXI_SLV_REG0_OFFSET, MOT_CONR);
		IPHB3_mWriteReg(XPAR_IPHB3_1_S00_AXI_BASEADDR, IPHB3_S00_AXI_SLV_REG0_OFFSET, MOT_CONL);
				
	
	//	XUartLite_Recv(&UART_inst, &DATAPTR,
	//			2);
		

//	SA_freq = SA_freq - 1;
//	if (SA_freq == 0){
//		SA_freq = 20;
	gpio_in = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);
		//xil_printf("update gpio %d\r\n",gpio_in);
//	}

	//	if (Flag == 1){
			
	//		Flag = 0;
	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst,OLEDrgb_BuildRGB(255, 0,0));
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 3);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"volt");
	PMDIO_putnum(&pmodOLEDrgb_inst, sensor, 10);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"  ");
	

	OLEDrgb_SetFontColor(&pmodOLEDrgb_inst,OLEDrgb_BuildRGB(0, 255,100));
	OLEDrgb_SetCursor(&pmodOLEDrgb_inst, 0, 7);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"MOT");
	PMDIO_putnum(&pmodOLEDrgb_inst, gpio_in, 10);
	OLEDrgb_PutString(&pmodOLEDrgb_inst,"  ");
	//	}
	} // rotary button has been pressed - exit the loop
	NX4IO_RGBLED_setDutyCycle(RGB1, 0, 0, 0);
	NX4IO_RGBLED_setDutyCycle(RGB2, 0, 0, 0);
	xil_printf("\nPWMdet test completed\n");

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
* Steps through edge detection to find a high and low count to determine the duty cycle
*
* @note
* software solution for pulse width detection 
 *****************************************************************************/

void FIT_Handler(void)
{
		
	//gpio_in = XGpio_DiscreteRead(&GPIOInst0, GPIO_0_INPUT_0_CHANNEL);	
	//SA_freq = SA_freq - 1;
//	if (SA_freq == 0){
	//	SA_freq = 1249;
	//		xil_printf("infit\r\n");
	//	Flag = 1;
//	}	
}

int SysMonFractionToInt(float FloatNum)
{
	float Temp;

	Temp = FloatNum;
	if (FloatNum < 0) {
		Temp = -(FloatNum);
	}

	return( ((int)((Temp -(float)((int)Temp)) * (1000.0f))));
}

