/*
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION 
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * 
 *
 * This file is a generated sample test application.
 *
 * This application is intended to test and/or illustrate some 
 * functionality of your system.  The contents of this file may
 * vary depending on the IP in your system and may use existing
 * IP driver functions.  These drivers will be generated in your
 * SDK application project when you run the "Generate Libraries" menu item.
 *
 */

#include "ADXL362.h"
#include "xil_cache.h"

/*****************************************************************************/
/************************ Functions Declarations *****************************/
/*****************************************************************************/
void uartIntHandler(void);

/*****************************************************************************/
/********************** Variable Definitions *********************************/
/*****************************************************************************/
volatile char rxData = 0;
volatile char mode = 0;

/******************************************************************************
* @brief Main function.
*
* @return Always returns 0.
******************************************************************************/
int main() 
{
   Xil_ICacheEnable();
   Xil_DCacheEnable();

   // Enable Interrupts
   microblaze_register_handler((XInterruptHandler)uartIntHandler, (void *)0);
   Xil_Out32(XPAR_RS232_UART_1_BASEADDR+0xC, (1 << 4));
   microblaze_enable_interrupts();

   // Initialize SPI
   SPI_Init(SPI_BASEADDR, 0, 0, 0);

   // Software Reset
   ADXL362_WriteReg(ADXL362_SOFT_RESET, ADXL362_RESET_CMD);
   delay_ms(10);
   ADXL362_WriteReg(ADXL362_SOFT_RESET, 0x00);

   // Enable Measurement
   ADXL362_WriteReg(ADXL362_POWER_CTL, (2 << ADXL362_MEASURE));

   ADXL362_DisplayMainMenu();

   while(1)
   {
	  switch(rxData)
	   {
	   	   case 'a':
	   		   (mode == 0) ? ADXL362_DisplayAll() : ADXL362_DisplayAllSmall();
	   		   delay_ms(100);
	   		   break;
	   	   case 'x':
	   		   (mode == 0) ? ADXL362_Print('x') : ADXL362_PrintSmall('x');
	   		   delay_ms(100);
	   		   break;
	   	   case 'y':
	   		   (mode == 0) ? ADXL362_Print('y') : ADXL362_PrintSmall('y');
	   		   delay_ms(100);
	   		   break;
	   	   case 'z':
	   		   (mode == 0) ? ADXL362_Print('z') : ADXL362_PrintSmall('z');
	   		   delay_ms(100);
	   		   break;
	   	   case 't':
	   		   ADXL362_PrintTemp();
	   		   break;
	   	   case 'r':
	   		   ADXL362_SetRange();
	   		   break;
	   	   case 's':
	   		   ADXL362_SwitchRes();
	   		   break;
	   	   case 'm':
	   		   ADXL362_DisplayMainMenu();
	   		   break;
	   	   case 'i':
	   		   ADXL362_PrintID();
	   		   break;
	   	   case 0:
	   		   break;
	   	   default:
	   		   xil_printf("\n\r> Wrong option! Please select one of the options below");
	   		   ADXL362_DisplayMenu();
	   		   break;
	   }
   }

   Xil_DCacheDisable();
   Xil_ICacheDisable();

   return 0;
}

/******************************************************************************
* @brief Interrupt Handler for UART.
*
* @return None.
******************************************************************************/
void uartIntHandler(void)
{
	if(Xil_In32(XPAR_RS232_UART_1_BASEADDR + 0x08) & 0x01)
	{
		rxData = Xil_In32(XPAR_RS232_UART_1_BASEADDR);
	}
}

