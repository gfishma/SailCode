/*
 * w25qxx.c
 *
 *  Created on: May 27, 2020
 *      Author: timecy
 */

#include "w25qxx.h"
//#include "w25qxxConf.h"

#if (_W25QXX_DEBUG==1)
#include <stdio.h>
#endif

#define W25QXX_DUMMY_BYTE         0xA5

//w25qxx_class	w25qxx;

#if (_W25QXX_USE_FREERTOS==1)
#define	W25qxx_Delay(delay)		osDelay(delay)
#include "cmsis_os.h"

#else
#define	W25qxx_Delay(delay)		HAL_Delay(delay)
#endif

//###################################################################################################################
uint8_t	W25qxx_Spi(w25qxx_class *self, uint8_t Data)
{
	uint8_t	ret;
	spi_dev_transfer(&self->spi_dev, 1, &Data, &ret);
	return ret;
}
//###################################################################################################################
uint32_t W25qxx_ReadID(w25qxx_class *self)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);

	W25qxx_Spi(self, 0x9F);
	Temp0 = W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
	Temp1 = W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
	Temp2 = W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);

	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
	return Temp;
}
//###################################################################################################################
void W25qxx_ReadUniqID(w25qxx_class *self)
{
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0x4B);
	for(uint8_t	i=0;i<4;i++)
		W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
	for(uint8_t	i=0;i<8;i++)
		self->UniqID[i] = W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
}
//###################################################################################################################
void W25qxx_WriteEnable(w25qxx_class *self)
{
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0x06);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
	W25qxx_Delay(1);
}
//###################################################################################################################
void W25qxx_WriteDisable(w25qxx_class *self)
{
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0x04);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
	W25qxx_Delay(1);
}
//###################################################################################################################
uint8_t W25qxx_ReadStatusRegister(w25qxx_class *self, uint8_t SelectStatusRegister_1_2_3)
{
	uint8_t	status=0;
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	if(SelectStatusRegister_1_2_3==1)
	{
		W25qxx_Spi(self, 0x05);
		status=W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
		self->StatusRegister1 = status;
//		self->StatusRegister1 = status;
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
		W25qxx_Spi(self, 0x35);
		status=W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
		self->StatusRegister2 = status;
//		self->StatusRegister2 = status;
	}
	else
	{
		W25qxx_Spi(self, 0x15);
		status=W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
		self->StatusRegister3 = status;
//		self->StatusRegister3 = status;
	}
  gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
	return status;
}
//###################################################################################################################
void W25qxx_WriteStatusRegister(w25qxx_class *self, uint8_t	SelectStatusRegister_1_2_3,uint8_t Data)
{
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	if(SelectStatusRegister_1_2_3==1)
	{
		W25qxx_Spi(self, 0x01);
		self->StatusRegister1 = Data;
//		self->StatusRegister1 = Data;
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
		W25qxx_Spi(self, 0x31);
		self->StatusRegister2 = Data;
//		self->StatusRegister2 = Data;
	}
	else
	{
		W25qxx_Spi(self, 0x11);
		self->StatusRegister3 = Data;
//		self->StatusRegister3 = Data;
	}
	W25qxx_Spi(self, Data);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
}
//###################################################################################################################
void W25qxx_WaitForWriteEnd(w25qxx_class *self)
{
	W25qxx_Delay(1);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0x05);
	do
	{
		self->StatusRegister1 = W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
		W25qxx_Delay(1);
	}
	while ((self->StatusRegister1 & 0x01) == 0x01);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
