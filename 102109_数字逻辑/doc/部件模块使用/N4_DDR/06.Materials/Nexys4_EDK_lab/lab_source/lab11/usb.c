/*
 * usb.c
 *
 *  Created on: 2013-11-26
 *      Author: tp
 */

#include "xps2.h"
#include "xparameters.h"
#include "xstatus.h"
#include "stdio.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define PS2_DEVICE_ID		XPAR_PS2_0_DEVICE_ID


/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

#define TOTAL_TEST_BYTES	18 	/* Total Number of bytes to be
				      		transmitted/received */
#define KEYBOARD_ACK		0xFA	/* ACK from keyboard */
#define printf xil_printf	   	/* A smaller footprint printf */

/************************** Function Prototypes *****************************/

int Ps2PolledExample(u16 Ps2DeviceId);


/************************** Variable Definitions ****************************/

static XPs2 Ps2Inst; 		/* Ps2 driver instance */

/*
 * Transmit Buffer contains data for glowing the LEDs on the
 * PS/2 keyboard.
 */
u8 TxBuffer[TOTAL_TEST_BYTES] = {0xED, 0x00, 0xED, 0x01,
					0xED, 0x02, 0xED, 0x04,
					0xED, 0x07, 0xED, 0x06,
					0xED, 0x01, 0xED, 0x00,
					0xED, 0x07};
/*
 * Receive Buffer.
 */
u8 RxBuffer;


/****************************************************************************/
/**
*
* Main function that invokes the PS/2 polled example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{

	int Status;

	/*
	 * Run the Ps2PolledExample, specify the parameters that
	 * are generated in xparameters.h.
	 */
	Status = Ps2PolledExample(PS2_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;

}

/****************************************************************************/
/**
*
* This function runs a polled mode test on the Ps2 device using the driver
* APIs.
*
* @param	Ps2DeviceId is the XPAR_<PS2_instance>_DEVICE_ID value
*		from xparameters.h.
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
*
****************************************************************************/
int Ps2PolledExample(u16 Ps2DeviceId)
{
	int Status;
	XPs2_Config *ConfigPtr;
	u32 Count;
	u32 BytesSent;
	u32 StatusReg;
	u32 BytesReceived;
	u32 Delay;


	/*
	 * Initialize the Ps2 driver.
	 */
	ConfigPtr = XPs2_LookupConfig(Ps2DeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	XPs2_CfgInitialize(&Ps2Inst, ConfigPtr, ConfigPtr->BaseAddress);

	/*
	 * Self Test the Ps2 device.
	 */
	Status = XPs2_SelfTest(&Ps2Inst);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	printf("\r\nPS/2 Demo using Polled Mode\r\n");

	/*
	 * Send the data to a PS/2 keyboard.
	 */
	printf("Transmit some bytes to the PS/2 device \r\n");
	printf("Observe that the SCROLL/NUM/CAPS Lock Led's toggle \r\n");


	/*
	 * Receive the data from a PS/2 keyboard.
	 */
	Count = 1;
	printf("\r\n Press the Keys on the keyboard \r\n");
	printf("Echoing PS/2 scan codes from a PS/2 input device \r\n");
	while (1) {

		do {
			StatusReg = XPs2_GetStatus(&Ps2Inst);
		}while((StatusReg & XPS2_STATUS_RX_FULL) == 0);
		BytesReceived = XPs2_Recv(&Ps2Inst, &RxBuffer, 1);

		printf ("%x \r\n", RxBuffer);
		Count ++;
	}

	return XST_SUCCESS;
}

