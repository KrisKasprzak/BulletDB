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
/*
to do

1. add methods to set read/write/erase instructions
2. remove addrecord
3. clean up some of the get stuff--not srue all are needed
4. resolve waht is returned an address or record --probably a record
5. maybe get rid of dumpdata or make private or protected
6. maybe get rid of address dump or make private
7. add methods to set the chip size
8. remove manafest constant to set secot size--since we don't use iter_swap
9. remove all the manafest constants that are not used
10. fix the names and such for the wird timing functions
11. write an example for multi file approach
12. maybe where we dump all recordsetID and user can print one to screen?
13. start writing better read me
1. intent for library
2. typical use cases
3. how it works
4. what it can do
5. what it can't do
6. list of methods and explaination
7. timing graph?
8. maybe show some serial output code?
9. definately show a sensor with reads ever 5 ms/ 100 ms/ 500 ms (maybe accelerameter) 
14. limitations
15. how to adapt to other chips (read data sheet and get size and instruction codes
16. test with my other chips
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

#define BULLET_DB_VER 1.04

#define NULL_RECORD 0xFF

#define MAX_FIELDS 20
#define MAXCHARLEN 20

#define MAXDATACHARLEN 15

// #define CARD_SIZE 520191
#define CARD_SIZE 8384511
#define PAGE_SIZE 4096

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
#define CMD_CHIP_ERASE         0xC7
#define CMD_WRITE_ENABLE       0x06
#define CMD_WRITE_STATUS_REG   0x01
#define CMD_READ_STATUS_REG    0x05
#define CMD_SECTOR_ERASE       0x20//not tested
#define STAT_WIP 1

/*
#define PAGE 4096
#define PAGES 2048

#define STAT_WEL 2

#define CMD_PAGE_PROGRAM       0x02

#define CMD_WRITE_DISABLE      0x04//not tested


#define CMD_READ_HIGH_SPEED    0x0B//not tested

#define CMD_BLOCK32K_ERASE     0x52//not tested
#define CMD_RESET_DEVICE       0xF0//<<-different from winbond
#define CMD_READ_ID            0x9F
#define CMD_RELEASE_POWER_DOWN 0xAB//not tested
#define CMD_POWER_DOWN         0xB9//not tested
*/
#define CMD_BLOCK64K_ERASE     0xD8//not tested
#define WRITEENABLE   0x06
#define WRITE         0x02
#define READ          0x03
#define RID           0xAB
#define JEDEC         0x9F
#define CHIPERASE     0x60
#define COMPLETE      0x05
#define RECOVERY 2

class  BulletDB {
		
public:

	BulletDB(int CS_PIN);
	
	bool init();
	
	uint8_t addField(const char *FieldName, uint8_t *Data);	
	uint8_t addField(const char *FieldName, int *Data);
	uint8_t addField(const char *FieldName, int16_t *Data);
	uint8_t addField(const char *FieldName, uint16_t *Data);
	uint8_t addField(const char *FieldName, uint32_t *Data);
	uint8_t addField(const char *FieldName, int32_t *Data);
	uint8_t addField(const char *FieldName, float *Data);
	uint8_t addField(const char *FieldName, double *Data);
	uint8_t addField(const char *FieldName, char  *Data, uint8_t len);
	
	uint8_t addHeaderField(const char *FieldName, uint8_t *Data);
	uint8_t addHeaderField(const char *FieldName, int *Data);
	uint8_t addHeaderField(const char *FieldName, int16_t *Data);
	uint8_t addHeaderField(const char *FieldName, uint16_t *Data);
	uint8_t addHeaderField(const char *FieldName, uint32_t *Data);
	uint8_t addHeaderField(const char *FieldName, int32_t *Data);
	uint8_t addHeaderField(const char *FieldName, float *Data);
	uint8_t addHeaderField(const char *FieldName, double *Data);
	
	int32_t readTotalRecords();
	
	int32_t getLastRecord();
		
	int32_t gotoLastRecord();
	
	char *getChipJEDEC();
		
	uint32_t getAddress();
	
	void setAddress(uint32_t Address);
	
	void gotoRecord(uint32_t Record);
	
	void gotoAbsoluteFirstRecord();
	
	uint32_t getRecords();
	
	uint32_t getCurrentRecord();
	
	uint32_t getTotalRecords();
	
	void eraseAll();
	
	void dumpBytes(uint8_t RecLen, uint32_t Recs);

	void erasePage(uint32_t PageNumber);

	uint32_t addRecord();

	uint32_t saveRecord();
	
	uint32_t saveHeader();

	void listFields();
	
	void listHeaderFields();

	uint8_t getFieldCount();
	
	uint8_t getHeaderFieldCount();
	
	uint8_t getFieldDataType(uint8_t Index);

	char * getFieldName(uint8_t Index);
	
	char * getHeaderFieldName(uint8_t Index);
	
	uint16_t getRecordLength();

	uint16_t getHeaderRecordLength();

	uint32_t getUsedSpace();
	
	uint32_t getTotalSpace();	
	
	uint8_t getField(uint8_t Data, uint8_t Field);
	int getField(int Data, uint8_t Field);
	int16_t getField(int16_t Data, uint8_t Field);
	uint16_t getField(uint16_t Data, uint8_t Field);
	int32_t getField(int32_t Data, uint8_t Field);
	uint32_t getField(uint32_t Data, uint8_t Field);
	float getField(float Record, uint8_t Field);
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
	
	
	//float getHeaderField(uint8_t Field);
	//char *getCharHeaderField(uint32_t Record, uint8_t Field);
		
private:

	unsigned char c;

	void DebugData(int Line);
	
	int16_t readChipJEDEC();
	
	unsigned long bt = 0;
	
	bool ReadComplete = false;
	char stng[MAXDATACHARLEN];
	uint8_t ReadData();
	void WriteData(uint8_t data);
	char ChipJEDEC[9];
	uint8_t a1Byte;
	uint8_t a2Bytes[2];
	uint8_t a4Bytes[4];
	uint8_t a8Bytes[8];
	char dateBytes[8];
	bool NewCard = false;
	uint32_t TempAddress = 0;
	uint32_t UsedAddress = 0;
	uint32_t Address = 0;
	uint32_t NextWritableAddress = 0;
	uint32_t Records = 0;
	uint32_t CurrentRecord = 0;
	uint32_t TotalRecords = 0;
	uint32_t i, j;
	
	
	uint8_t FieldCount = 0;
	uint8_t cspin;
	uint8_t RecordLength;
	int16_t pagesize;

	uint8_t DataType[MAX_FIELDS];
	uint8_t FieldStart[MAX_FIELDS];
	uint8_t FieldLength[MAX_FIELDS];
	char fieldname[MAX_FIELDS][MAXCHARLEN];
		
	uint8_t *u8data[MAX_FIELDS];
	int *intdata[MAX_FIELDS];
	int16_t *i16data[MAX_FIELDS];
	uint16_t *u16data[MAX_FIELDS];
	int32_t *i32data[MAX_FIELDS];
	uint32_t *u32data[MAX_FIELDS];
	float *fdata[MAX_FIELDS];
	double *ddata[MAX_FIELDS];
	char *cdata[MAX_FIELDS];
	char buf[MAXDATACHARLEN];
	
		// header stuff
	uint8_t Header_RecordLength;
	uint8_t Header_FieldCount = 0;
	
	uint8_t Header_DataType[MAX_FIELDS];
	uint8_t Header_FieldStart[MAX_FIELDS];
	uint8_t Header_FieldLength[MAX_FIELDS];
	char Header_FieldName[MAX_FIELDS][MAXCHARLEN];
	
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
};



#endif