//  while ((self->StatusRegister1 & 0x01) == 0x01);
// HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
}
//###################################################################################################################
bool W25qxx_Init(w25qxx_class* self)
{
	self->Lock = 1;
//	self->Lock=1;
	while(HAL_GetTick()<100)
		W25qxx_Delay(1);
	spi_dev_config(&self->spi_dev, spi_cpol_low, spi_cpha_1edge);
	W25qxx_Delay(100);
	uint32_t	id;
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx Init Begin...\r\n");
	#endif
	id=W25qxx_ReadID(self);

	#if (_W25QXX_DEBUG==1)
	printf("w25qxx ID:0x%X\r\n",id);
	#endif
	switch(id&0x0000FFFF)
	{
		case 0x401A:	// 	w25q512
			self->ID = W25Q512;
			self->BlockCount = 1024;
//			self->ID=W25Q512;
//			self->BlockCount=1024;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q512\r\n");
			#endif
		break;
		case 0x4019:	// 	w25q256
			self->ID = W25Q256;
			self->BlockCount = 512;
//			self->ID=W25Q256;
//			self->BlockCount=512;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q256\r\n");
			#endif
		break;
		case 0x6018:	// 	w25q128
		case 0x4018:	// 	w25q128
			self->ID = W25Q128;
			self->BlockCount = 256;
//			self->ID=W25Q128;
//			self->BlockCount=256;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q128\r\n");
			#endif
		break;
		case 0x4017:	//	w25q64
			self->ID=W25Q64;
			self->BlockCount=128;
//			self->ID=W25Q64;
//			self->BlockCount=128;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q64\r\n");
			#endif
		break;
		case 0x4016:	//	w25q32
			self->ID=W25Q32;
			self->BlockCount=64;
//			self->ID=W25Q32;
//			self->BlockCount=64;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q32\r\n");
			#endif
		break;
		case 0x4015:	//	w25q16
			self->ID=W25Q16;
			self->BlockCount=32;
//			self->ID=W25Q16;
//			self->BlockCount=32;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q16\r\n");
			#endif
		break;
		case 0x4014:	//	w25q80
			self->ID=W25Q80;
			self->BlockCount=16;
//			self->ID=W25Q80;
//			self->BlockCount=16;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q80\r\n");
			#endif
		break;
		case 0x4013:	//	w25q40
			self->ID=W25Q40;
			self->BlockCount=8;
//			self->ID=W25Q40;
//			self->BlockCount=8;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q40\r\n");
			#endif
		break;
		case 0x4012:	//	w25q20
			self->ID=W25Q20;
			self->BlockCount=4;
//			self->ID=W25Q20;
//			self->BlockCount=4;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q20\r\n");
			#endif
		break;
		case 0x4011:	//	w25q10
			self->ID=W25Q10;
			self->BlockCount=2;
//			self->ID=W25Q10;
//			self->BlockCount=2;
			#if (_W25QXX_DEBUG==1)
			printf("w25qxx Chip: w25q10\r\n");
			#endif
		break;
		default:
				#if (_W25QXX_DEBUG==1)
				printf("w25qxx Unknown ID\r\n");
				#endif
				self->Lock=0;
//				self->Lock=0;
			return false;

	}
	self->PageSize=256;
	self->SectorSize=0x1000;
	self->SectorCount=self->BlockCount*16;
	self->PageCount=(self->SectorCount*self->SectorSize)/self->PageSize;
	self->BlockSize=self->SectorSize*16;
	self->CapacityInKiloByte=(self->SectorCount*self->SectorSize)/1024;
	W25qxx_ReadUniqID(self);
	W25qxx_ReadStatusRegister(self, 1);
	W25qxx_ReadStatusRegister(self, 2);
	W25qxx_ReadStatusRegister(self, 3);
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx Page Size: %d Bytes\r\n",self->PageSize);
	printf("w25qxx Page Count: %d\r\n",self->PageCount);
	printf("w25qxx Sector Size: %d Bytes\r\n",self->SectorSize);
	printf("w25qxx Sector Count: %d\r\n",self->SectorCount);
	printf("w25qxx Block Size: %d Bytes\r\n",self->BlockSize);
	printf("w25qxx Block Count: %d\r\n",self->BlockCount);
	printf("w25qxx Capacity: %d KiloBytes\r\n",self->CapacityInKiloByte);
	printf("w25qxx Init Done\r\n");
	#endif
	self->Lock=0;
	return true;
}
//###################################################################################################################
void W25qxx_EraseChip(w25qxx_class *self)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	printf("w25qxx EraseChip Begin...\r\n");
	#endif
	W25qxx_WriteEnable(self);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0xC7);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd(self);
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseBlock done after %d ms!\r\n",HAL_GetTick()-StartTime);
	#endif
	W25qxx_Delay(10);
	self->Lock=0;
}
//###################################################################################################################
void W25qxx_EraseSector(w25qxx_class *self, uint32_t SectorAddr)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	printf("w25qxx EraseSector %d Begin...\r\n",SectorAddr);
	#endif
	W25qxx_WaitForWriteEnd(self);
	SectorAddr = SectorAddr * self->SectorSize;
  W25qxx_WriteEnable(self);
gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
  W25qxx_Spi(self, 0x20);
	if(self->ID>=W25Q256)
		W25qxx_Spi(self, (SectorAddr & 0xFF000000) >> 24);
  W25qxx_Spi(self, (SectorAddr & 0xFF0000) >> 16);
  W25qxx_Spi(self, (SectorAddr & 0xFF00) >> 8);
  W25qxx_Spi(self, SectorAddr & 0xFF);
gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
  W25qxx_WaitForWriteEnd(self);
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseSector done after %d ms\r\n",HAL_GetTick()-StartTime);
	#endif
	W25qxx_Delay(1);
	self->Lock=0;
}
//###################################################################################################################
void W25qxx_EraseBlock(w25qxx_class *self, uint32_t BlockAddr)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseBlock %d Begin...\r\n",BlockAddr);
	W25qxx_Delay(100);
	uint32_t	StartTime=HAL_GetTick();
	#endif
	W25qxx_WaitForWriteEnd(self);
	BlockAddr = BlockAddr * self->SectorSize*16;
  W25qxx_WriteEnable(self);
gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
  W25qxx_Spi(self, 0xD8);
	if(self->ID>=W25Q256)
		W25qxx_Spi(self, (BlockAddr & 0xFF000000) >> 24);
  W25qxx_Spi(self, (BlockAddr & 0xFF0000) >> 16);
  W25qxx_Spi(self, (BlockAddr & 0xFF00) >> 8);
  W25qxx_Spi(self, BlockAddr & 0xFF);
gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
  W25qxx_WaitForWriteEnd(self);
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx EraseBlock done after %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	W25qxx_Delay(1);
	self->Lock=0;
}
//###################################################################################################################
uint32_t	W25qxx_PageToSector(w25qxx_class *self, uint32_t	PageAddress)
{
	return ((PageAddress*self->PageSize)/self->SectorSize);
}
//###################################################################################################################
uint32_t	W25qxx_PageToBlock(w25qxx_class *self, uint32_t	PageAddress)
{
	return ((PageAddress*self->PageSize)/self->BlockSize);
}
//###################################################################################################################
uint32_t	W25qxx_SectorToBlock(w25qxx_class *self, uint32_t	SectorAddress)
{
	return ((SectorAddress*self->SectorSize)/self->BlockSize);
}
//###################################################################################################################
uint32_t	W25qxx_SectorToPage(w25qxx_class *self, uint32_t	SectorAddress)
{
	return (SectorAddress*self->SectorSize)/self->PageSize;
}
//###################################################################################################################
uint32_t	W25qxx_BlockToPage(w25qxx_class *self, uint32_t	BlockAddress)
{
	return (BlockAddress*self->BlockSize)/self->PageSize;
}
//###################################################################################################################
bool 	W25qxx_IsEmptyPage(w25qxx_class *self, uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_PageSize)
{
	while(self->Lock==1)
	W25qxx_Delay(1);
	self->Lock=1;
	if(((NumByteToCheck_up_to_PageSize+OffsetInByte)>self->PageSize)||(NumByteToCheck_up_to_PageSize==0))
		NumByteToCheck_up_to_PageSize=self->PageSize-OffsetInByte;
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckPage:%d, Offset:%d, Bytes:%d begin...\r\n",Page_Address,OffsetInByte,NumByteToCheck_up_to_PageSize);
	W25qxx_Delay(100);
	uint32_t	StartTime=HAL_GetTick();
	#endif
	uint8_t	pBuffer[32];
	uint32_t	WorkAddress;
	uint32_t	i;
	for(i=OffsetInByte; i<self->PageSize; i+=sizeof(pBuffer))
	{
//		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		WorkAddress=(i+Page_Address*self->PageSize);
		W25qxx_Spi(self, 0x0B);
		if(self->ID>=W25Q256)
			W25qxx_Spi(self, (WorkAddress & 0xFF000000) >> 24);
		W25qxx_Spi(self, (WorkAddress & 0xFF0000) >> 16);
		W25qxx_Spi(self, (WorkAddress & 0xFF00) >> 8);
		W25qxx_Spi(self, WorkAddress & 0xFF);
		W25qxx_Spi(self, 0);
		spi_dev_read(&self->spi_dev, sizeof(pBuffer), pBuffer);
//		HAL_SPI_Receive(&_W25QXX_SPI,pBuffer, sizeof(pBuffer), 100);
//		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
		for(uint8_t x=0;x<sizeof(pBuffer);x++)
		{
			if(pBuffer[x]!=0xFF)
				goto NOT_EMPTY;
		}
	}
	if((self->PageSize+OffsetInByte)%sizeof(pBuffer)!=0)
	{
		i-=sizeof(pBuffer);
		for( ; i<self->PageSize; i++)
		{
//			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
			WorkAddress=(i+Page_Address*self->PageSize);
			W25qxx_Spi(self, 0x0B);
			if(self->ID>=W25Q256)
				W25qxx_Spi(self, (WorkAddress & 0xFF000000) >> 24);
			W25qxx_Spi(self, (WorkAddress & 0xFF0000) >> 16);
			W25qxx_Spi(self, (WorkAddress & 0xFF00) >> 8);
			W25qxx_Spi(self, WorkAddress & 0xFF);
			W25qxx_Spi(self, 0);

			spi_dev_read(&self->spi_dev, 1, pBuffer);
//			HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,1,100);
//			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
			if(pBuffer[0]!=0xFF)
				goto NOT_EMPTY;
		}
	}
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckPage is Empty in %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	self->Lock=0;
	return true;
	NOT_EMPTY:
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckPage is Not Empty in %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	self->Lock=0;
	return false;
}
//###################################################################################################################
bool 	W25qxx_IsEmptySector(w25qxx_class *self, uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_SectorSize)
{
	while(self->Lock==1)
	W25qxx_Delay(1);
	self->Lock=1;
	if((NumByteToCheck_up_to_SectorSize>self->SectorSize)||(NumByteToCheck_up_to_SectorSize==0))
		NumByteToCheck_up_to_SectorSize=self->SectorSize;
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n",Sector_Address,OffsetInByte,NumByteToCheck_up_to_SectorSize);
	W25qxx_Delay(100);
	uint32_t	StartTime=HAL_GetTick();
	#endif
	uint8_t	pBuffer[32];
	uint32_t	WorkAddress;
	uint32_t	i;
	for(i=OffsetInByte; i<self->SectorSize; i+=sizeof(pBuffer))
	{
//		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		WorkAddress=(i+Sector_Address*self->SectorSize);
		W25qxx_Spi(self, 0x0B);
		if(self->ID>=W25Q256)
			W25qxx_Spi(self, (WorkAddress & 0xFF000000) >> 24);
		W25qxx_Spi(self, (WorkAddress & 0xFF0000) >> 16);
		W25qxx_Spi(self, (WorkAddress & 0xFF00) >> 8);
		W25qxx_Spi(self, WorkAddress & 0xFF);
		W25qxx_Spi(self, 0);

		spi_dev_read(&self->spi_dev, sizeof(pBuffer), pBuffer);
//		HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,sizeof(pBuffer),100);
//		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
		for(uint8_t x=0;x<sizeof(pBuffer);x++)
		{
			if(pBuffer[x]!=0xFF)
				goto NOT_EMPTY;
		}
	}
	if((self->SectorSize+OffsetInByte)%sizeof(pBuffer)!=0)
	{
		i-=sizeof(pBuffer);
		for( ; i<self->SectorSize; i++)
		{
//			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
			WorkAddress=(i+Sector_Address*self->SectorSize);
			W25qxx_Spi(self, 0x0B);
			if(self->ID>=W25Q256)
				W25qxx_Spi(self, (WorkAddress & 0xFF000000) >> 24);
			W25qxx_Spi(self, (WorkAddress & 0xFF0000) >> 16);
			W25qxx_Spi(self, (WorkAddress & 0xFF00) >> 8);
			W25qxx_Spi(self, WorkAddress & 0xFF);
			W25qxx_Spi(self, 0);
			spi_dev_read(&self->spi_dev, 1, pBuffer);
//			HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,1,100);
//			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
			if(pBuffer[0]!=0xFF)
				goto NOT_EMPTY;
		}
	}
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckSector is Empty in %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	self->Lock=0;
	return true;
	NOT_EMPTY:
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckSector is Not Empty in %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	self->Lock=0;
	return false;
}
//###################################################################################################################
bool 	W25qxx_IsEmptyBlock(w25qxx_class *self, uint32_t Block_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_BlockSize)
{
	while(self->Lock==1)
	W25qxx_Delay(1);
	self->Lock=1;
	if((NumByteToCheck_up_to_BlockSize>self->BlockSize)||(NumByteToCheck_up_to_BlockSize==0))
		NumByteToCheck_up_to_BlockSize=self->BlockSize;
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n",Block_Address,OffsetInByte,NumByteToCheck_up_to_BlockSize);
	W25qxx_Delay(100);
	uint32_t	StartTime=HAL_GetTick();
	#endif
	uint8_t	pBuffer[32];
	uint32_t	WorkAddress;
	uint32_t	i;
	for(i=OffsetInByte; i<self->BlockSize; i+=sizeof(pBuffer))
	{
//		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		WorkAddress=(i+Block_Address*self->BlockSize);
		W25qxx_Spi(self, 0x0B);
		if(self->ID>=W25Q256)
			W25qxx_Spi(self, (WorkAddress & 0xFF000000) >> 24);
		W25qxx_Spi(self, (WorkAddress & 0xFF0000) >> 16);
		W25qxx_Spi(self, (WorkAddress & 0xFF00) >> 8);
		W25qxx_Spi(self, WorkAddress & 0xFF);
		W25qxx_Spi(self, 0);

		spi_dev_read(&self->spi_dev, sizeof(pBuffer), pBuffer);
//		HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,sizeof(pBuffer),100);
//		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
		for(uint8_t x=0;x<sizeof(pBuffer);x++)
		{
			if(pBuffer[x]!=0xFF)
				goto NOT_EMPTY;
		}
	}
	if((self->BlockSize+OffsetInByte)%sizeof(pBuffer)!=0)
	{
		i-=sizeof(pBuffer);
		for( ; i<self->BlockSize; i++)
		{
//			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
			WorkAddress=(i+Block_Address*self->BlockSize);
			W25qxx_Spi(self, 0x0B);
			if(self->ID>=W25Q256)
				W25qxx_Spi(self, (WorkAddress & 0xFF000000) >> 24);
			W25qxx_Spi(self, (WorkAddress & 0xFF0000) >> 16);
			W25qxx_Spi(self, (WorkAddress & 0xFF00) >> 8);
			W25qxx_Spi(self, WorkAddress & 0xFF);
			W25qxx_Spi(self, 0);
			spi_dev_read(&self->spi_dev, 1, pBuffer);
//			HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,1,100);
//			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
			if(pBuffer[0]!=0xFF)
				goto NOT_EMPTY;
		}
	}
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckBlock is Empty in %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	self->Lock=0;
	return true;
	NOT_EMPTY:
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx CheckBlock is Not Empty in %d ms\r\n",HAL_GetTick()-StartTime);
	W25qxx_Delay(100);
	#endif
	self->Lock=0;
	return false;
}
//###################################################################################################################
void W25qxx_WriteByte(w25qxx_class *self, uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	printf("w25qxx WriteByte 0x%02X at address %d begin...",pBuffer,WriteAddr_inBytes);
	#endif
	W25qxx_WaitForWriteEnd(self);
  W25qxx_WriteEnable(self);
gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
  W25qxx_Spi(self, 0x02);
	if(self->ID>=W25Q256)
		W25qxx_Spi(self, (WriteAddr_inBytes & 0xFF000000) >> 24);
  W25qxx_Spi(self, (WriteAddr_inBytes & 0xFF0000) >> 16);
  W25qxx_Spi(self, (WriteAddr_inBytes & 0xFF00) >> 8);
  W25qxx_Spi(self, WriteAddr_inBytes & 0xFF);
  W25qxx_Spi(self, pBuffer);
gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
  W25qxx_WaitForWriteEnd(self);
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx WriteByte done after %d ms\r\n",HAL_GetTick()-StartTime);
	#endif
	self->Lock=0;
}
//###################################################################################################################
void 	W25qxx_WritePage(w25qxx_class *self, uint8_t *pBuffer	,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToWrite_up_to_PageSize)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	if(((NumByteToWrite_up_to_PageSize+OffsetInByte)>self->PageSize)||(NumByteToWrite_up_to_PageSize==0))
		NumByteToWrite_up_to_PageSize=self->PageSize-OffsetInByte;
	if((OffsetInByte+NumByteToWrite_up_to_PageSize) > self->PageSize)
		NumByteToWrite_up_to_PageSize = self->PageSize-OffsetInByte;
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx WritePage:%d, Offset:%d ,Writes %d Bytes, begin...\r\n",Page_Address,OffsetInByte,NumByteToWrite_up_to_PageSize);
	W25qxx_Delay(100);
	uint32_t	StartTime=HAL_GetTick();
	#endif
	W25qxx_WaitForWriteEnd(self);
  W25qxx_WriteEnable(self);
gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
  W25qxx_Spi(self, 0x02);
	Page_Address = (Page_Address*self->PageSize)+OffsetInByte;
	if(self->ID>=W25Q256)
		W25qxx_Spi(self, (Page_Address & 0xFF000000) >> 24);
  W25qxx_Spi(self, (Page_Address & 0xFF0000) >> 16);
  W25qxx_Spi(self, (Page_Address & 0xFF00) >> 8);
  W25qxx_Spi(self, Page_Address&0xFF);

	spi_dev_write(&self->spi_dev, NumByteToWrite_up_to_PageSize, pBuffer);
//	HAL_SPI_Transmit(&_W25QXX_SPI,pBuffer,NumByteToWrite_up_to_PageSize,100);
gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
  W25qxx_WaitForWriteEnd(self);
	#if (_W25QXX_DEBUG==1)
	StartTime = HAL_GetTick()-StartTime;
	for(uint32_t i=0;i<NumByteToWrite_up_to_PageSize ; i++)
	{
		if((i%8==0)&&(i>2))
		{
			printf("\r\n");
			W25qxx_Delay(10);
		}
		printf("0x%02X,",pBuffer[i]);
	}
	printf("\r\n");
	printf("w25qxx WritePage done after %d ms\r\n",StartTime);
	W25qxx_Delay(100);
	#endif
	W25qxx_Delay(1);
	self->Lock=0;
}
//###################################################################################################################
void 	W25qxx_WriteSector(w25qxx_class *self, uint8_t *pBuffer	,uint32_t Sector_Address,uint32_t OffsetInByte	,uint32_t NumByteToWrite_up_to_SectorSize)
{
	if((NumByteToWrite_up_to_SectorSize>self->SectorSize)||(NumByteToWrite_up_to_SectorSize==0))
		NumByteToWrite_up_to_SectorSize=self->SectorSize;
	#if (_W25QXX_DEBUG==1)
	printf("+++w25qxx WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n",Sector_Address,OffsetInByte,NumByteToWrite_up_to_SectorSize);
	W25qxx_Delay(100);
	#endif
	if(OffsetInByte>=self->SectorSize)
	{
		#if (_W25QXX_DEBUG==1)
		printf("---w25qxx WriteSector Faild!\r\n");
		W25qxx_Delay(100);
		#endif
		return;
	}
	uint32_t	StartPage;
	int32_t		BytesToWrite;
	uint32_t	LocalOffset;
	if((OffsetInByte+NumByteToWrite_up_to_SectorSize) > self->SectorSize)
		BytesToWrite = self->SectorSize-OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_SectorSize;
	StartPage = W25qxx_SectorToPage(self, Sector_Address)+(OffsetInByte/self->PageSize);
	LocalOffset = OffsetInByte%self->PageSize;
	do
	{


		W25qxx_WritePage(self, pBuffer,StartPage,LocalOffset,BytesToWrite);
		StartPage++;
		BytesToWrite-=self->PageSize-LocalOffset;
		pBuffer += self->PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToWrite>0);
	#if (_W25QXX_DEBUG==1)
	printf("---w25qxx WriteSector Done\r\n");
	W25qxx_Delay(100);
	#endif
}
//###################################################################################################################
void 	W25qxx_WriteBlock	(w25qxx_class *self, uint8_t* pBuffer ,uint32_t Block_Address	,uint32_t OffsetInByte	,uint32_t	NumByteToWrite_up_to_BlockSize)
{
	if((NumByteToWrite_up_to_BlockSize>self->BlockSize)||(NumByteToWrite_up_to_BlockSize==0))
		NumByteToWrite_up_to_BlockSize=self->BlockSize;
	#if (_W25QXX_DEBUG==1)
	printf("+++w25qxx WriteBlock:%d, Offset:%d ,Write %d Bytes, begin...\r\n",Block_Address,OffsetInByte,NumByteToWrite_up_to_BlockSize);
	W25qxx_Delay(100);
	#endif
	if(OffsetInByte>=self->BlockSize)
	{
		#if (_W25QXX_DEBUG==1)
		printf("---w25qxx WriteBlock Faild!\r\n");
		W25qxx_Delay(100);
		#endif
		return;
	}
	uint32_t	StartPage;
	int32_t		BytesToWrite;
	uint32_t	LocalOffset;
	if((OffsetInByte+NumByteToWrite_up_to_BlockSize) > self->BlockSize)
		BytesToWrite = self->BlockSize-OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_BlockSize;
	StartPage = W25qxx_BlockToPage(self, Block_Address)+(OffsetInByte/self->PageSize);
	LocalOffset = OffsetInByte%self->PageSize;
	do
	{
		W25qxx_WritePage(self, pBuffer,StartPage,LocalOffset,BytesToWrite);
		StartPage++;
		BytesToWrite-=self->PageSize-LocalOffset;
		pBuffer += self->PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToWrite>0);
	#if (_W25QXX_DEBUG==1)
	printf("---w25qxx WriteBlock Done\r\n");
	W25qxx_Delay(100);
	#endif
}
//###################################################################################################################
void 	W25qxx_ReadByte(w25qxx_class *self, uint8_t *pBuffer,uint32_t Bytes_Address)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	printf("w25qxx ReadByte at address %d begin...\r\n",Bytes_Address);
	#endif
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0x0B);
	if(self->ID>=W25Q256)
		W25qxx_Spi(self, (Bytes_Address & 0xFF000000) >> 24);
	W25qxx_Spi(self, (Bytes_Address & 0xFF0000) >> 16);
	W25qxx_Spi(self, (Bytes_Address& 0xFF00) >> 8);
	W25qxx_Spi(self, Bytes_Address & 0xFF);
	W25qxx_Spi(self, 0);
	*pBuffer = W25qxx_Spi(self, W25QXX_DUMMY_BYTE);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx ReadByte 0x%02X done after %d ms\r\n",*pBuffer,HAL_GetTick()-StartTime);
	#endif
	self->Lock=0;
}
//###################################################################################################################
void W25qxx_ReadBytes(w25qxx_class *self, uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	#if (_W25QXX_DEBUG==1)
	uint32_t	StartTime=HAL_GetTick();
	printf("w25qxx ReadBytes at Address:%d, %d Bytes  begin...\r\n",ReadAddr,NumByteToRead);
	#endif
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0x0B);
	if(self->ID>=W25Q256)
		W25qxx_Spi(self, (ReadAddr & 0xFF000000) >> 24);
  W25qxx_Spi(self, (ReadAddr & 0xFF0000) >> 16);
  W25qxx_Spi(self, (ReadAddr& 0xFF00) >> 8);
  W25qxx_Spi(self, ReadAddr & 0xFF);
	W25qxx_Spi(self, 0);
	spi_dev_read(&self->spi_dev, NumByteToRead, pBuffer);
