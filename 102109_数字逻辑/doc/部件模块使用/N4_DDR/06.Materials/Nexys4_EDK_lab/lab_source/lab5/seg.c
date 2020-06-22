/* $Id: xgpio_example.c,v 1.1.2.1 2009/11/25 07:38:15 svemula Exp $ */
/******************************************************************************
*
* (c) Copyright 2002-2009 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio_example.c
*
* This file contains a design example using the GPIO driver (XGpio) and hardware
* device.  It only uses a channel 1 of a GPIO device.
*
* This example can be ran on the Xilinx ML300 board using the Prototype Pins &
* LEDs of the board connected to the GPIO.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  03/13/02 First release
* 1.00a rpm  08/04/03 Removed second example and invalid macro calls
* 2.00a jhl  12/15/03 Added support for dual channels
* 2.00a sv   04/20/05 Minor changes to comply to Doxygen and coding guidelines
* 3.00a ktn  11/20/09 Minor changes as per coding guidelines.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xgpio.h"
#include "xil_io.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

#define LED 0x01   /* Assumes bit 0 of GPIO is connected to an LED  */

/*
 * The following constant maps to the name of the hardware instances that
 * were created in the EDK XPS system.
 */
#define GPIO_EXAMPLE_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID

/*
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye.  This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY     1000000

/*
 * The following constant is used to determine which channel of the GPIO is
 * used for the LED if there are 2 channels supported.
 */
#define LED_CHANNEL 1

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#ifdef PRE_2_00A_APPLICATION

/*
 * The following macros are provided to allow an application to compile that
 * uses an older version of the driver (pre 2.00a) which did not have a channel
 * parameter. Note that the channel parameter is fixed as channel 1.
 */
#define XGpio_SetDataDirection(InstancePtr, DirectionMask) \
        XGpio_SetDataDirection(InstancePtr, LED_CHANNEL, DirectionMask)

#define XGpio_DiscreteRead(InstancePtr) \
        XGpio_DiscreteRead(InstancePtr, LED_CHANNEL)

#define XGpio_DiscreteWrite(InstancePtr, Mask) \
        XGpio_DiscreteWrite(InstancePtr, LED_CHANNEL, Mask)

#define XGpio_DiscreteSet(InstancePtr, Mask) \
        XGpio_DiscreteSet(InstancePtr, LED_CHANNEL, Mask)

#endif

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */

XGpio Gpio,Gpio1; /* The Instance of the GPIO Driver */

/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the GPIO level 1
* driver to turn on and off an LED.
*
* @param	None
*
* @return	XST_FAILURE to indicate that the GPIO Intialisation had failed.
*
* @note		This function will not return if the test is running.
*
******************************************************************************/
int select(int m);


int main(void)
{

	int Status,i,m,n,p,q,temp1,temp2,temp3,temp4;


	/*
	 * Initialize the GPIO driver
	 */
	Status = XGpio_Initialize(&Gpio, XPAR_AXI_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XGpio_Initialize(&Gpio1, XPAR_LED_7SEGMENT_DEVICE_ID);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
    XGpio_SetDataDirection(&Gpio, 0x1, 0);

    while(1)
    {
    	for(p=0;p<10;p++)
    	{
    		temp2=select(p);

    	for(q=0;q<100;q++)
    	   {
    			XGpio_DiscreteWrite(&Gpio1, 0x1, temp2);
    	    	XGpio_DiscreteWrite(&Gpio, 0x1, 0xfd);
    	    	for(i=0;i<60000;i++);
    	    	temp1=select(q/10);

    	    	XGpio_DiscreteWrite(&Gpio1, 0x1, temp1);
    	    	XGpio_DiscreteWrite(&Gpio, 0x1, 0xfe);
    	    	for(i=0;i<60000;i++);


    	    }
    }
    }

	return XST_SUCCESS;

}
int select(int m)
{
	int temp=0;
	switch(m)
	{
	case 0:
		temp=0b0000001;
		break;
	case 1:
		temp=0b1001111;
		break;
	case 2:
		temp=0b0010010;
		break;
	case 3:
		temp=0b0000110;
		break;
	case 4:
		temp=0b1001100;
		break;
	case 5:
		temp=0b0100100;
		break;
	case 6:
		temp=0b0100000;
		break;
	case 7:
		temp=0b0001111;
		break;
	case 8:
		temp=0b0000000;
		break;
	case 9:
		temp=0b0000100;
		break;
	default:
		temp=0b0000001;
		break;
	}
	return temp;

}
