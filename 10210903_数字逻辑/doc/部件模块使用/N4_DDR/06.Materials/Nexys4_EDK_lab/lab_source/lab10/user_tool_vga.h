/*
 * user_tool_vga.h
 *
 *  Created on: 2013-12-2
 *      Author: ForeverHyjal
 */

#ifndef USER_TOOL_VGA_H_
#define USER_TOOL_VGA_H_
#include "ip_vga.h"
#include "xparameters.h"
static int sin[64]={0x80,0x8c,0x98,0xa4,0xb0,0xbb,0xc6,0xd0,
		 	 	 	0xd9,0xe2,0xe9,0xf0,0xf5,0xf9,0xfc,0xfe,
		 	 	 	0xff,0xfe,0xfc,0xf9,0xf5,0xf0,0xe9,0xe2,
		 	 	 	0xd9,0xd0,0xc6,0xbb,0xb0,0xa4,0x98,0x8c,
		 	 	 	0x80,0x73,0x67,0x5b,0x4f,0x44,0x39,0x2f,
		 	 	 	0x26,0x1d,0x16,0x0f,0x0a,0x06,0x03,0x01,
		 	 	 	0x01,0x01,0x03,0x06,0x0a,0x0f,0x16,0x1d,
		 	 	 	0x26,0x2f,0x39,0x44,0x4f,0x5b,0x67,0x73};
void Tool_vga_q();
void Tool_vga_sin(int f,int t,int h,int v);
void Tool_vga_sin_q();
void Tool_vga_free(int ho,int ve);
void Tool_vga_free_q();
int Tool_vga_xy_a();
int Tool_vga_xy_b();
void Tool_vga_xy_p(int a,int b);
void Tool_vga_xy_q();
int Tool_vga_pw_a();
void Tool_vga_pw_p(int v);
void Tool_vga_pw_q();


void Top_vga();
void Top_vga_sin();
void Top_vga_free();
void Top_vga_xy();
void Top_vga_pw();

int Choice()
{
	int c;
	c=inbyte();
	return c;
}

void Tool_vga_q()
{
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x0);
}
void Tool_vga_sin(int f,int t,int h,int v)
{
	int i;
	int j;
	for(i=0;i<640;i++)
	{
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x3);
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG1_OFFSET,i);
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG2_OFFSET,f*sin[((i+h)/t)%64]/4+v);
		for(j=0;j<100;j++);
	}
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x2);
}

void Tool_vga_sin_q()
{
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x0);
}

void Tool_vga_free(int ho,int ve)
{
	int j;
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x3);
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG1_OFFSET,ho);
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG2_OFFSET,ve);
	for(j=0;j<100;j++);
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x2);
}

void Tool_vga_free_q()
{
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x0);
}

int Tool_vga_xy_a()
{
	int ch=-1;
	int dig;
	int a=0;
	while(1)
	{
		while((ch=Choice())==-1);
		if(ch=='#')
			break;
		if(ch<'0' || ch>'9')
			continue;
		dig=ch-'0';
		a=a*10+dig;
	}
	return a;
}

int Tool_vga_xy_b()
{
	int ch=-1;
	int dig;
	int b=0;
	while(1)
	{
		while((ch=Choice())==-1);
		if(ch=='#')
			break;
		if(ch<'0' || ch>'9')
			continue;
		dig=ch-'0';
		b=b*10+dig;
	}
	return b;
}

void Tool_vga_xy_p(int a,int b)
{
	int i;
	int j;
	for(i=0;i<640;i++)
	{
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x3);
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG1_OFFSET,i);
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG2_OFFSET,480-i*a+b);
		for(j=0;j<100;j++);
	}
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x2);
}

void Tool_vga_xy_q()
{
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x0);
}

