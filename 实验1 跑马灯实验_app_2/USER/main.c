#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "oled.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
//ALIENTEK 探索者STM32F407开发板 实验1
//跑马灯实验 -库函数版本
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK
int T=12;
float pitch,roll,yaw;//欧拉角
int temp,t;
int main(void)
{ 
 
	delay_init(168);		  //初始化延时函数
    SCB->VTOR = FLASH_BASE | 0x10000;//设置偏移量
	LED_Init();		        //初始化LED端口
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0,2,"LED_APP_RUNING"); 
    OLED_ShowString(0,4,"APP_LED=0.5S"); 
    LED0=1;
    LED1=1;
	while(1)
	{
        delay_ms(500);        
        LED0=!LED0;
        LED1=!LED1;       
	} 	
    }
 



