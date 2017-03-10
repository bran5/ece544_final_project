

/***************************** Include Files *******************************/
#include "pwm_detection.h"
#include "xil_io.h"
/************************** Function Definitions ***************************/


/****************************************************************************/
/**
*Read the high count of the PWM signal 
*
*
* @param	- NONE
*
* @return
* 		- returns the high count of the PWM signal 
*
* @note		The Base Address of the NEXYS4IO peripheral will be in xparameters.h
*
*****************************************************************************/
uint32t _PWMdet_rd_hcount PWMdet_rd_hcount(void) {
    
    uinit32_t high_count;
    
	//read the high count of the PWM detection IP
	high_count = PWM_DETECTION_mReadReg(XPAR_PWM_DETECTION_0_S00_AXI_BASEADDR, PWM_DET_HCOUNT_REG);

    return high_count;

}

/****************************************************************************/
/**
*Read the low count of the PWM signal 
*
*
* @param	- NONE
*
* @return
* 		- returns the low count of the PWM signal 
*
* @note		The Base Address of the NEXYS4IO peripheral will be in xparameters.h
*
*****************************************************************************/
uint32t _PWMdet_rd_lcount PWMdet_rd_hcount(void) {
    
    uinit32_t low_count;
    
	//read the high count of the PWM detection IP
	low_count = PWM_DETECTION_mReadReg(XPAR_PWM_DETECTION_0_S00_AXI_BASEADDR, PWM_DET_LCOUNT_REG);

    return low_count;

}