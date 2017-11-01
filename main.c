#include <reg52.h>
#include "hjc52.h"

sbit DC_OUT = P1^0;
sbit K1 = P3^4;
sbit K2 = P3^5;
sbit K3 = P3^6;
sbit K4 = P3^7;
bit PWM_STATE = 0;

unsigned char code table[]={0x3f,0x06,0x5b,
0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
0x77,0x7c,0x39,0x5e,0x79,0x71};

unsigned char TH0_HIGH = 0xff;
unsigned char TL0_HIGH = 0x7f;
unsigned char TH0_LOW = 0xff;
unsigned char TL0_LOW = 0x7f;

unsigned char PULSE_COUNT = 0;

unsigned char TABLE_STATE[6];

unsigned char SPEED = 0;

unsigned int TIMER_COUNT = 0;
unsigned int REAL_SPEED = 0;

void delayms(unsigned int xms)
{
 unsigned int i,j;
 for(i=xms;i>0;i--)
    for(j=110;j>0;j--);
}

void matrixkeyscan()
{
	if (K4 == 0)
	{
		delayms(2);
		if(K4 == 0)
		{
			if(SPEED != 126)
			{
				SPEED += 6;
			}
			while(K4 == 0);
		}
	}
	if (K3 == 0)
	{
		delayms(2);
		if(K3 == 0)
		{
			if(SPEED != 0)
			{
				SPEED -=6;
			}
			while(K3 == 0);
		}
	}
	if (K2 == 0)
	{
		delayms(2);
		if(K2 == 0)
		{
			SPEED = 0;
		}
		while(K2 == 0);
	}
	TABLE_STATE[5] = SPEED%10;
	TABLE_STATE[4] = SPEED%100/10;
	TABLE_STATE[3] = SPEED/100;
}

void main(void)
{
	TMOD=0X01; //对T0设置【C/T=0;作定时器之用；GATE=0,不受P3.2控制；M1M0=01，工作在方式1。】  T1不作设置，都为0。 所以TMOD=0X01。
	TH0=0;//T0定时器初值的高八位为0
	TL0=0;//T0定时器初值的低八位为0
	ET0=1; //允许定时器T0中断
	TR0=1; //允许定时器T0计数
	EA=1; //打开中断总允许开关
	
	hjc52_init();
	
	EX1=1;
	IT1 = 1;
	
	while(1)
	{
		int i = 0;
		int temp = 0;
		temp = 0x7f - SPEED;
		TL0_HIGH = temp;
		temp = 0x7f + SPEED;
		TL0_LOW = temp;
		
		matrixkeyscan();
		
		TABLE_STATE[2] = REAL_SPEED%10;
		TABLE_STATE[1] = REAL_SPEED%100/10;
		TABLE_STATE[0] = REAL_SPEED/100;
		
		for(i = 0;i != 6;++i)
		{
			unsigned char P0_OUT = 0;
			DU = 1;
			if(i == 1)
			{
				P0 = table[TABLE_STATE[i]] + 0x80;
			}
			else
			{
				P0 = table[TABLE_STATE[i]];
			}
			DU = 0;
			P0 = 0xff;
			WE = 1;
			P0_OUT = 0x01;
			P0_OUT = P0_OUT << i;
			P0_OUT = ~P0_OUT;
			P0 = P0_OUT;
			WE = 0;
			delayms(1);
		}
	}
}


void T0_Delay(void) interrupt 1
{
  TR0=0; //在执行定时器中断的过程中，首先停止定时器计数
	
	PWM_STATE = ~PWM_STATE;
	
	
	if(PWM_STATE)
	{
		TL0 = TL0_HIGH;
		TH0 = TH0_HIGH;
		TIMER_COUNT++;
		if(TIMER_COUNT == 3610)
		{
			TIMER_COUNT = 0;
			REAL_SPEED = PULSE_COUNT*3.14;
			PULSE_COUNT = 0;
		}
	}
	else
	{
		TL0 = TL0_LOW;
		TH0 = TH0_LOW;
	}
	
	DC_OUT = PWM_STATE;
	
  TR0=1; //退出中断程序之前，先打开定时器T0让其计数计数
}

void count(void) interrupt 2
{
	PULSE_COUNT++;
}