int Tool_vga_pw_a()
{
	int ch=-1;
	int dig;
	int v=0;
	while(1)
	{
		while((ch=Choice())==-1);
		if(ch=='#')
			break;
		if(ch<'0' || ch>'9')
			continue;
		dig=ch-'0';
		v=v*10+dig;
	}
	return v;
}
void Tool_vga_pw_p(int v)
{
	int j;
	int i;
	int t=0;
	int s=0;
	while(1)
	{
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x3);
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG1_OFFSET,s);
		s=s+v;
		IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG2_OFFSET,0.5*t*t);
		for(j=0;j<100000;j++)
			for(i=0;i<10;i++);
		t=t+1;
		if(s>=640 || 0.5*t*t>=480)
			break;
	}
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x2);
}


void Tool_vga_pw_q()
{
	IP_VGA_mWriteReg(XPAR_IP_VGA_0_BASEADDR,IP_VGA_SLV_REG0_OFFSET,0x0);
}




void Top_vga()
{
	int ch=-1;
	while(1)
	{
		Menu_vga();
		while((ch=Choice())==-1);
		switch(ch)
		{
		case 'a':Top_vga_sin();break;
		case 'b':Top_vga_free();break;
		case 'c':Top_vga_xy();break;
		case 'd':Top_vga_pw();break;
		case 'q':Tool_vga_q();return;
		default:break;
		}
		ch=-1;
	}
}

void Top_vga_sin()
{
	int f=1;
	int t=1;
	int h=0;
	int v=0;
	int ch=-1;
	Menu_vga_sin();
	Tool_vga_sin(f,t,h,v);
	while(1)
	{
		while((ch=Choice())==-1);
		switch(ch)
		{
		case 'w':f=f+1;Tool_vga_sin(f,t,h,v);break;
		case 's':f=f-1;if(f==0)f=1;Tool_vga_sin(f,t,h,v);break;
		case 'a':t=t>>1;if(t==0)t=1;Tool_vga_sin(f,t,h,v);break;
		case 'd':t=t<<1;if(t==0)t=1;Tool_vga_sin(f,t,h,v);break;
		case 'j':h++;Tool_vga_sin(f,t,h,v);break;
		case 'k':h--;if(h==-1)h=0;Tool_vga_sin(f,t,h,v);break;
		case 'i':v--;if(v==-1)v=0;Tool_vga_sin(f,t,h,v);break;
		case 'm':v++;Tool_vga_sin(f,t,h,v);break;
		case 'q':Tool_vga_sin_q();return;
		default:break;
		}
		ch=-1;
	}
}

void Top_vga_free()
{
	Menu_vga_free();
	int ch=-1;
	int ho=0;
	int ve=200;
	Tool_vga_free(ho,ve);
	while(1)
	{
		while((ch=Choice())==-1);
		switch(ch)
		{
		case 'w':ve=ve-1;ho=ho+1;Tool_vga_free(ho,ve);break;
		case 's':ve=ve+1;ho=ho+1;Tool_vga_free(ho,ve);break;
		case 'q':Tool_vga_free_q();return;
		default:break;
		}
		ch=-1;
	}
}

void Top_vga_xy()
{
	int ch=-1;
	int a;
	int b;
	while(1)
	{
		Menu_vga_xy();
		while((ch=Choice())==-1);
		switch(ch)
		{
		case 'a':print("\r\ninput a>");a=Tool_vga_xy_a();break;
		case 'b':print("\r\ninput b>");b=Tool_vga_xy_b();break;
		case 'c':Tool_vga_xy_p(a,b);break;
		case 'q':Tool_vga_xy_q();return;
		default :break;
		}
	}
}

void Top_vga_pw()
{
	int ch=-1;
	int v;
	while(1)
	{
		Menu_vga_pw();
		while((ch=Choice())==-1);
		switch(ch)
		{
		case 'a':print("\r\ninput V>");v=Tool_vga_pw_a();break;
		case 'b':Tool_vga_pw_p(v);break;
		case 'q':Tool_vga_pw_q();return;
		default:break;
		}
	}
}
#endif /* USER_TOOL_VGA_H_ */
