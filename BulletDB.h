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
	1.1		12/2022			kasprzak			added more methods
	1.2		1/2023			kasprzak			added addRecord, moved everything to record/field based
	1.4		10/2023			kasprzak			added getFirstRecord for going to first record in a recordset
	1.5		11/2023			kasprzak			made arguement list for getFirstRecord consistent with getField
	1.6		03/2024			kasprzak			fixed getfield for doubles
	1.8		01/2025			kasprzak			added ability to getDataBaseRecordSize and private put
	2.0		02/2025			kasprzak			changed ReadData to ReadBytes and now read in sequence the field lenght (2x faster)
	3.0		05/2025			kasprzak			changed WriteData to WriteBytes and now read in sequence the field lenght (5x faster), added faster block erase method

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

#define BULLET_DB_VER 3.0

#define NULL_RECORD 0xFF

#define MAX_FIELDS 50
#define MAXCHARLEN 20

#define CARD_SIZE 8388608 // 2 ^ 17

#define PAGE_SIZE 256
#define SECTOR_SIZE 4096
#define BLOCK_SIZE 65536

#define SPEED_WRITE 25000000
#define SPEED_READ  25000000

#define DT_U8 	1
#define DT_INT 	2
#define DT_I16 	3
#define DT_U16 	4
#define DT_I32 	5
#define DT_U32 	6
#define DT_FLOAT 7
#define DT_DOUBLE 8
#define DT_CHAR 9

#define CHIP_NEW 0
#define CHIP_OK 1
#define CHIP_INVALID -1
#define CHIP_FULL -2
#define CHIP_FORCE_RESTART -3

#define CMD_READ_DATA          0x03
#define CMD_READ_ID            0x4B
#define CMD_CHIP_ERASE         0xC7
#define CMD_WRITE_ENABLE       0x06
#define CMD_WRITE_STATUS_REG   0x01
#define CMD_READ_STATUS_REG    0x05
#define CMD_SECTOR_ERASE       0x20
#define STAT_WIP 1

#define CMD_BLOCK64K_ERASE     0xD8 
#define WRITEENABLE   0x06
#define WRITE         0x02
#define READ          0x03
#define READ_FAST     0x0B 
#define RID           0xAB
#define JEDEC         0x9F
#define CHIPERASE     0x60
#define COMPLETE      0x05
#define RECOVERY 2

class  BulletDB {
		
public:

	BulletDB(int CS_PIN);

	bool init();
	
	uint8_t addField(uint8_t *Data);	
	uint8_t addField(int *Data);
	uint8_t addField(int16_t *Data);
	uint8_t addField(uint16_t *Data);
	uint8_t addField(uint32_t *Data);
	uint8_t addField(int32_t *Data);
	uint8_t addField(float *Data);
	uint8_t addField(double *Data);
	uint8_t addField(char  *Data, uint8_t len);
	
	uint8_t addHeaderField(uint8_t *Data);
	uint8_t addHeaderField(int *Data);
	uint8_t addHeaderField(int16_t *Data);
	uint8_t addHeaderField(uint16_t *Data);
	uint8_t addHeaderField(uint32_t *Data);
	uint8_t addHeaderField(int32_t *Data);
	uint8_t addHeaderField(float *Data);
	uint8_t addHeaderField(double *Data);
	
	uint32_t findLastRecord();
	
	uint8_t getDatabaseRecordLength();
	
	uint32_t gotoLastRecord();
	
	char *getChipJEDEC();
	
	uint64_t getUniqueChipID();
		
	uint32_t getAddress();
	
	
	
	void setAddress(uint32_t Address);
	
	void gotoRecord(uint32_t Record);
	
	uint32_t getFirstRecord(uint16_t Data, uint8_t FieldID);
		
	uint32_t getCurrentRecord();
	
	uint32_t getLastRecord();
	
	uint32_t getMaxRecords();
		
	void eraseAll();
	
	void eraseFast();
	
	void dumpBytes(uint32_t StartRecord, uint32_t TotalRecords);

	void eraseBlock(uint32_t BlockNumber);

	bool addRecord();

	bool saveRecord();
	
	bool saveHeader();

	void listFields();
	
	void listHeaderFields();

	uint8_t getFieldCount();
	
	uint8_t getHeaderFieldCount();
	
	uint8_t getFieldDataType(uint8_t Index);

	uint16_t getRecordLength();

	uint16_t getHeaderRecordLength();

	uint32_t getUsedSpace();
	
