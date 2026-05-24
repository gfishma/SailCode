/*
 * app.c
 *
 *  Created on: 2020年7月6日
 *      Author: timecy
 */


#include <scmd.h>

extern int32_t send(uint8_t sn, uint8_t * buf, uint16_t len);
extern scmd_class scmd_ctrl;

extern uint16_t sock_fb_buff_len[8];
extern uint8_t  sock_fb_buff[8][2048];

int scmd_callback(char* pData, unsigned short len)
{
	pData[len] = 0;
	if(scmd_ctrl.msgSource == 1)
	{
		printf("%.*s\r\n", len, pData);
	}
	else if((scmd_ctrl.msgSource >= 2) && (scmd_ctrl.msgSource <=9))
	{
		uint16_t i;
		sock_fb_buff_len[scmd_ctrl.msgSource - 2] = len;
		for(i = 0; i < len; i++)
			sock_fb_buff[scmd_ctrl.msgSource - 2][i] = pData[i];
		sock_fb_buff[scmd_ctrl.msgSource - 2][i] = 0;
	}
	return 0;
}