//	HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,NumByteToRead,2000);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
	#if (_W25QXX_DEBUG==1)
	StartTime = HAL_GetTick()-StartTime;
	for(uint32_t i=0;i<NumByteToRead ; i++)
	{
		if((i%8==0)&&(i>2))
		{
			printf("\r\n");
			W25qxx_Delay(10);
		}
		printf("0x%02X,",pBuffer[i]);
	}
	printf("\r\n");
	printf("w25qxx ReadBytes done after %d ms\r\n",StartTime);
	W25qxx_Delay(100);
	#endif
	W25qxx_Delay(1);
	self->Lock=0;
}
//###################################################################################################################
void 	W25qxx_ReadPage(w25qxx_class *self, uint8_t *pBuffer,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_PageSize)
{
	while(self->Lock==1)
		W25qxx_Delay(1);
	self->Lock=1;
	if((NumByteToRead_up_to_PageSize>self->PageSize)||(NumByteToRead_up_to_PageSize==0))
		NumByteToRead_up_to_PageSize=self->PageSize;
	if((OffsetInByte+NumByteToRead_up_to_PageSize) > self->PageSize)
		NumByteToRead_up_to_PageSize = self->PageSize-OffsetInByte;
	#if (_W25QXX_DEBUG==1)
	printf("w25qxx ReadPage:%d, Offset:%d ,Read %d Bytes, begin...\r\n",Page_Address,OffsetInByte,NumByteToRead_up_to_PageSize);
	W25qxx_Delay(100);
	uint32_t	StartTime=HAL_GetTick();
	#endif
	Page_Address = Page_Address*self->PageSize+OffsetInByte;
	gpio_set(&self->spi_dev.cs, GPIO_PIN_RESET);
	W25qxx_Spi(self, 0x0B);
	if(self->ID>=W25Q256)
		W25qxx_Spi(self, (Page_Address & 0xFF000000) >> 24);
	W25qxx_Spi(self, (Page_Address & 0xFF0000) >> 16);
	W25qxx_Spi(self, (Page_Address& 0xFF00) >> 8);
	W25qxx_Spi(self, Page_Address & 0xFF);
	W25qxx_Spi(self, 0);

	spi_dev_read(&self->spi_dev, NumByteToRead_up_to_PageSize, pBuffer);
//	HAL_SPI_Receive(&_W25QXX_SPI,pBuffer,NumByteToRead_up_to_PageSize,100);
	gpio_set(&self->spi_dev.cs, GPIO_PIN_SET);
	#if (_W25QXX_DEBUG==1)
	StartTime = HAL_GetTick()-StartTime;
	for(uint32_t i=0;i<NumByteToRead_up_to_PageSize ; i++)
	{
		if((i%8==0)&&(i>2))
		{
			printf("\r\n");
			W25qxx_Delay(10);
		}
		printf("0x%02X,",pBuffer[i]);
	}
	printf("\r\n");
	printf("w25qxx ReadPage done after %d ms\r\n",StartTime);
	W25qxx_Delay(100);
	#endif
	W25qxx_Delay(1);
	self->Lock=0;
}
//###################################################################################################################
void 	W25qxx_ReadSector(w25qxx_class *self, uint8_t *pBuffer,uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_SectorSize)
{
	if((NumByteToRead_up_to_SectorSize>self->SectorSize)||(NumByteToRead_up_to_SectorSize==0))
		NumByteToRead_up_to_SectorSize=self->SectorSize;
	#if (_W25QXX_DEBUG==1)
	printf("+++w25qxx ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n",Sector_Address,OffsetInByte,NumByteToRead_up_to_SectorSize);
	W25qxx_Delay(100);
	#endif
	if(OffsetInByte>=self->SectorSize)
	{
		#if (_W25QXX_DEBUG==1)
		printf("---w25qxx ReadSector Faild!\r\n");
		W25qxx_Delay(100);
		#endif
		return;
	}
	uint32_t	StartPage;
	int32_t		BytesToRead;
	uint32_t	LocalOffset;
	if((OffsetInByte+NumByteToRead_up_to_SectorSize) > self->SectorSize)
		BytesToRead = self->SectorSize-OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_SectorSize;
	StartPage = W25qxx_SectorToPage(self, Sector_Address)+(OffsetInByte/self->PageSize);
	LocalOffset = OffsetInByte%self->PageSize;
	do
	{
		W25qxx_ReadPage(self, pBuffer,StartPage,LocalOffset,BytesToRead);
		StartPage++;
		BytesToRead-=self->PageSize-LocalOffset;
		pBuffer += self->PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToRead>0);
	#if (_W25QXX_DEBUG==1)
	printf("---w25qxx ReadSector Done\r\n");
	W25qxx_Delay(100);
	#endif
}
//###################################################################################################################
void 	W25qxx_ReadBlock(w25qxx_class *self, uint8_t* pBuffer,uint32_t Block_Address,uint32_t OffsetInByte,uint32_t	NumByteToRead_up_to_BlockSize)
{
	if((NumByteToRead_up_to_BlockSize>self->BlockSize)||(NumByteToRead_up_to_BlockSize==0))
		NumByteToRead_up_to_BlockSize=self->BlockSize;
	#if (_W25QXX_DEBUG==1)
	printf("+++w25qxx ReadBlock:%d, Offset:%d ,Read %d Bytes, begin...\r\n",Block_Address,OffsetInByte,NumByteToRead_up_to_BlockSize);
	W25qxx_Delay(100);
	#endif
	if(OffsetInByte>=self->BlockSize)
	{
		#if (_W25QXX_DEBUG==1)
		printf("w25qxx ReadBlock Faild!\r\n");
		W25qxx_Delay(100);
		#endif
		return;
	}
	uint32_t	StartPage;
	int32_t		BytesToRead;
	uint32_t	LocalOffset;
	if((OffsetInByte+NumByteToRead_up_to_BlockSize) > self->BlockSize)
		BytesToRead = self->BlockSize-OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_BlockSize;
	StartPage = W25qxx_BlockToPage(self, Block_Address)+(OffsetInByte/self->PageSize);
	LocalOffset = OffsetInByte%self->PageSize;
	do
	{
		W25qxx_ReadPage(self, pBuffer,StartPage,LocalOffset,BytesToRead);
		StartPage++;
		BytesToRead-=self->PageSize-LocalOffset;
		pBuffer += self->PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToRead>0);
	#if (_W25QXX_DEBUG==1)
	printf("---w25qxx ReadBlock Done\r\n");
	W25qxx_Delay(100);
	#endif
}
//###################################################################################################################



