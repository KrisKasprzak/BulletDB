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

  rev   date      author        change
  1.0   10/2022      kasprzak      initial code

*/

#include "SPI.h"
#include "BulletDB.h"

void BulletDB::DebugData(int Line){
	
	Serial.print("Line: "); Serial.print(Line); 
	Serial.print(" Add: "); Serial.print(Address); 
	Serial.print(" c Rec: "); Serial.print(CurrentRecord); 
	Serial.print(" t Rec's: "); Serial.println(TotalRecords); 
}

BulletDB::BulletDB(int CS_PIN) {
	
  cspin = CS_PIN;
  
}

bool BulletDB::init() {
	
	uint8_t temp = 0;
	
	pinMode(cspin, OUTPUT);

	digitalWriteFast(cspin, HIGH);

	delay(20);
	
	SPI.begin();
	delay(20);

	// getChipID();
	
	delay(20);
	
	// is this a new chip?
	Address = 0;
	Special = false;
	temp = ReadData();
	Address = 0;
		
	if(temp == 0xFF){
		// yep new chip
		Special = true;
	}
	
	
	// to do, use the return from getChipID()
	return true;
}

bool BulletDB::getChipID(){


// to do, have some checks or other
	uint16_t byteID[3];
	
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWriteFast(cspin, LOW);
	SPI.transfer(0x9F);
	byteID[0] = SPI.transfer(0);
	byteID[1] = SPI.transfer(0);
	byteID[2] = SPI.transfer(0);
	digitalWriteFast(cspin, HIGH);
	flash_wait_for_write = 1;
	write_pause();
	SPI.endTransaction();
	
	Serial.print("Chip data: byteID[0] "); Serial.print(byteID[0], HEX);
	Serial.print(", byteID[1] "); Serial.print(byteID[1], HEX);
	Serial.print(", byteID[2] "); Serial.println(byteID[2], HEX);
	
	if ((byteID[0] != 0) && (byteID[1] != 0) && (byteID[2] != 0)) {
		return true;
	}
	else {
		return false;
	}
	return true;
}

uint16_t BulletDB::getLastRecord(){
	
	uint16_t RecordSetID = 0;
	uint8_t  RecType = 0;
	uint32_t i = 0;
	
	
	Records = 0;
	TotalRecords = 0;
	Address = 0;
		

	for (i = 0; i < CARD_SIZE; i += RecordLength) {

		Address = i;
		a1Byte = ReadData();
		RecType = a1Byte;
		Address -=1;

				
		if (RecType == 0xFF) {
			// recently formatted chip
	
			if (Address > 0){
				Address = Address - RecordLength;
				TotalRecords--;
			}
			return RecordSetID;
		}

		if (RecType == 1){
			// it's a header
			Address +=1;
			a2Bytes[0] = ReadData();
			a2Bytes[1] = ReadData();
			Address = Address - 3;
			RecordSetID = (a2Bytes[0] << 8) | (a2Bytes[1]);
			TotalRecords++;
			Records = 1;	// reset record counter
		}
		if (RecType == 2){
			// its' data
			Records++;
			TotalRecords++;
		}
				
	}
	return 0;
  
}



