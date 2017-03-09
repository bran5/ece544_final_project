

/***************************** Include Files *******************************/
#include "motor_driver.h"
#include "xil_io.h"
/************************** Function Definitions ***************************/



/****************************************************************************/
/**
* Read motor encoder
*
*Reads the high count of the motor encoder defined
*
* @return
* 		- encoder_cnt	returns the count of the motor encoder
*
* @note		The Base Address of the MOTOR_DRIVER peripheral will be in xparameters.h
*
*****************************************************************************/
uint32_t motor_driver_readEnc(void) {

	uint32_t encoder_cnt;

	encoder_cnt = MOTOR_DRIVER_mReadReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_ENC_CNT_REG);

	return encoder_cnt;
}

/****************************************************************************/
/**
* Read motor encoder ready signal
*
*Reads the count ready signal so we know when the count is finished
*
* @return
* 		- encoder_cnt_rdy	returns the ready signal for the count high
*
* @note		The Base Address of the MOTOR_DRIVER peripheral will be in xparameters.h
*
*****************************************************************************/
uint32_t motor_driver_readEnc_cnt_rdy(void){
    
    uint32_t encoder_cnt_rdy;

	encoder_cnt_rdy = MOTOR_DRIVER_mReadReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_ENC_CNT_RDY_REG);

	return encoder_cnt_rdy;
    
}

/****************************************************************************/
/**
*Set the PWM high count of the motor driver and the direction of the motor (clockwise or counter clockwise)
*
*Checks to see if the direction has changed and if so, stops the motor first then waits for motor to stop
*then changes direction and resumes speed
*
* @param	high_set_point is the count of the on time of the PWM signal
*
* @param	direction indicates clockwise of counter clockwise direction of the motor
*
* @return
* 		- NONE
*
* @note		The Base Address of the NEXYS4IO peripheral will be in xparameters.h
*
*****************************************************************************/
enum _MOTOR_DRIVER_dir motor_driver_setPWM_setDir(uint32_t high_set_point, enum _MOTOR_DRIVER_dir direction, enum _MOTOR_DRIVER_dir old_dir ) {

	uint32_t encoder_cnt;

	//check to see if the motor direction changed
	if (direction != old_dir){

		//if yes then stop motor
		MOTOR_DRIVER_mWriteReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_ON_SET_POINT_REG, 0x0000);

		encoder_cnt = motor_driver_readEnc();

		//wait for motor to stop
		while(encoder_cnt != 0x0000) {
			encoder_cnt = motor_driver_readEnc();
		}

		//change direction of motor
		switch(direction){
		case(clkwise):
				MOTOR_DRIVER_mWriteReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_DIRECTION_REG, clkwise);
				break;
		case(cntr_clkwise):
				MOTOR_DRIVER_mWriteReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_DIRECTION_REG, cntr_clkwise);
				break;
		default:
				MOTOR_DRIVER_mWriteReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_DIRECTION_REG, cntr_clkwise);
				break;
		}

		//save direction of motor to compare with next time
		old_dir = direction;
	}

	//set the speed of the motor
	MOTOR_DRIVER_mWriteReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_ON_SET_POINT_REG, high_set_point);

	//return and save old direction to compare with next time
	return old_dir;

}


/****************************************************************************/
/**
*Set the PWM high count of the motor driver 
*
*
* @param	high_set_point is the count of the on time of the PWM signal
*
* @return
* 		- NONE
*
* @note		The Base Address of the NEXYS4IO peripheral will be in xparameters.h
*
*****************************************************************************/
void _MOTOR_DRIVER_dir motor_driver_setPWM(uint32_t high_set_point) {


	//set the speed of the motor
	MOTOR_DRIVER_mWriteReg(XPAR_MOTOR_DRIVER_0_S00_AXI_BASEADDR, MOTOR_DRIVER_ON_SET_POINT_REG, high_set_point);


}
