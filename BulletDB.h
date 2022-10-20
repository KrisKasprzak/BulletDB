// todo document everything....





/*
  The MIT License (MIT)

  library writen by Kris Kasprzak
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  On a personal note, if you develop an application or product using this library 
  and make millions of dollars, I'm happy for you!

	rev		date			author				change
	1.0		10/2022			kasprzak			initial code



*/

#ifndef BULLETDB_MENU_H
#define BULLETDB_MENU_H

#if ARDUINO >= 100
	 #include "Arduino.h"
	 #include "Print.h"
#else
	
#endif

#ifdef __cplusplus
	
#endif

#include <SPI.h>  

#define MAX_FIELDS 20
#define MAX_CHAR_LEN 30

#define CARD_SIZE 8384511
#define PAGE_SIZE 4096

#define SPEED_WRITE 25000000
#define SPEED_READ 25000000

#define DT_U8 	0
#define DT_INT 	1
#define DT_I16 	2
#define DT_U16 	3
#define DT_I32 	5
#define DT_U32 	6
#define DT_FLOAT 7
	

#define PAGE 4096
#define PAGES 2048
#define STAT_WIP 1
#define STAT_WEL 2
#define CMD_WRITE_STATUS_REG   0x01
#define CMD_PAGE_PROGRAM       0x02
#define CMD_READ_DATA          0x03
#define CMD_WRITE_DISABLE      0x04//not tested
#define CMD_READ_STATUS_REG    0x05
#define CMD_WRITE_ENABLE       0x06
#define CMD_READ_HIGH_SPEED    0x0B//not tested
#define CMD_SECTOR_ERASE       0x20//not tested
#define CMD_BLOCK32K_ERASE     0x52//not tested
#define CMD_RESET_DEVICE       0xF0//<<-different from winbond
#define CMD_READ_ID            0x9F
#define CMD_RELEASE_POWER_DOWN 0xAB//not tested
#define CMD_POWER_DOWN         0xB9//not tested
#define CMD_CHIP_ERASE         0xC7
#define CMD_BLOCK64K_ERASE     0xD8//not tested
#define WRITEENABLE   0x06
#define WRITE         0x02
#define READ          0x03
#define RID           0xAB
#define CHIPERASE     0x60
#define COMPLETE      0x05
#define RECOVERY 2


class  BulletDB {
		
public:

	BulletDB(int CS_PIN);
	
	bool init();
	
	bool getChipID();
	
	uint8_t addField(const char *FieldName, uint8_t *Data);
	
	uint8_t addField(const char *FieldName, int *Data);
	
	uint8_t addField(const char *FieldName, int16_t *Data);
	
	uint8_t addField(const char *FieldName, uint16_t *Data);
	
	uint8_t addField(const char *FieldName, uint32_t *Data);
	
	uint8_t addField(const char *FieldName, int32_t *Data);
	
	uint8_t addField(const char *FieldName, float *Data);
	
	// uint8_t addField(const char *FieldName, char *Data);
	
	uint32_t getAddress();
	
	void setAddress(uint32_t Address);
	
	void gotoRecord(uint32_t Record);
	
	void gotoAbsoluteFirstRecord();
	
	uint16_t getLastRecord();
	
	uint32_t getRecords();
	
	uint32_t getCurrentRecord();
	
	uint32_t getTotalRecords();
	
	void eraseAll();

	void erasePage(uint32_t PageNumber);

	// uint32_t closeRecord();

	uint32_t addRecord();

	uint32_t saveRecord();

	void listFields();

	uint8_t getFieldCount();

	char * getFieldName(uint8_t Index);

	uint8_t getRecordLength();

	uint32_t getUsedSpace();
	
	uint32_t getTotalSpace();	
	
	void dumpData();
	
	// goofy but with all these data types return a float for now
	// todo make load funcations to return correct data type
	float getField(uint32_t Record, uint8_t Field);
	
	void writeSpecialField(uint32_t AddressOffset, uint8_t Data);
	void writeSpecialField(uint32_t AddressOffset, int Data);
	void writeSpecialField(uint32_t AddressOffset, int16_t Data);
	void writeSpecialField(uint32_t AddressOffset, uint16_t Data);
	void writeSpecialField(uint32_t AddressOffset, int32_t Data);
	void writeSpecialField(uint32_t AddressOffset, uint32_t Data);
	void writeSpecialField(uint32_t AddressOffset, float Data);
	//void writeSpecialField(uint32_t AddressOffset, char Data);
	

	uint8_t getSpecialField(uint32_t Record, uint32_t AddressOffset, uint8_t Data);
	int getSpecialField(uint32_t Record, uint32_t AddressOffset, int Data);
	int16_t getSpecialField(uint32_t Record, uint32_t AddressOffset, int16_t Data);
	uint16_t getSpecialField(uint32_t Record, uint32_t AddressOffset, uint16_t Data);
	int32_t getSpecialField(uint32_t Record, uint32_t AddressOffset, int32_t Data);
	uint32_t getSpecialField(uint32_t Record, uint32_t AddressOffset, uint32_t Data);
	float getSpecialField(uint32_t Record, uint32_t AddressOffset, float Data);
	//char getSpecialField(uint32_t Record, uint32_t AddressOffset, char Data);
	
	
	
private:

	void DebugData(int Line);
		uint8_t ReadData();
	void WriteData(uint8_t data);
	
	uint8_t a1Byte;
	uint8_t a2Bytes[2];
	uint8_t a4Bytes[4];
	char dateBytes[8];
	bool Special = false;
	uint32_t TempAddress = 0;
	uint32_t Address = 0;
	uint32_t RecordID = 0;
	uint32_t Records = 0;
	uint32_t CurrentRecord = 0;
	uint32_t TotalRecords = 0;
	uint32_t i;
	char fieldname[MAX_FIELDS][MAX_CHAR_LEN];
	uint8_t FieldCount = 0;
	uint8_t cspin;
	uint8_t RecordLength;
	int16_t pagesize;

	uint8_t DataType[MAX_FIELDS];
	uint8_t FieldStart[MAX_FIELDS];
	uint8_t FieldLength[MAX_FIELDS];
	
	uint8_t *u8data[MAX_FIELDS];
	int *intdata[MAX_FIELDS];
	int16_t *i16data[MAX_FIELDS];
	uint16_t *u16data[MAX_FIELDS];
	int32_t *i32data[MAX_FIELDS];
	uint32_t *u32data[MAX_FIELDS];
	float *fdata[MAX_FIELDS];
	
	uint8_t bytes2[2];
	uint8_t bytes4[4];
	uint8_t bytes8[4];
	uint8_t bytesc[];
	unsigned char flash_wait_for_write = 0;
	uint8_t readvalue;
	void write_pause(void);
	unsigned char flash_read_status(void);
	
	void B2ToBytes(uint8_t *bytes, int16_t var);
	void B2ToBytes(uint8_t *bytes, uint16_t var);
	void B4ToBytes(uint8_t *bytes, int var);
	void B4ToBytes(uint8_t *bytes, int32_t var);
	void B4ToBytes(uint8_t *bytes, uint32_t var);
	void FloatToBytes(uint8_t *bytes_temp, float float_variable);
	

};



#endif