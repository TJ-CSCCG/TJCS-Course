/***************************** Include Files *********************************/
/*****************************************************************************/
#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "i2c.h"
#include "ADT7420.h"
#include "mb_interface.h"
#include "xil_printf.h"

void uartIntHandler(void);
char rxData = '0';

int main()
{
	Xil_ICacheEnable();
	Xil_DCacheEnable();

	// Set Interrupt Handler
	microblaze_register_handler((XInterruptHandler)uartIntHandler, (void*)0);
	// Enable UART Interrupts
	Xil_Out32(XPAR_RS232_BASEADDR+0x0C, (1 << 4));
	// Enable Microblaze Interrupts
	microblaze_enable_interrupts();

	// Initialize ADT7420 Device
	ADT7420_Init();

	// Display Main Menu on UART
	ADT7420_DisplayMainMenu();

    do{
    	rxData = inbyte();
    	inbyte();
    	switch(rxData)
    	{
    	case 't':
    		Display_Temp(ADT7420_ReadTemp());
    		break;
    	case 'r':
    		ADT7420_SetResolution();
    		break;
    	case 'h':
    		ADT7420_DisplaySetTHighMenu();
    		break;
    	case 'l':
    		ADT7420_DisplaySetTLowMenu();
    		break;
    	case 'c':
    		ADT7420_DisplaySetTCritMenu();
    		break;
        case 'y':
        	ADT7420_DisplaySetTHystMenu();
    		break;
    	case 'f':
    		ADT7420_DisplaySetFaultQueueMenu();
    		break;
    	case 's':
			ADT7420_DisplaySettings();
			break;
    	case 'm':
    		ADT7420_DisplayMenu();
    		break;
    	case '0':
    		break;
    	default:
    		xil_printf("\n\rWrong option! Please select one of the options below.");
    		ADT7420_DisplayMenu();
    		break;
    	}
    }while(rxData != 'q');

	xil_printf("Exiting application\n\r");

	exit(0);

   Xil_DCacheDisable();
   Xil_ICacheDisable();

   return 0;
}

void uartIntHandler(void)
{
	if(Xil_In32(XPAR_RS232_BASEADDR + 0x08) & 0x01)
		{
			rxData = Xil_In32(XPAR_UARTLITE_0_BASEADDR);
		}
}
