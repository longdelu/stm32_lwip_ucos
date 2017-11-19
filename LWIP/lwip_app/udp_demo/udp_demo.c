#include "udp_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 
//////////////////////////////////////////////////////////////////////////////////    
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F4&F7������
//UDP ���Դ���      
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2016/2/29
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved                             
//*******************************************************************************
//�޸���Ϣ
//��������Ϊudp���������󶨵�һ����֪�Ķ˿ں�����
//////////////////////////////////////////////////////////////////////////////////       
 
//UDP�������ݻ�����
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];   //UDP�������ݻ����� 
//UDP������������
const u8 *tcp_demo_sendbuf="UDP demo send data\r\n";

/*
 * \brief UDP�ص�����
 */
static void __udp_demo_recv_callback(void *arg,
		                             struct udp_pcb *upcb,
									 struct pbuf *p,
									 struct ip_addr *addr,
									 u16_t port)
{

	udp_sendto(upcb, p,addr,port); //ֱ�ӽ����յ������ݷ��ص�Դ��

	pbuf_free(p);                  //�ͷŵ����ݰ��ռ䡣
}


//UDP����
void udp_demo_init(void)
{
 
   struct udp_pcb *udp_pcb;

   struct ip_addr   server_ipaddr;  	//�����屾��(ip)������ip��ַ

   IP4_ADDR(&server_ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);

   /* ����UDP���ƿ� */
   udp_pcb = udp_new();
   
   /* ��������Ϊ���������󶨵�һ����֪�Ķ˿�(echo�˿�) */
   udp_bind(udp_pcb, &server_ipaddr, UDP_DEMO_PORT);

   /* ע��UDP���������ջص����� */
   udp_recv(udp_pcb, __udp_demo_recv_callback, NULL);
} 

























