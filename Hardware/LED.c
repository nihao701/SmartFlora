#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//初始化
	GPIO_InitTypeDef GPIO_InitStrcture;
	GPIO_InitStrcture.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStrcture.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_0;
	GPIO_InitStrcture.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStrcture);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1);
}
void LED_Set(int LED,int Mode)
{
	if(LED==0)
	{
		if(Mode==0)
			GPIO_ResetBits(GPIOA,GPIO_Pin_0);
		if(Mode==1)
			GPIO_SetBits(GPIOA,GPIO_Pin_0);
			
	}
	if(LED==1)
	{
		if(Mode==0)
			GPIO_ResetBits(GPIOA,GPIO_Pin_1);
		if(Mode==1)
			GPIO_SetBits(GPIOA,GPIO_Pin_1);
			
	}
}

void LED0_Turn(void)//取反
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_0)==0)//如果当前LED1输出为0，即点亮，取反一下
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_0);
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	}
}

void LED1_Turn(void)//取反
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_1)==0)//如果当前LED1输出为0，即点亮，取反一下
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_1);
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
	}
}

void LED0_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);
}

void LED0_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_0);
}

void LED1_ON(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);
}

void LED1_OFF(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_1);
}