uint8_t BulletDB::addField(const char *FieldName, uint8_t *Data) {

	DataType[FieldCount] = DT_U8;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	u8data[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
	}
	
uint8_t BulletDB::addField(const char *FieldName, int *Data) {

	DataType[FieldCount] = DT_INT;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	intdata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
	}
	
uint8_t BulletDB::addField(const char *FieldName, int16_t *Data) {

	DataType[FieldCount] = DT_I16;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	i16data[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
	}

uint8_t BulletDB::addField(const char *FieldName, uint16_t *Data) {

	DataType[FieldCount] = DT_U16;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	u16data[FieldCount] = Data;
	FieldCount++;
return FieldCount - 1;
}

uint8_t BulletDB::addField(const char *FieldName, int32_t *Data) {

	DataType[FieldCount] = DT_U32;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	i32data[FieldCount] = Data;
	FieldCount++;
return FieldCount - 1;
}

uint8_t BulletDB::addField(const char *FieldName, uint32_t *Data) {

	DataType[FieldCount] = DT_U32;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	u32data[FieldCount] = Data;
	FieldCount++;
return FieldCount - 1;
}
uint8_t BulletDB::addField(const char *FieldName, float *Data) {

	DataType[FieldCount] = DT_FLOAT;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	fdata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

void BulletDB::listFields() {

	for (i = 0; i < FieldCount; i++){
		Serial.print("Number: ");
		Serial.print(i);
		Serial.print(", Name: ");
		Serial.print(fieldname[i]);
		Serial.print(", type: ");
		if( DataType[i] == DT_U8){
			Serial.print("uint8_t");
		}
		else if( DataType[i] == DT_INT){
			Serial.print("int");
		}
		else if( DataType[i] == DT_I16){
			Serial.print("int16_t");
		}
		else if( DataType[i] == DT_U16){
			Serial.print("uint16_t");
		}
		else if( DataType[i] == DT_I32){
			Serial.print("int32_t");
		}
		else if( DataType[i] == DT_U32){
			Serial.print("uint32_t");
		}
		else if( DataType[i] == DT_FLOAT){
			Serial.print("float");
		}
		Serial.print(", start: ");
		Serial.print(FieldStart[i]);
		Serial.print(", Length: ");
		Serial.println(FieldLength[i]);
		
	}
	
}


void BulletDB::dumpData() {
	
	// for debugging mainly
	
	uint8_t ret;
	uint16_t RecordSetID = 0;
	
	TempAddress = Address;
	
	gotoAbsoluteFirstRecord();

	for (i = 0; i < 200; i++){
		Address = i * RecordLength;
		
		ret = ReadData();
		
		a2Bytes[0] = ReadData();
		a2Bytes[1] = ReadData();
		RecordSetID = (a2Bytes[0] << 8) | (a2Bytes[1]);
		Address = Address - 3;
		
		
		Serial.print("Address: "); Serial.print(Address);
		Serial.print(", Record "); Serial.print(i);
		Serial.print(", Type : "); Serial.print(ret);
		Serial.print(", RecordSetID "); Serial.println(RecordSetID);
	
	}
	
	Address = TempAddress;

}

uint8_t BulletDB::getFieldCount(){
	return FieldCount;
}

char * BulletDB::getFieldName(uint8_t Index){
	return fieldname[Index];
}


uint8_t BulletDB::getRecordLength(){
	return RecordLength;
	
}

uint32_t BulletDB::getUsedSpace(){
	return Address + RecordLength;
	
}

uint32_t BulletDB::getTotalSpace(){
	return CARD_SIZE;
}


uint8_t BulletDB::ReadData() {
	
  SPI.beginTransaction(SPISettings(SPEED_READ, MSBFIRST, SPI_MODE0));
  digitalWriteFast(cspin, LOW);
  SPI.transfer(READ); // read instruction
  SPI.transfer((Address >> 16) & 0xFF);
  SPI.transfer((Address >> 8) & 0xFF);
  SPI.transfer(Address & 0xFF);
  readvalue = SPI.transfer(0x00);
  digitalWriteFast(cspin, HIGH);
  SPI.endTransaction();
/*
    Serial.print("Read Address (DEC/HEX): (");
    Serial.print(Address, DEC);
    Serial.print(", ");
    Serial.print(Address, HEX);
    Serial.print("), Data: (");
    Serial.print(readvalue, DEC);
    Serial.print(", ");
    Serial.print(readvalue, HEX);
    Serial.print(", ");
    Serial.print((char) readvalue);
    Serial.println(")");
*/

  flash_wait_for_write = 1;
  write_pause();
  Address = Address + 1;
  return readvalue;
  
}

void BulletDB::WriteData(uint8_t data) {
  /*
     Serial.print("Save to (DEC/HEX): (");
     Serial.print(Address, DEC);
     Serial.print(", ");
     Serial.print(Address, HEX);
     Serial.print("), Data: (");
     Serial.print(data, DEC);
     Serial.print(", ");
     Serial.print(data, HEX);
     Serial.println(")");
  */

  SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
  digitalWriteFast(cspin, LOW);
  SPI.transfer(WRITEENABLE); // write instruction
  digitalWriteFast(cspin, HIGH);
  SPI.endTransaction();
  flash_wait_for_write = 1;
  write_pause();
  SPI.beginTransaction(SPISettings(25000000, MSBFIRST, SPI_MODE0));
  digitalWriteFast(cspin, LOW);
  SPI.transfer(WRITE); // write instruction
  SPI.transfer((Address >> 16) & 0xFF);
  SPI.transfer((Address >> 8) & 0xFF);
  SPI.transfer(Address & 0xFF);
  SPI.transfer(data);
  //SPI.transfer(255); // data sheet says to write this...
  digitalWriteFast(cspin, HIGH);
  SPI.endTransaction();
  flash_wait_for_write = 1;
  write_pause();
  Address = Address + 1;

}

void BulletDB::write_pause(void)
{
  if (flash_wait_for_write) {
    while (flash_read_status() & STAT_WIP);
    flash_wait_for_write = 0;
  }
}

unsigned char BulletDB::flash_read_status(void)

{
  unsigned char c;
  // This can't do a write_pause
  digitalWriteFast(cspin, LOW);
  SPI.transfer(CMD_READ_STATUS_REG);
  c = SPI.transfer(0x00);
  digitalWriteFast(cspin, HIGH);
  return (c);
}

void BulletDB::B2ToBytes(uint8_t *bytes, int16_t var) {
  bytes[0] = (uint8_t) (var >> 8);
  bytes[1] = (uint8_t) (var);
}

void BulletDB::B2ToBytes(uint8_t *bytes, uint16_t var) {
  bytes[0] = (uint8_t) (var >> 8);
  bytes[1] = (uint8_t) (var);
}
void BulletDB::B4ToBytes(uint8_t *bytes, int var) {
  bytes[0] = (uint8_t) (var >> 24);
  bytes[1] = (uint8_t) (var >> 16);
  bytes[2] = (uint8_t) (var >> 8);
  bytes[3] = (uint8_t) (var);
}
void BulletDB::B4ToBytes(uint8_t *bytes, int32_t var) {
  bytes[0] = (uint8_t) (var >> 24);
  bytes[1] = (uint8_t) (var >> 16);
  bytes[2] = (uint8_t) (var >> 8);
  bytes[3] = (uint8_t) (var);
}

void BulletDB::B4ToBytes(uint8_t *bytes, uint32_t var) {
  bytes[0] = (uint8_t) (var >> 24);
  bytes[1] = (uint8_t) (var >> 16);
  bytes[2] = (uint8_t) (var >> 8);
  bytes[3] = (uint8_t) (var);
}

void BulletDB::FloatToBytes(uint8_t *bytes_temp, float float_variable) {
  memcpy(bytes_temp, (unsigned char*) (&float_variable), 4);
}

uint32_t BulletDB::addRecord(){
		

	if (Special){
		Special = false;
		return CurrentRecord;
	}
	
	Records++;
	CurrentRecord++;
	TotalRecords++;
	
	Address = Address + RecordLength;

	return CurrentRecord;
}

/*
uint32_t BulletDB::closeRecord(){
	
	// to do...do something intelligent like return true if correct bytes written?
	
	// all this function does is make sure address is offset a full record length
	// this can only happen if user is writing special fields to create some type of header
	// save record writes a full record, so no need to close, but let them call anyway...

	return CurrentRecord;
	
}
*/

uint32_t BulletDB::saveRecord() {
	
	
	TempAddress = Address;
	
	for (i= 0; i < FieldCount; i++){
		
		if (DataType[i] == DT_U8){
			WriteData(*u8data[i]);
		}
		else if (DataType[i] == DT_INT){
			B4ToBytes(a4Bytes, *i16data[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);
		}
		else if (DataType[i] == DT_I16){
			B2ToBytes(a2Bytes, *i16data[i]);
			WriteData(a2Bytes[0]);
			WriteData(a2Bytes[1]);
		}
		else if (DataType[i] == DT_U16){
			B2ToBytes(a2Bytes, *u16data[i]);
			WriteData(a2Bytes[0]);
			WriteData(a2Bytes[1]);
		}
		else if (DataType[i] == DT_I32){
			B4ToBytes(a4Bytes, *i32data[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);
		}
		else if (DataType[i] == DT_U32){
			B4ToBytes(a4Bytes, *u32data[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);
		}
		else if (DataType[i] == DT_FLOAT){
			
			FloatToBytes(a4Bytes, *fdata[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);	

		}	
		
	}
	
	Address = TempAddress;

	return CurrentRecord;
}

void BulletDB::eraseAll(){
	//Serial.println("Erasing chip");
	flash_wait_for_write = 1;
	write_pause();
	digitalWriteFast(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWriteFast(cspin, HIGH);
	delay(5);
	digitalWriteFast(cspin, LOW);
	SPI.transfer(CHIPERASE);
	digitalWriteFast(cspin, HIGH);
	flash_wait_for_write = 1;
	write_pause();

	//Serial.println("Erase done");
}
	
void BulletDB::erasePage(uint32_t PageNumber){
	uint32_t TempAddress = Address;
	Address = PageNumber * PAGE_SIZE;
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWriteFast(cspin, LOW);
	SPI.transfer(WRITEENABLE); // write instruction
	digitalWriteFast(cspin, HIGH);
	SPI.endTransaction();
	flash_wait_for_write = 1;
	write_pause();
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWriteFast(cspin, LOW);
	SPI.transfer(CMD_SECTOR_ERASE); // write instruction
	SPI.transfer((Address >> 16) & 0xFF);
	SPI.transfer((Address >> 8) & 0xFF);
	SPI.transfer(Address & 0xFF);
	digitalWriteFast(cspin, HIGH);
	SPI.endTransaction();
	flash_wait_for_write = 1;
	write_pause();
	
	Address = TempAddress;
}

float BulletDB::getField(uint32_t Record, uint8_t Field){

	TempAddress = Address;
	
	Address = (Record * RecordLength) + FieldStart[Field];
	
	if (DataType[Field] == DT_U8){
		a1Byte = ReadData();
		Address = TempAddress;
		return a1Byte;
	}
	else if (DataType[Field] == DT_INT){
		a4Bytes[0] = ReadData();
		a4Bytes[1] = ReadData();
		a4Bytes[2] = ReadData();
		a4Bytes[3] = ReadData(); //6
		Address = TempAddress;
		return ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));
	}
	else if (DataType[Field] == DT_I16){
		a2Bytes[0] = ReadData();
		a2Bytes[1] = ReadData();
		Address = TempAddress;
		return (a2Bytes[0] << 8) | (a2Bytes[1]);
	}
	else if (DataType[Field] == DT_U16){
		a2Bytes[0] = ReadData();
		a2Bytes[1] = ReadData();
		Address = TempAddress;
		return (a2Bytes[0] << 8) | (a2Bytes[1]);
	}
	else if (DataType[Field] == DT_U32){
		a4Bytes[0] = ReadData();
		a4Bytes[1] = ReadData();
		a4Bytes[2] = ReadData();
		a4Bytes[3] = ReadData(); //6
		Address = TempAddress;
		return ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));
	}
	else if (DataType[Field] == DT_FLOAT){
		
		a4Bytes[0] = ReadData();
		a4Bytes[1] = ReadData();
		a4Bytes[2] = ReadData();
		a4Bytes[3] = ReadData();
				
	Address = TempAddress;	
		return *(float *)&a4Bytes;
	}	
	// should never get here, but keep the compiler happy
	Address = TempAddress;
	return 0.0;
}

uint32_t BulletDB::getCurrentRecord(){
	return CurrentRecord;
	
}

uint32_t BulletDB::getRecords(){
	return Records;
	
}

uint32_t BulletDB::getTotalRecords(){
	return TotalRecords;
	
}

uint32_t BulletDB::getAddress(){
	return Address;
	
}

void BulletDB::gotoAbsoluteFirstRecord(){
	Address = 0x00;
}

void BulletDB::gotoRecord(uint32_t RecordNumber){
	
	CurrentRecord = RecordNumber;

	Address = RecordNumber * RecordLength;


}

uint8_t BulletDB::getSpecialField(uint32_t Record, uint32_t AddressOffset, uint8_t Data){

	TempAddress = Address;
	Address = (Record * RecordLength) + AddressOffset;
	a1Byte = ReadData();
	Address = TempAddress;
	return a1Byte;

}

int BulletDB::getSpecialField(uint32_t Record, uint32_t AddressOffset, int Data){

	TempAddress = Address;
	Address = (Record * RecordLength) + AddressOffset;
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));
	
}

int16_t BulletDB::getSpecialField(uint32_t Record, uint32_t AddressOffset, int16_t Data){

	TempAddress = Address;
	Address = (Record * RecordLength) + AddressOffset;
	a2Bytes[0] = ReadData();
	a2Bytes[1] = ReadData();
	Address = TempAddress;
	return (a2Bytes[0] << 8) | (a2Bytes[1]);
	
}

uint16_t BulletDB::getSpecialField(uint32_t Record, uint32_t AddressOffset, uint16_t Data){

	TempAddress = Address;
	Address = (Record * RecordLength) + AddressOffset;
	a2Bytes[0] = ReadData();
	a2Bytes[1] = ReadData();
	Address = TempAddress;
	return (a2Bytes[0] << 8) | (a2Bytes[1]);
	
}

int32_t BulletDB::getSpecialField(uint32_t Record, uint32_t AddressOffset, int32_t Data){

	TempAddress = Address;
	Address = (Record * RecordLength) + AddressOffset;
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

uint32_t BulletDB::getSpecialField(uint32_t Record, uint32_t AddressOffset, uint32_t Data){

	TempAddress = Address;
	Address = (Record * RecordLength) + AddressOffset;
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

float BulletDB::getSpecialField(uint32_t Record, uint32_t AddressOffset, float Data){

	TempAddress = Address;
	Address = (Record * RecordLength) + AddressOffset;
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData();	
	Address = TempAddress;	
	return *(float *)&a4Bytes;

}



// to do, implment a rec len check so we dont write past a record

void BulletDB::writeSpecialField(uint32_t AddressOffset, uint8_t Data){
	
	TempAddress = Address;
	Address = Address + AddressOffset;
	WriteData(Data);
	Address = TempAddress;
	
	
}

void BulletDB::writeSpecialField(uint32_t AddressOffset, int16_t Data){
	
	TempAddress = Address;
	Address = Address + AddressOffset;
	B2ToBytes(a2Bytes, Data);
	WriteData(a2Bytes[0]);
	WriteData(a2Bytes[1]);
	Address = TempAddress;
	
	
}

void BulletDB::writeSpecialField(uint32_t AddressOffset, uint16_t Data){
	
	TempAddress = Address;
	Address = Address + AddressOffset;
	B2ToBytes(a2Bytes, Data);
	WriteData(a2Bytes[0]);
	WriteData(a2Bytes[1]);
	Address = TempAddress;
	
	
}
void BulletDB::writeSpecialField(uint32_t AddressOffset, int Data){
	
	TempAddress = Address;
	Address = Address + AddressOffset;
	B4ToBytes(a4Bytes, Data);
	WriteData(a4Bytes[0]);
	WriteData(a4Bytes[1]);
	WriteData(a4Bytes[2]);
	WriteData(a4Bytes[3]);
	Address = TempAddress;
		
}

void BulletDB::writeSpecialField(uint32_t AddressOffset, int32_t Data){
	
	TempAddress = Address;
	Address = Address + AddressOffset;
	B4ToBytes(a4Bytes, Data);
	WriteData(a4Bytes[0]);
	WriteData(a4Bytes[1]);
	WriteData(a4Bytes[2]);
	WriteData(a4Bytes[3]);
	Address = TempAddress;
		
}

void BulletDB::writeSpecialField(uint32_t AddressOffset, uint32_t Data){
	
	TempAddress = Address;
	Address = Address + AddressOffset;
	B4ToBytes(a4Bytes, Data);
	WriteData(a4Bytes[0]);
	WriteData(a4Bytes[1]);
	WriteData(a4Bytes[2]);
	WriteData(a4Bytes[3]);
	Address = TempAddress;
		
}

void BulletDB::writeSpecialField(uint32_t AddressOffset, float Data){
	
	TempAddress = Address;
	Address = Address + AddressOffset;
	FloatToBytes(a4Bytes, *fdata[i]);
	WriteData(a4Bytes[0]);
	WriteData(a4Bytes[1]);
	WriteData(a4Bytes[2]);
	WriteData(a4Bytes[3]);
	Address = TempAddress;
	
}

