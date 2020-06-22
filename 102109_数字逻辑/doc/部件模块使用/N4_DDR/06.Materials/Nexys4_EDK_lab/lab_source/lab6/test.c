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
#include "ip_da.h"
#include "ip_ad.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xuartlite.h"
#include "xuartlite_l.h"

void print(char *str);

void Test_AD()
{
	u32 R;
	int V;
	R=IP_AD_mReadReg(XPAR_IP_AD_0_BASEADDR,IP_AD_SLV_REG0_OFFSET);
	V=3.3*((double)R/(double)4095)*100;
	xil_printf("AD:%ld\r\n",R);
}

void Test_DA()
{
	u32 R;
	double V=2.7;   //此处修改数模转换的电压值 0-3.3
	R=(u32)((V/3.3)*4095);
	IP_DA_mWriteReg(XPAR_IP_DA_0_BASEADDR,IP_DA_SLV_REG0_OFFSET,R);
	xil_printf("DA:%ld\r\n",R);
}

int main()
{
    init_platform();

    char a;
    while(1)
    {
    a=inbyte();
    Test_AD();
    Test_DA();
    }



    cleanup_platform();

    return 0;
}

