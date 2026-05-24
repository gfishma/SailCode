#include "buffer.h"

/**
 * 获得队列使用量
 * @param	buffer
 * @return	返回已使用量
 */
unsigned long buff_getUsed(const buff_class *buffer)
{
    int len = buffer->wrCnt - buffer->rdCnt;

    if(len >= 0)
        return (unsigned long)len;
    else
        return (buffer->size + len);
}

/**
 * 获得队列空余空间
 * @param	buffer
 * @return	队列空余数
 */
unsigned long buff_getFree(const buff_class *buffer)
{
    return buffer->size - buff_getUsed(buffer) - 1;
}

/**
 * 检测队列是否满
 * @param	buffer
 * @return	0:满    -1:非满
 */
int buff_isFull(const buff_class *buffer)
{
    if(buffer->rdCnt == (buffer->wrCnt + 1) % buffer->size)
        return 0;
    else
        return -1;
}

/**
 * 检测队列是否为空
 * @param	buffer
 * @return	0:空    -1:非空
 */
int buff_isEmpty(const buff_class *buffer)
{
    if(buffer->rdCnt == buffer->wrCnt)
        return 0;
    else
        return -1;
}

/**
 * 等待队列写时忙状态结束
 * @param	buffer
 * @return	0:闲    -1:超时
 */
int buff_waitWrBusy(const buff_class *buffer)
{
	unsigned long t = 0;
	while(buffer->isWrBusy == 1)
	{
		if(t++ > 0xffffff)
			return -1;
	}
	return 0;
}

/**
 * 等待队列读时忙状态结束
 * @param	buffer
 * @return	0:闲    -1:超时
 */
int buff_waitRdBusy(const buff_class *buffer)
{
	unsigned long t = 0;
	while(buffer->isRdBusy == 1)
	{
		if(t++ > 0xffffff)
			return -1;
	}
	return 0;
}

/**
 * 数据入队
 * @param  buffer
 * @param  buf    要入队的数据
 * @param  length 要入队的数据长度
 * @return        返回入队的字节数
 */
unsigned long buff_write(buff_class *buffer, const void* buf, unsigned long length)
{
    unsigned char *bufPtr = (unsigned char *)buf;
    unsigned long offset = 0;
    unsigned long nFree = 0;
//    unsigned long nWrite = 0;

    /* 传入的数据长度为0, 直接返回 */
    if(!length)
		return 0;

    /* 队列没有空间, 直接返回 */
    if((nFree = buff_getFree(buffer)) == 0)
		return 0;

    /* 若列队没有足够空间，直接返回 */
	if(nFree < length)
		return 0;
	
	/* 若等待闲超时，直接返回 */
	if(buff_waitWrBusy(buffer) == -1)
		return 0;
	
	buffer->isWrBusy = 1;
//	nWrite = length;
//    nWrite = nFree>=length ? length : nFree;

    /* 判断队列是否跨尾 */
    offset = buffer->size - buffer->wrCnt;
    if (offset >= length)
    {
        memcpy((uint8_t *)buffer->payload + buffer->wrCnt, bufPtr, length);
        buffer->wrCnt += length;
    }
    else
    {
        memcpy((uint8_t *)buffer->payload + buffer->wrCnt, bufPtr, offset);
        memcpy((uint8_t *)buffer->payload, (uint8_t *)bufPtr + offset, length - offset);
        buffer->wrCnt = length - offset;
    }
	
	buffer->isWrBusy = 0;
    return length;
}

/**
 * 数据出队
 * @param  buffer
 * @param  buf    存放出队的数据
 * @param  length 出队的数据长度
 * @return        返回出队字节数
 */
unsigned long buff_read(buff_class *buffer, void* buf, unsigned long length)
{
    unsigned char *bufPtr = (unsigned char *)buf;
    unsigned long offset = 0;
    unsigned long nUsed = 0;
//    unsigned long nRead = 0;

    /* 出队数据长度为0, 直接返回 */
    if(!length)
		return 0;

    /* 计算实际能够出队的数据长度 */
    if((nUsed = buff_getUsed(buffer)) == 0)
		return 0;

    /* 若列队没有足够可读数量，直接返回 */
	if(nUsed < length)
		return 0;
	
	/* 若等待闲超时，直接返回 */
	if(buff_waitRdBusy(buffer) == -1)
		return 0;
	
	buffer->isRdBusy = 1;
//	nRead = length;

    /* 判断要读的数据是否跨尾 */
    offset = buffer->size - buffer->rdCnt;
    if( offset >= length)
    {
        memcpy((void *)bufPtr, (const void *)(buffer->payload + buffer->rdCnt), length);
        buffer->rdCnt += length;
    }
    else
    {
        memcpy((void *)bufPtr, (const void *)(buffer->payload + buffer->rdCnt), offset);
        memcpy((void *)(bufPtr + offset), (const void *)(buffer->payload), length - offset);
        buffer->rdCnt = length - offset;
    }

	buffer->isRdBusy = 0;
    return length;
}

/**
 * 数据转移，将bufferSrc的数据转移到buffer上
 * @param  bufSrc	源Buffer
 * @param  buf    	Buffer
 * @param  length 	出队的数据长度
 * @return        	转移字节数
 */
unsigned long buff_move(buff_class *bufSrc, buff_class* buf, unsigned long length)
{
	unsigned char bufTmp[1];
    unsigned long nUsed = 0;
	unsigned short i;

    /* 出队数据长度为0, 直接返回 */
    if(!length)
		return 0;

    /* 计算实际能够出队的数据长度 */
    if((nUsed = buff_getUsed(bufSrc)) == 0)
		return 0;

    /* 若列队没有足够空间，直接返回 */
	if(nUsed < length)
		return 0;
	
	for(i = 0; i < length; i++)
	{
		buff_read(bufSrc, bufTmp, 1);
		buff_write(buf, bufTmp, 1);
	}
    return i;
}

