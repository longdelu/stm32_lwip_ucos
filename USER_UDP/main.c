#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sdram.h"
#include "lan8720.h" 
#include "timer.h"
#include "pcf8574.h"
#include "usmart.h"
#include "malloc.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "udp_demo.h" 
/************************************************
 ALIENTEK 阿波罗STM32F429网络实验2
 LWIP带UCOSII系统移植
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com  
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//在LCD上显示地址信息任务
//任务优先级
#define DISPLAY_TASK_PRIO   8
//任务堆栈大小
#define DISPLAY_STK_SIZE   128
//任务堆栈
OS_STK   DISPLAY_TASK_STK[DISPLAY_STK_SIZE];
//任务函数
void display_task(void *pdata);


//udp服务器函数任务
//任务优先级
#define  UDP_TASK_PRIO      1
//任务堆栈大小
#define UDP_STK_SIZE      1024
//任务堆栈
OS_STK   UDP_TASK_STK[UDP_STK_SIZE];
//任务函数
void udp_task(void *pdata);  


//START任务
//任务优先级
#define START_TASK_PRIO      10
//任务堆栈大小
#define START_STK_SIZE      128
//任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata); 

//在LCD上显示地址信息
//mode:1 显示DHCP获取到的地址
//     其他 显示静态地址
void show_address(u8 mode)
{
   if(mode==2)
   {
      printf("DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);                  //打印动态IP地址

      printf("DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);   //打印网关地址

      printf("NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);   //打印子网掩码地址

   }
   else 
   {
      printf("Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);                  //打印动态IP地址

      printf("Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);   //打印网关地址


   }   
}

int main(void)
{   
    Stm32_Clock_Init(360,25,2,8);   //设置时钟,180Mhz   
    HAL_Init();                     //初始化HAL库
    delay_init(180);                //初始化延时函数
    uart_init(115200);              //初始化USART
    usmart_dev.init(90);           //初始化USMART   
    LED_Init();                     //初始化LED 
    KEY_Init();                     //初始化按键
    SDRAM_Init();                   //初始化SDRAM
    PCF8574_Init();                 //初始化PCF8574
    my_mem_init(SRAMIN);          //初始化内部内存池
    my_mem_init(SRAMEX);          //初始化外部内存池
    my_mem_init(SRAMCCM);          //初始化CCM内存池
    POINT_COLOR = RED;               //红色字体
    printf("Apollo STM32F4/F7");
    printf("LWIP+UCOSII Test");
    
   OSInit();                    //UCOS初始化
   while(lwip_comm_init())        //lwip初始化
   {
      printf("Lwip Init failed!");    //lwip初始化失败
      delay_ms(500);
      LCD_Fill(30,110,230,150,WHITE);
      delay_ms(500);
   }
   printf("Lwip Init Success!");       //lwip初始化成功
   OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
   OSStart(); //开启UCOS
}

//start任务
void start_task(void *pdata)
{
   OS_CPU_SR cpu_sr;
   pdata = pdata ;
   
   OSStatInit();           //初始化统计任务
   OS_ENTER_CRITICAL();     //关中断
#if LWIP_DHCP
   lwip_comm_dhcp_creat(); //创建DHCP任务
#endif
   
   OSTaskCreate(display_task,(void*)0,(OS_STK*)&DISPLAY_TASK_STK[DISPLAY_STK_SIZE-1],DISPLAY_TASK_PRIO); //显示任务
   OSTaskSuspend(OS_PRIO_SELF); //挂起start_task任务
   OS_EXIT_CRITICAL();        //开中断
}

//显示地址等信息
void display_task(void *pdata)
{
   while(1)
   { 
#if LWIP_DHCP                           //当开启DHCP的时候
      if(lwipdev.dhcpstatus != 0)          //开启DHCP
      {
         show_address(lwipdev.dhcpstatus );   //显示地址信息
          
         OSTaskCreate(udp_task,(void*)0,(OS_STK*)&UDP_TASK_STK[UDP_STK_SIZE-1],UDP_TASK_PRIO);//创建UDP服务器任务 
          
         OSTaskSuspend(OS_PRIO_SELF);       //显示完地址信息后挂起自身任务, 挂起任务会导致立马重新执行一次调度
          
      }
#else
      show_address(0);                   //显示静态地址
      OSTaskSuspend(OS_PRIO_SELF);           //显示完地址信息后挂起自身任务
#endif //LWIP_DHCP
      OSTimeDlyHMSM(0,0,0,500);
   }
}


//udp服务器任务
void udp_task(void *pdata)
{
   udp_demo_init();
   while(1)
   {
      LED1 = !LED1;
      OSTimeDlyHMSM(0,0,0,500);  //延时500ms
    }
}
