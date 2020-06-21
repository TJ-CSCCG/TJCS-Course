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
#include "audio_ip.h"

void print(char *str);

int main()
{
    init_platform();
    char *tone="";
    int i;
    print("You can press the key from 1 to 8\r\n");
    while(1)
    {
    	*tone = inbyte();
    	print(tone);
    	print("\r\n");
    	switch (*tone)
    	{
    		case '1': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b00111); break;
    		case '2': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b01001); break;
    		case '3': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b01011); break;
    		case '4': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b01100); break;
    		case '5': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b01110); break;
    		case '6': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b10000); break;
    		case '7': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b10010); break;
    		case '8': AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0b10011); break;
    		default : AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0);       break;
    	}
    	for (i=0; i<1000000; i++){}
    	AUDIO_IP_mWriteSlaveReg0(XPAR_AUDIO_IP_0_BASEADDR, 0, 0);
    }
    cleanup_platform();
    return 0;
}
