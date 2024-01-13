/*
 * Copyright (c) 2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xuartlite_l.h"

void print(char *str);

XGpio Gpio_button;
XGpio Gpio_switch;

int main()
{
	init_platform();
	int Status_button;
	int Status_switch;
	Status_button = XGpio_Initialize(&Gpio_button, XPAR_AXI_GPIO_BUTTON_DEVICE_ID);
	if (Status_button != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Status_switch = XGpio_Initialize(&Gpio_switch, XPAR_AXI_GPIO_SWITCH_DEVICE_ID);
	if (Status_switch != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(&Gpio_button, 1, 0xf);
	XGpio_SetDataDirection(&Gpio_switch, 1, 0xffff);

	int value_button;
	int value_switch;
	int temp_button;
	int temp_switch;

	temp_button = XGpio_DiscreteRead(&Gpio_button, 1);
	temp_switch = XGpio_DiscreteRead(&Gpio_switch, 1);

	while(1)
	{
		value_button = XGpio_DiscreteRead(&Gpio_button, 1);
		value_switch = XGpio_DiscreteRead(&Gpio_switch, 1);

		if(value_button != temp_button)
		{
		    if((value_button & 1) == 1)
				print("BTNU is pushed!\r\n");

			if((value_button & 2) == 2)
				print("BTNL is pushed!\r\n");

			if((value_button & 4) == 4)
				print("BTNR is pushed!\r\n");

			if((value_button & 8) == 8)
				print("BTND is pushed!\r\n");

			temp_button = value_button;
		}

		if(value_switch != temp_switch)
		{
			if((value_switch & 1) == 1)
				print("SW0:ON  ");

			if((value_switch & 2) == 2)
				print("SW1:ON  ");

			if((value_switch & 4) == 4)
				print("SW2:ON  ");

			if((value_switch & 8) == 8)
				print("SW3:ON  ");

			if((value_switch & 16) == 16)
				print("SW4:ON  ");

			if((value_switch & 32) == 32)
				print("SW5:ON  ");

			if((value_switch & 64) == 64)
				print("SW6:ON  ");

			if((value_switch & 128) == 128)
				print("SW7:ON  ");

			if((value_switch & 256) == 256)
				print("SW8:ON  ");

			if((value_switch & 512) == 512)
				print("SW9:ON  ");

			if((value_switch & 1024) == 1024)
				print("SW10:ON  ");

			if((value_switch & 2048) == 2048)
				print("SW11:ON  ");

			if((value_switch & 4096) == 4096)
				print("SW12:ON  ");

			if((value_switch & 8192) == 8192)
				print("SW13:ON  ");

			if((value_switch & 16384) == 16384)
				print("SW14:ON  ");

			if((value_switch & 32768) == 32768)
				print("SW15:ON  ");

			if(value_switch != 0)
				print("\r\n");

			temp_switch = value_switch;
		}
	}
    cleanup_platform();
    return 0;
}
