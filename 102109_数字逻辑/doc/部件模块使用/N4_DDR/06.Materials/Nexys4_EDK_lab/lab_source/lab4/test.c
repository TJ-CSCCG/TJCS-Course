#include "xparameters.h"
#include "xgpio.h"
#include "xutil.h"
#include "led_ip.h"

int main (void)
{
    XGpio dip, push,led;
	int i, psb_check, dip_check,Status;

    xil_printf("-- Start of the Program --\r\n");

    XGpio_Initialize(&dip, XPAR_DIP_SWITCHES_DEVICE_ID);
	XGpio_SetDataDirection(&dip, 1, 0);

	XGpio_Initialize(&push, XPAR_PUSH_BUTTONS_DEVICE_ID);
	XGpio_SetDataDirection(&push, 1, 0);

	Status = XGpio_Initialize(&led, XPAR_LED_IP_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(&led, 1, 0);

	while (1){
	  psb_check = XGpio_DiscreteRead(&push, 1);
	  xil_printf("Push Buttons Status %x\r\n", psb_check);
	  dip_check = XGpio_DiscreteRead(&dip, 1);
	  xil_printf("DIP Switch Status %x\r\n", dip_check);

	  // output dip switches value on LED_ip device
	  LED_IP_mWriteReg(XPAR_LED_IP_0_BASEADDR, 0, dip_check);

	  for (i=0; i<999999; i++);
	}
}
