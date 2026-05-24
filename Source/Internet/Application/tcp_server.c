/*
 * tcp_server.c
 *
 *  Created on: Jul 5, 2020
 *      Author: timecy
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>

#include "os.h"
#include "buffer.h"
#include "std_spi.h"
#include "std_gpio.h"
#include "tcp_server.h"
#include "scmd.h"
#include "socket.h"

#ifdef _TCP_DEBUG_
   #include <stdio.h>
#endif

//#indef _TCP_OS_
//
//#endif


void network_init(void);

/* Private macro -------------------------------------------------------------*/

extern scmd_class scmd_ctrl;
extern spi_bus_class spi_bus_list[];
extern uint16_t spi_bus_list_qty;

uint8_t send_flag = 0;

//gpio_class w5500_reset = GPIOM_NEW(E, 15, GPIO_MODE_OUTPUT_PP);
gpio_class w5500_reset = GPIOM_NEW(D, 7, GPIO_MODE_OUTPUT_PP);

spi_dev_class wizchip_spi =
{
		.bus = &spi_bus_list[2],
		.cs =  GPIOM_NEW(D, 4, GPIO_MODE_OUTPUT_PP),
//		.cs =  GPIOM_NEW(E, 12, GPIO_MODE_OUTPUT_PP),
		.mode.cpol = spi_cpol_low,
		.mode.cpha = spi_cpha_1edge,
};

uint8_t 	sock_size[2][SOCK_QTY_MAX] = {{2,2,2,2,2,2,2,2}, {2,2,2,2,2,2,2,2}};
uint16_t 	local_port 		= 5000;
wiz_NetInfo gWIZNETINFO 	= {	.mac = 	{	0x00, 0x08, 0xdc,0x11, 0x11, 0x11	},
								.ip = 	{	10, 0, 0, 100						},
								.sn = 	{	255,0, 0, 0							},
								.gw = 	{	10, 0, 0, 1							},
								.dns = 	{	8, 8, 8, 8							},
								.dhcp = NETINFO_STATIC							};
uint8_t gDATABUF[DATA_BUF_SIZE];
uint8_t sock_payload[SOCK_QTY_MAX][DATA_BUF_SIZE];
buff_class sock_buff[SOCK_QTY_MAX];

uint16_t sock_fb_buff_len[8];
uint8_t  sock_fb_buff[8][2048];


void tcp_server_buff_init(void)
{
	for(uint8_t i = 0; i < SOCK_QTY_MAX; i++)
	{
		buff_init(&sock_buff[i], DATA_BUF_SIZE, sock_payload[i]);
	}
}

void vTCP_Server_Thread(void *argument)
{
	for(;;)
	{
		uint8_t i = 0;
		uint8_t tmp = 0;
		uint16_t len= 0;
		uint32_t macID = 0;
		macID = HAL_GetUIDw0();
		gWIZNETINFO.mac[3] = (macID >> 24) & 0xff;
		gWIZNETINFO.mac[2] = (macID >> 16) & 0xff;
		gWIZNETINFO.mac[1] = (macID >> 8 ) & 0xff;
		gWIZNETINFO.mac[0] = (macID >> 0 ) & 0xff;

		tcp_server_buff_init();
		gpio_config(&w5500_reset);
		gpio_config(&wizchip_spi.cs);
		TCP_Delay(10);
		gpio_set(&wizchip_spi.cs, GPIO_PIN_SET);
		gpio_set(&w5500_reset, GPIO_PIN_RESET);
		TCP_Delay(700);
		gpio_set(&w5500_reset, GPIO_PIN_SET);
		gpio_set(&wizchip_spi.cs, GPIO_PIN_RESET);
		// 注册临界区
		// 注册片选信号
		// 注册读写函数

		/* WIZCHIP SOCKET Buffer initialize */
		if(ctlwizchip(CW_INIT_WIZCHIP,(void*)sock_size) == -1){
			 printf("WIZCHIP Initialized fail.\r\n");
			 while(1);
		}

		/* PHY link status check */
		do{
			 if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1){
					printf("Unknown PHY Link stauts.\r\n");
			 }
		}while(tmp == PHY_LINK_OFF);

		/* Network initialization */
		network_init();

		for(;;)
		{
			switch(getSn_SR(i))										// 获取socketi的状态
			{
				case SOCK_INIT:										// Socket处于初始化完成(打开)状态
					listen(i);
					break;
				case SOCK_ESTABLISHED:								// Socket处于连接建立状态
					if(getSn_IR(i) & Sn_IR_CON)
					{
						setSn_IR(i, Sn_IR_CON);						// Sn_IR的CON位置1，通知W5500连接已建立
					}
					// 接收数据的数量
					len=getSn_RX_RSR(i);							// 读取W5500空闲接收缓存寄存器的值并赋给len，Sn_RX_RSR表示接收缓存中已接收和保存的数据大小
					if(len>0)
					{
						recv(i,gDATABUF,len);						// W5500接收来自客户端的数据，并通过SPI发送给MCU
						buff_write(&sock_buff[i], gDATABUF, len);
						len = 0;
					}
					// 发送数据的数量
					if(sock_fb_buff_len[i] > 0)
					{
						send(i, sock_fb_buff[i], sock_fb_buff_len[i]);
						sock_fb_buff_len[i] = 0;
					}
					break;
				case SOCK_CLOSE_WAIT:								// Socket处于等待关闭状态
					close(i);
					break;
				case SOCK_CLOSED:
					socket(i,Sn_MR_TCP,local_port,Sn_MR_ND);		// 打开Socketi，并配置为TCP无延时模式，打开一个本地端口
					break;
			}
			i = (i + 1) % SOCK_QTY_MAX;
			uint8_t PHY_STA = 0;
			PHY_STA = getPHYCFGR();
			if((PHY_STA & 0x01) == 0)
			{
				i = 0;
				printf("Cable disconnected.\r\n");
				break;
			}
			TCP_Delay(10);
		}
	}
}

/**
  * @brief  Intialize the network information to be used in WIZCHIP
  * @retval None
  */
void network_init(void)
{
	uint8_t tmpstr[6];
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
	ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);
	printf("\r\n=== %s NET CONF ===\r\n",(char*)tmpstr);
	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],
		  gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
	printf("IP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
	printf("======================\r\n");
}





