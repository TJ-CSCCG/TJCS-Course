/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xgpio.h"
#include "led_ip.h"
#include "xstatus.h"


/************************** Constant Definitions *****************************/

int main(void)
{
	XGpio led;
	int Status;

	Status = XGpio_Initialize(&led, XPAR_LED_IP_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XGpio_SetDataDirection(&led, 1, 0);

	int i,j,k;
	int t;               //周期计数器(范围0~199)

	int duty_led[16];    //LED的占空(范围0~200;数值越大,LED越亮;下同)
	int duty_led_buf[16];//缓冲

	int num[16];
	int reg=0b0000000000000001;

	for(i=0;i<16;i++){
		if(i==0)
			num[i]=reg;
		else{
			reg=reg<<1;
			num[i]=reg;
		}
	}

 	char a;

 	while(1){
 	    print("please make the choice!\n\r");
        a=inbyte();      //接受来自串口的数据

		for(i=0;i<16;i++)
		   duty_led[i]=0;

		duty_led_buf[0] = 80;
		duty_led_buf[1] = 150;
		duty_led_buf[2] = 300;
		duty_led_buf[3] = 600;
		duty_led_buf[4] = 300;
		duty_led_buf[5] = 150;
		duty_led_buf[6] = 80;
		duty_led_buf[7] = 0;
		duty_led_buf[8] = 0;
		duty_led_buf[9] = 0;
		duty_led_buf[10] = 0;
		duty_led_buf[11] = 0;
		duty_led_buf[12] = 0;
		duty_led_buf[13] = 0;
		duty_led_buf[14] = 0;
		duty_led_buf[15] = 0;

       for( j = 0 ; j<50 ; j++ ){         //调节流水灯间隔时间
	      for( i = 0 ; i<20 ; i++ ){      //调节流水灯速度
	         for( t = 0 ; t<200  ; t++ ){ //根据占空比,控制每个LED的亮度
	              if(a=='1')        //设置串口输入为1流水灯从左往右流动
	              {
                    for(k=0;k<16;k++){
                      if(t<duty_led[k])
                    	LED_IP_mWriteReg(XPAR_LED_IP_0_BASEADDR,0,num[k]);
                      else      //0代表灯灭，1代表灯亮
                    	LED_IP_mWriteReg(XPAR_LED_IP_0_BASEADDR,0,0b0000000000000000);
                     }
	              }
	              else             //设置串口输入为其他时，流水灯从右往左流动
	              {
	            	for(k=0;k<16;k++){
	            	  if(t<duty_led[k])
	            		 LED_IP_mWriteReg(XPAR_LED_IP_0_BASEADDR,0,num[15-k]);
	            	  else
	            		  LED_IP_mWriteReg(XPAR_LED_IP_0_BASEADDR,0,0b0000000000000000);
	            	  }
	              }
	          }
	    }

	  //占空队列移动
	    for(i=0;i<16;i++){
	    	if(i==15)
	    		duty_led[i]=duty_led_buf[0];
	    	else
	    		duty_led[i]=duty_led[i+1];
	    }

	    for(i=0;i<16;i++){
	   	   if(i==15)
	   	       duty_led_buf[i]=0;
	   	   else
	   	       duty_led_buf[i]=duty_led_buf[i+1];
	   	}
	 }
  }
 	cleanup_platform();
}

