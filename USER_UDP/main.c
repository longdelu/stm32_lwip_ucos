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
 ALIENTEK ������STM32F429����ʵ��2
 LWIP��UCOSIIϵͳ��ֲ
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com  
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//��LCD����ʾ��ַ��Ϣ����
//�������ȼ�
#define DISPLAY_TASK_PRIO   8
//�����ջ��С
#define DISPLAY_STK_SIZE   128
//�����ջ
OS_STK   DISPLAY_TASK_STK[DISPLAY_STK_SIZE];
//������
void display_task(void *pdata);


//udp��������������
//�������ȼ�
#define  UDP_TASK_PRIO      1
//�����ջ��С
#define UDP_STK_SIZE      1024
//�����ջ
OS_STK   UDP_TASK_STK[UDP_STK_SIZE];
//������
void udp_task(void *pdata);  


//START����
//�������ȼ�
#define START_TASK_PRIO      10
//�����ջ��С
#define START_STK_SIZE      128
//�����ջ
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata); 

//��LCD����ʾ��ַ��Ϣ
//mode:1 ��ʾDHCP��ȡ���ĵ�ַ
//     ���� ��ʾ��̬��ַ
void show_address(u8 mode)
{
   if(mode==2)
   {
      printf("DHCP IP :%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);                  //��ӡ��̬IP��ַ

      printf("DHCP GW :%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);   //��ӡ���ص�ַ

      printf("NET MASK:%d.%d.%d.%d",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);   //��ӡ���������ַ

   }
   else 
   {
      printf("Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);                  //��ӡ��̬IP��ַ

      printf("Static GW:%d.%d.%d.%d",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);   //��ӡ���ص�ַ


   }   
}

int main(void)
{   
    Stm32_Clock_Init(360,25,2,8);   //����ʱ��,180Mhz   
    HAL_Init();                     //��ʼ��HAL��
    delay_init(180);                //��ʼ����ʱ����
    uart_init(115200);              //��ʼ��USART
    usmart_dev.init(90);           //��ʼ��USMART   
    LED_Init();                     //��ʼ��LED 
    KEY_Init();                     //��ʼ������
    SDRAM_Init();                   //��ʼ��SDRAM
    PCF8574_Init();                 //��ʼ��PCF8574
    my_mem_init(SRAMIN);          //��ʼ���ڲ��ڴ��
    my_mem_init(SRAMEX);          //��ʼ���ⲿ�ڴ��
    my_mem_init(SRAMCCM);          //��ʼ��CCM�ڴ��
    POINT_COLOR = RED;               //��ɫ����
    printf("Apollo STM32F4/F7");
    printf("LWIP+UCOSII Test");
    
   OSInit();                    //UCOS��ʼ��
   while(lwip_comm_init())        //lwip��ʼ��
   {
      printf("Lwip Init failed!");    //lwip��ʼ��ʧ��
      delay_ms(500);
      LCD_Fill(30,110,230,150,WHITE);
      delay_ms(500);
   }
   printf("Lwip Init Success!");       //lwip��ʼ���ɹ�
   OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
   OSStart(); //����UCOS
}

//start����
void start_task(void *pdata)
{
   OS_CPU_SR cpu_sr;
   pdata = pdata ;
   
   OSStatInit();           //��ʼ��ͳ������
   OS_ENTER_CRITICAL();     //���ж�
#if LWIP_DHCP
   lwip_comm_dhcp_creat(); //����DHCP����
#endif
   
   OSTaskCreate(display_task,(void*)0,(OS_STK*)&DISPLAY_TASK_STK[DISPLAY_STK_SIZE-1],DISPLAY_TASK_PRIO); //��ʾ����
   OSTaskSuspend(OS_PRIO_SELF); //����start_task����
   OS_EXIT_CRITICAL();        //���ж�
}

//��ʾ��ַ����Ϣ
void display_task(void *pdata)
{
   while(1)
   { 
#if LWIP_DHCP                           //������DHCP��ʱ��
      if(lwipdev.dhcpstatus != 0)          //����DHCP
      {
         show_address(lwipdev.dhcpstatus );   //��ʾ��ַ��Ϣ
          
         OSTaskCreate(udp_task,(void*)0,(OS_STK*)&UDP_TASK_STK[UDP_STK_SIZE-1],UDP_TASK_PRIO);//����UDP���������� 
          
         OSTaskSuspend(OS_PRIO_SELF);       //��ʾ���ַ��Ϣ�������������, ��������ᵼ����������ִ��һ�ε���
          
      }
#else
      show_address(0);                   //��ʾ��̬��ַ
      OSTaskSuspend(OS_PRIO_SELF);           //��ʾ���ַ��Ϣ�������������
#endif //LWIP_DHCP
      OSTimeDlyHMSM(0,0,0,500);
   }
}


//udp����������
void udp_task(void *pdata)
{
   udp_demo_init();
   while(1)
   {
      LED1 = !LED1;
      OSTimeDlyHMSM(0,0,0,500);  //��ʱ500ms
    }
}
