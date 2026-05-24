#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdio.h>
#include <string.h>


typedef struct __buff_class
{
	volatile unsigned long	wrCnt;
	volatile unsigned long	rdCnt;
	volatile unsigned long	size;
	volatile unsigned char	*payload;
	volatile unsigned char	isWrBusy;
	volatile unsigned char	isRdBusy;
	volatile unsigned char	isReady;		// "1" is ready
}buff_class;

void buff_init(buff_class *buffer, unsigned long size, void* payload);
//buff_class* buff_new(unsigned long size);
void buff_clear(buff_class *buffer);
unsigned long buff_write(buff_class *buffer, const void* buf, unsigned long length);
unsigned long buff_read(buff_class *buffer, void* buf, unsigned long length);
unsigned long buff_preview(buff_class *buffer, void* buf, unsigned long length);
int buff_findKeyWord(buff_class *buffer, void *pKeyWords, unsigned long length);
unsigned long buff_move(buff_class *bufSrc, buff_class* buf, unsigned long length);
unsigned long buff_getUsed(const buff_class *buffer);

#endif /* BUFFER_H_ */