	uint32_t getTotalSpace();	
		
	uint32_t ChipCapacity = 0;
	uint8_t getField(uint8_t Data, uint8_t Field);
	int getField(int Data, uint8_t Field);
	int16_t getField(int16_t Data, uint8_t Field);
	uint16_t getField(uint16_t Data, uint8_t Field);
	int32_t getField(int32_t Data, uint8_t Field);
	uint32_t getField(uint32_t Data, uint8_t Field);
	float getField(float Record, uint8_t Field);
	
	float getFieldSpecial(float Data, uint8_t Field);
		
	double getField(double Record, uint8_t Field);
	char *getCharField(uint8_t Field);
		
	uint8_t getHeaderField(uint8_t Data, uint8_t Field);
	int getHeaderField(int Data, uint8_t Field);
	int16_t getHeaderField(int16_t Data, uint8_t Field);
	uint16_t getHeaderField(uint16_t Data, uint8_t Field);
	int32_t getHeaderField(int32_t Data, uint8_t Field);
	uint32_t getHeaderField(uint32_t Data, uint8_t Field);
	float getHeaderField(float Record, uint8_t Field);
	double getHeaderField(double Record, uint8_t Field);
	char *getHeaderField(uint8_t Field);
	char *getCharHeaderField(uint8_t Field);
		
private:

	unsigned char c;

	void DebugData(int Line);
	
	bool readChipJEDEC();
	
	uint8_t ReadData();
	void ReadBytes(uint8_t Length);
	
	//bool WriteBytes(uint8_t Array[], uint8_t Length);
	void WriteByte(uint8_t data);
		
	unsigned long bt = 0;
	bool RecordAdded = false;
	bool ReadComplete = false;
	char stng[MAXCHARLEN];
		
	size_t pageOffset;
	
	char ChipJEDEC[15];
	uint8_t a1Byte[1];
	uint8_t a2Bytes[2];
	uint8_t a4Bytes[4];
	uint8_t a8Bytes[8];	
	uint8_t aBytes[8];
	
	
	char dateBytes[8];
	bool NewCard = false;
	uint8_t ReadSpeed = 0;
	uint32_t TempAddress = 0;
	uint32_t Address = 0;
	uint32_t MaxRecords = 0;
	uint32_t LastRecord = 0;
	uint32_t CurrentRecord = 0;
	uint32_t i= 0, j= 0;
		
	uint8_t FieldCount = 0;
	uint8_t cspin = 0;
	uint8_t RecordLength= 0, DataBaseRecordLength= 0;
	int16_t pagesize= 0;

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
	double *ddata[MAX_FIELDS];
	char *cdata[MAX_FIELDS];
	char buf[MAXCHARLEN];
	uint8_t len = 0;
	int8_t bytes[MAXCHARLEN];
	
	// header stuff
	uint8_t Header_RecordLength;
	uint8_t Header_FieldCount = 0;
	
	uint8_t Header_DataType[MAX_FIELDS];
	uint8_t Header_FieldStart[MAX_FIELDS];
	uint8_t Header_FieldLength[MAX_FIELDS];
	
	uint8_t 	*u8hdata[MAX_FIELDS];
	int 		*inthdata[MAX_FIELDS];
	int16_t 	*i16hdata[MAX_FIELDS];
	uint16_t 	*u16hdata[MAX_FIELDS];
	int32_t 	*i32hdata[MAX_FIELDS];
	uint32_t 	*u32hdata[MAX_FIELDS];
	float 		*fhdata[MAX_FIELDS];
	double 		*dhdata[MAX_FIELDS];

	unsigned char flash_wait_for_write = 0;
	uint8_t readvalue;
	void write_pause();
	unsigned char flash_read_status();
		
	void B2ToBytes(uint8_t *bytes, int16_t var);
	void B2ToBytes(uint8_t *bytes, uint16_t var);
	void B4ToBytes(uint8_t *bytes, int var);
	void B4ToBytes(uint8_t *bytes, int32_t var);
	void B4ToBytes(uint8_t *bytes, uint32_t var);
	void FloatToBytes(uint8_t *bytes, float var);
	void DoubleToBytes(uint8_t *bytes, double var);
	
	
	// method to put the recordlengh to address 0
	void putDatabaseRecordLength();
	void saveField(uint8_t Bytes, uint8_t Field);
	void saveField(uint8_t Array[], uint8_t Bytes, uint8_t Field);
		
};



#endif