/**
 * 预览数据
 * @param  buffer
 * @param  buf    存放预览的数据
 * @param  length 预览的数据长度
 * @return        返回预览字节数
 */
unsigned long buff_preview(buff_class *buffer, void* buf, unsigned long length)
{
    unsigned char *bufPtr = (unsigned char *)buf;
    unsigned long offset = 0;
    unsigned long nUsed = 0;

    /* 出队数据长度为0, 直接返回 */
    if(!length)
		return 0;

    /* 计算实际能够出队的数据长度 */
    if((nUsed = buff_getUsed(buffer)) == 0)
		return 0;

    /* 若列队没有足够空间，直接返回 */
	if(nUsed < length)
		return 0;
	
	/* 若等待闲超时，直接返回 */
	if(buff_waitRdBusy(buffer) == -1)
		return 0;
	
	buffer->isRdBusy = 1;
//	nRead = length;

    /* 判断要读的数据是否跨尾 */
    offset = buffer->size - buffer->rdCnt;
    if( offset >= length)
    {
        memcpy((void *)bufPtr, (const void *)(buffer->payload + buffer->rdCnt), length);
//        buffer->rdCnt += nRead;
    }
    else
    {
        memcpy((void *)bufPtr, (const void *)(buffer->payload + buffer->rdCnt), offset);
        memcpy((void *)(bufPtr + offset), (const void *)(buffer->payload), length - offset);
//        buffer->rdCnt = nRead - offset;
    }

	buffer->isRdBusy = 0;
    return length;
}


//1:BufStruct; 2:ptr of KeyWords; 3:KeyWord Wide
//Return: -1 for can not find the keyword or error, return byte count is offset from RdCnt;
//this function will find the key words from the effect byte.
//the max byte count it compare will not exceed the byte count that it storage.
int buff_findKeyWord(buff_class *buffer, void *pKeyWords, unsigned long length)
{
    unsigned char *pKeyWordsPtr = (unsigned char *)pKeyWords;
    unsigned long offset = 0;
    unsigned long nUsed = 0;
    unsigned long nIndex = 0;
    unsigned long realIndex = 0;
	unsigned long nMatch = 0;

    /* 出队数据长度为0, 直接返回 */
    if(!length)
		return -1;

    /* 计算实际能够出队的数据长度 */
    if((nUsed = buff_getUsed(buffer)) == 0)
		return -1;

    /* 若列队没有足够空间，直接返回 */
	if(nUsed < length)
		return -1;
	
	/* 若等待闲超时，直接返回 */
	if(buff_waitRdBusy(buffer) == -1)
		return -1;
	
//	buffer->isRdBusy = 1;
//	nMatch = length;
	
	if(nUsed >= length)//有效字节大于要查找的字节数
	{
		while(nIndex <= (nUsed-length))
		{
			while(nMatch < length)
			{
				realIndex = (buffer->rdCnt + nIndex + nMatch) % (buffer->size);
				if((buffer->payload[realIndex]) == pKeyWordsPtr[nMatch])	// Match one byte
				{
					nMatch++;
				}
				else
				{
					nMatch = 0;
					break;
				}
			}
			if(nMatch == length)//Match all, return the Byte count and Quit
			{
				realIndex = (buffer->rdCnt + nIndex + nMatch) % (buffer->size);

				if(realIndex >= buffer->rdCnt)
				{
					offset = (realIndex - buffer->rdCnt);
				}
				else
				{
					offset = buffer->size - buffer->rdCnt + realIndex;
				}
				return offset;
			}
			else//Continue to find
			{
				nIndex ++;
			}
		}
	}//if not find the keywords, will return -1;
	return -1;
}

/**
 * 清理队列
 * @param buffer
 */
void buff_clear(buff_class *buffer)
{
    buffer->rdCnt = buffer->wrCnt;
}

/**
 * 初始化一个队列
 * @param buffer
 * @param size 队列大小
 * @param payload 队列缓存地址
 */
void buff_init(buff_class *buffer, unsigned long size, void* payload)
{
	buffer->payload = (unsigned char *)payload;
	buffer->rdCnt = 0;
	buffer->wrCnt = 0;
	buffer->size = size;
	buffer->isWrBusy = 0;
	buffer->isRdBusy = 0;
	buffer->isReady = 1;
}

///**
// * 动态创建一个队列
// * @param  size 队列大小
// * @return      成功返回队列对象指针, 失败返回NULL
// */
//buff_class* buff_new(unsigned long size)
//{
//	buff_class* buffer = NULL;
//	if((buffer = (buff_class *)iram_malloc(sizeof(buff_class))) == NULL)
//		return NULL;
//	buffer->payload = NULL;
//	if((buffer->payload = (unsigned char *)iram_malloc(size)) == NULL)
//	{
//		iram_free(buffer);
//		return NULL;
//	}
//	buffer->rdCnt = 0;
//	buffer->wrCnt = 0;
//	buffer->size = size;
//	buffer->isWrBusy = 0;
//	buffer->isRdBusy = 0;
//	buffer->isReady = 1;
//
//	return buffer;
//}

///**
// * 对于动态创建的队列进行清理工作
// * @param buffer
// */
//void buff_free(buff_class *buffer)
//{
//	iram_free(buffer->payload);
//	iram_free(buffer);
//    buffer = NULL;
//}

