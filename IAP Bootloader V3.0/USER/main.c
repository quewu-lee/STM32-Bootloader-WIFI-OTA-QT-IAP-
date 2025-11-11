#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "usart3.h"
#include "led.h"
#include "lcd.h"
#include "key.h"   
#include "stmflash.h" 
#include "iap.h"   
#include "oled.h"
#include "esp8266.h"
#include "string.h"
u32 i=0;
vu16 APP_LEN=0; //ESP8266接收到数据的长度
vu16 APP_LEN_LOAD=0; //ESP8266接收到APP数据的长度
char* str1="ABCDEFGH"; //IAP程序执行判断字符数组
char* str2="ABCDEFGHI"; //IAP程序烧录判断字符数组
void Display_mesage() //OLED显示信息
{
//    OLED_ShowString(0,0,"KEY_UP:CopyFLASH");    
//    OLED_ShowString(0,2,"KEY1:EraseRAMAPP");
//    OLED_ShowString(0,4,"KEY2:RunFLASHAPP");    
//    OLED_ShowString(0,6,"KEY0:RunRAMAPP"); 
      OLED_ShowCHinese(0,2,0);OLED_ShowCHinese(16,2,1);OLED_ShowCHinese(16*2,2,2);OLED_ShowCHinese(16*3,2,3);
      OLED_ShowCHinese(16*4,2,4);OLED_ShowCHinese(16*5,2,5);OLED_ShowCHinese(16*6,2,6);OLED_ShowCHinese(16*7,2,7);
      OLED_ShowCHinese(0,4,8);OLED_ShowCHinese(16,4,9);OLED_ShowCHinese(16*2,4,10);OLED_ShowCHinese(16*3,4,11);OLED_ShowCHinese(16*4,4,12);       
}
void USART3_RX_BUF_CLAER()
{
        //将APP缓存数组清零
    for (i = 0; i <16*1024; i++)
    {
        USART3_RX_BUF[i] = 0;
    }    
}
int main(void)
{ 
	u8 t;
	u8 key;
	u32 oldcount=0;	//老的串口接收数据值
	u32 applenth=0;	//接收到的app代码长度
	u8 clearflag=0; 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);        //初始化延时函数
	uart_init(460800);		//初始化串口波特率为460800
	usart3_init(115200);	//串口初始化为115200    
	LED_Init();				//初始化LED  
 	KEY_Init();				//按键初始化 
    OLED_Init();            //OLED初始化
    OLED_Clear();           //OLED清屏
    Display_mesage();	    //OLED显示提示信息
    esp8266_quit_trans();   //退出透传模式    
    esp8266_start_trans();  //esp8266进行初始化 连接WIFI NAME:CMCC-hgzE PID:qcpegygq 透传模式开启
	esp8266_send_data("ESP8266 successfully sent data to the server!!!",50); //服务器检测是否能正常传输文件
    USART3_RX_BUF_CLAER(); //清空数据接收数据缓存
    USART3_RX_STA=0;
    LED0=1;
    LED1=1;
    while(1)
    {
            if(USART3_RX_STA&0X8000)		//接收到一次数据了
        { 
            APP_LEN=(USART3_RX_STA&(~(0X01<<15))); //记录服务器发来的APP程序代码长度         
            if(APP_LEN>300) //接受的数据为APPD代码数据
            {
                APP_LEN_LOAD=APP_LEN; //获取APP程序烧录长度
                printf("数据大小=%dBytesr\n",APP_LEN_LOAD); //打印APP程序大小 单位字节 
                //将接受的数据赋值给APP代码数组                
                for (i = 0; i <APP_LEN_LOAD; i++) {
                     APP_data[i]=USART3_RX_BUF[i];                
                    }
                //打印发送过来的APP数据
                printf("APPdata数据：\r\n");
                printf("****************APP数据****************\r\n");            
                for (i = 0; i <APP_LEN_LOAD; i++) {
                     printf("%c",APP_data[i]);                
                    }
                printf("\r\n");            
                printf("****************APP数据****************\r\n");                 
            }
            
            if(APP_LEN==8) //接受的数据为IAP程序执行
            {
                for (i = 0; i <8; i++) 
                {
                    if(USART3_RX_BUF[i]==str1[i])
                    {
                        if(i==7)
                        {
                            printf("开始执行FLASH用户代码");
                            if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
                            {	 
                                iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
                            }else 
                            {
                                printf("非FLASH应用程序,无法执行!\r\n");
                                OLED_Clear();
                                OLED_ShowString(0,3,"Illegal FLASH APP!");                 
                            }									 
                            clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示                            
                        }
                    }
                    else
                    {
                        USART3_RX_BUF_CLAER(); //清空数据接收数据缓存
                        break;
                    }
                }
            }
            
            
            if(APP_LEN==9) //接受的数据为IAP程序烧录命令
            {
                for (i = 0; i <9; i++) 
                {
                    if(USART3_RX_BUF[i]==str2[i])
                    {
                        if(i==8)
                        {
                            if(APP_LEN_LOAD>300)
                            {
                                printf("开始更新固件...\r\n");	
                                OLED_Clear();
                                if(((*(vu32*)(APP_data+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
                                {                                
                                    OLED_ShowString(0,3,"Copying APP2FLASH...");       
                                    iap_write_appbin(FLASH_APP1_ADDR,APP_data,APP_LEN_LOAD);//更新FLASH代码  
                                    OLED_Clear();
                                    OLED_ShowString(0,3,"Copy APP Successed!!");                    
                                    printf("固件更新完成!\r\n");	
                                    printf("固件大小为%dBytes\r\n",APP_LEN_LOAD);
                                }
                                else
                                {
                                    OLED_Clear();
                                    OLED_ShowString(0,3,"Illegal FLASH APP! ");                                    
                                }
                                USART3_RX_BUF_CLAER(); //清空数据接收数据缓存
                            }else 
                            {
                                printf("没有可以更新的固件!\r\n");
                                LCD_ShowString(30,210,200,16,16,"No APP!");
                                OLED_Clear();
                                OLED_ShowString(0,3,"No APP!");                
                            }
                            clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示                            
                        }
                    }
                    else
                    {
                        USART3_RX_BUF_CLAER(); //清空数据接收数据缓存
                        break;
                    }
                }
            }                                                             
            USART3_RX_STA=0;  //接收完成标志位清零，准备接收下一次APP代码 
        }           
          
		t++;
		delay_ms(10);
		if(t==30)
		{
			LED0=!LED0;
			t=0;
			if(clearflag)
			{
				clearflag--;
				if(clearflag==0)
                {
                    OLED_Clear();
                    Display_mesage(); //恢复按键提示
                }
			}
		}
        
		key=KEY_Scan(0);
		if(key==WKUP_PRES)	//WK_UP按键按下
		{
			if(APP_LEN_LOAD>300)
			{
				printf("开始更新固件...\r\n");	
				LCD_ShowString(30,210,200,16,16,"Copying APP2FLASH...");
                OLED_Clear();
                OLED_ShowString(0,3,"Copying APP2FLASH...");    
  
                iap_write_appbin(FLASH_APP1_ADDR,APP_data,APP_LEN_LOAD);//更新FLASH代码  
                LCD_ShowString(30,210,200,16,16,"Copy APP Successed!!");
                OLED_Clear();
                OLED_ShowString(0,3,"Copy APP Successed!!");                    
                printf("固件更新完成!\r\n");	
                printf("固件大小为%dBytes\r\n",APP_LEN_LOAD);	
                USART3_RX_BUF_CLAER(); //清空数据接收数据缓存
 			}else 
			{
				printf("没有可以更新的固件!\r\n");
				LCD_ShowString(30,210,200,16,16,"No APP!");
                OLED_Clear();
                OLED_ShowString(0,3,"No APP!");                
			}
			clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示									 
		}
		if(key==KEY1_PRES)	//KEY1按下
		{
			if(applenth)
			{																	 
				printf("固件清除完成!\r\n");    
				LCD_ShowString(30,210,200,16,16,"APP Erase Successed!");
                OLED_Clear();
                OLED_ShowString(0,3,"APP Erase Successed!");                
				applenth=0;
			}else 
			{
				printf("没有可以清除的固件!\r\n");
				LCD_ShowString(30,210,200,16,16,"No APP!");
                OLED_Clear();
                OLED_ShowString(0,3,"No APP!");                 
			}
			clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示									 
		}
		if(key==KEY2_PRES)	//KEY2按下
		{
			printf("开始执行FLASH用户代码");
			if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{	 
				iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
			}else 
			{
				printf("非FLASH应用程序,无法执行!\r\n");
				LCD_ShowString(30,210,200,16,16,"Illegal FLASH APP!");	
                OLED_Clear();
                OLED_ShowString(0,3,"Illegal FLASH APP!");                 
			}									 
			clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示	  
		}
		if(key==KEY0_PRES)	//KEY0按下
		{
			printf("开始执行SRAM用户代码!!\r\n");
			if(((*(vu32*)(0X20001000+4))&0xFF000000)==0x20000000)//判断是否为0X20XXXXXX.
			{	 
				iap_load_app(0X20001000);//SRAM地址
			}else 
			{
				printf("非SRAM应用程序,无法执行!\r\n");
				LCD_ShowString(30,210,200,16,16,"Illegal SRAM APP!");
                OLED_Clear();
                OLED_ShowString(0,3,"Illegal SRAM APP!");                    
			}									 
			clearflag=7;//标志更新了显示,并且设置7*300ms后清除显示	 
		}				   
		 
	}  
}
