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
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  On a personal note, if you develop an application or product using this library
  and make millions of dollars, I'm happy for you!

revisions: see .h file
*/

#include "SPI.h"
#include "BulletDB.h"

void BulletDB::DebugData(int Line){
	
	Serial.print("Line: "); Serial.print(Line); 
	Serial.print(" Address: "); Serial.print(Address); 
	Serial.print(" Current Rec: "); Serial.print(CurrentRecord); 
	Serial.print(" Max Rec  : "); Serial.print(MaxRecords); 
	Serial.print(" Field Count  : "); Serial.println(FieldCount); 

}

BulletDB::BulletDB(int CS_PIN) {
	
  cspin = CS_PIN;  
  
}

bool BulletDB::init() {
	
	int16_t Return;
	
	ReadComplete = false;
	
	SPI.begin();
	delay(20);	
	pinMode(cspin, OUTPUT);

	digitalWrite(cspin, HIGH);
	delay(20);	

	Return = readChipJEDEC();

	return Return;
}

void BulletDB::putDatabaseRecordLength() {
	
	
	setAddress(0);
	Address = 0;
	
	WriteByte(RecordLength);

}

uint8_t BulletDB::getDatabaseRecordLength() {	
	
	setAddress(0);
	Address = 0;
	ReadBytes(1);	
	DataBaseRecordLength = aBytes[0];
	
	return DataBaseRecordLength;

}

uint64_t BulletDB::getUniqueChipID(){
	
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	delay(10);
	
	SPI.transfer(CMD_READ_ID);
	
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	SPI.transfer(0x00);
	SPI.transfer(0x00);

	aBytes[0] = SPI.transfer(0x00);
	aBytes[1] = SPI.transfer(0x00);
	aBytes[2] = SPI.transfer(0x00);
	aBytes[3] = SPI.transfer(0x00);
	aBytes[4] = SPI.transfer(0x00);
	aBytes[5] = SPI.transfer(0x00);
	aBytes[6] = SPI.transfer(0x00);
	aBytes[7] = SPI.transfer(0x00);

	digitalWrite(cspin, HIGH);
	SPI.endTransaction();
	
	return (uint64_t) aBytes[0] << 56  
	| (uint64_t) aBytes[1] << 48
	| (uint64_t) aBytes[2] << 40
	| (uint64_t) aBytes[3] << 32
	| (uint64_t) aBytes[4] << 24
	| (uint64_t) aBytes[5] << 16
	| (uint64_t) aBytes[6] << 8
	| (uint64_t) aBytes[7]	
	;
	
}

 bool BulletDB::readChipJEDEC(){
	 
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	delay(10);
	
	SPI.transfer(JEDEC);

	aBytes[0] = SPI.transfer(0x00);
	aBytes[1] = SPI.transfer(0x00);
	aBytes[2] = SPI.transfer(0x00);

	digitalWrite(cspin, HIGH);
	SPI.endTransaction();

	ChipCapacity = pow(2, aBytes[2]);
	
	if ((aBytes[0] == 0) || (aBytes[0] == NULL_RECORD)) {
		strcpy(ChipJEDEC,"INVALID CHIP");
		return false;
	}
	else if ((aBytes[1] == 0) || (aBytes[1] == NULL_RECORD)) {
		strcpy(ChipJEDEC,"INVALID CHIP");
		return false;
	}
	else if ((aBytes[2] == 0) || (aBytes[2] == NULL_RECORD)) {
		strcpy(ChipJEDEC,"INVALID CHIP");
		return false;
	}
	else {
		sprintf(ChipJEDEC,"%02x:%02x:%02x",aBytes[0],aBytes[1],aBytes[2]);
		//Serial.print("Chip data: ");Serial.println(ChipJEDEC);
	}
	return true;
	
 }
 
 char *BulletDB::getChipJEDEC(){
	
	return ChipJEDEC;
	
}

uint32_t BulletDB::findLastRecord(){
	
	uint32_t StartRecord = 0;
	uint32_t MiddleRecord = 0;
	uint32_t EndRecord = 0;
	bool Found = false;
	uint8_t RecType = 0;
	uint8_t NextRecType = 0;
	uint32_t Iteration = 0;

	// get maximum possible records
	// can't more records that memory
	MaxRecords = CARD_SIZE / RecordLength;
	// we back out one record to accomodate a possible partial record
	// we back out one more since were' not starting at 0;
	MaxRecords = MaxRecords - 2;	
	
	// test first record
	Address = 1 * RecordLength;
	
	ReadBytes(1);
	
	RecType = aBytes[0];

	if (RecType == NULL_RECORD){
		// no DATA
		NewChip = true;
		CurrentRecord = 0;
		LastRecord = 0;
		ReadComplete = true;
		return LastRecord;
	}
	
	// test end
	Address = MaxRecords * RecordLength;
	
	ReadBytes(1);
	
	RecType = aBytes[0];

	if (RecType != NULL_RECORD){
		// card full
		NewChip = false;
		CurrentRecord = MaxRecords;
		LastRecord = MaxRecords;
		ReadComplete = true;
		return LastRecord;
	}
	
	// begin the bisectional seek to get the last valid record 
	// (where record start address is not 0xFFFF and next record IS 0xFFFF)
	StartRecord = 1;
	EndRecord = MaxRecords;
	
	// OK last record is somewhere in between...
	while(!Found) {
		
		Iteration++;
		
		MiddleRecord = (EndRecord + StartRecord) / 2;
		Address = MiddleRecord * RecordLength;
		ReadBytes(1);	
		RecType = aBytes[0];

		Address = (MiddleRecord + 1)  * RecordLength;
		ReadBytes(1);	
		NextRecType = aBytes[0];
	
		if ((RecType == NULL_RECORD) && (NextRecType == NULL_RECORD)){
			EndRecord = MiddleRecord;
		}
		if ((RecType != NULL_RECORD) && (NextRecType != NULL_RECORD)){
			StartRecord = MiddleRecord;
		}
		if ((RecType != NULL_RECORD) && (NextRecType == NULL_RECORD)){
			// we found the end
			LastRecord = MiddleRecord;
			Found = true;
		}		
		if (Iteration > 24) { // 23 bits is max iteration for 23 bit for this chip, 1 more for good luck
			NewChip = true;
			CurrentRecord = 0;
			LastRecord = 0;
			ReadComplete = true;
			return CHIP_FORCE_RESTART;
		}			
	}
		
	NewChip = false;
	LastRecord = MiddleRecord;
	CurrentRecord = MiddleRecord;

	ReadComplete = true;
		
	return LastRecord;
	
}

uint32_t BulletDB::getFirstRecord( uint16_t TargetData, uint8_t FieldID){
	
	uint32_t StartRecord = 0;
	uint32_t MiddleRecord = 0;
	uint32_t EndRecord = 0;
	bool Found = false;
	uint32_t Iteration = 0;
	uint16_t DataCR = 0, DataPR = 0;
	
	gotoRecord(1);
	DataCR = getField(DataCR, FieldID);

	if (DataCR == TargetData){
		return 1;
	}
		
	// test last record, if target is > last record, record does not exist
	gotoRecord(getLastRecord());
	DataCR = getField(DataCR, FieldID);
	gotoRecord(MiddleRecord-1);
	DataPR = getField(DataPR, FieldID);
	if (TargetData > DataCR){
		return 0;
	}
	
	if ((DataCR == TargetData) && (DataPR == (TargetData - 1))){
		return getLastRecord();
	}
	
	// begin the bisectional seek to get the last valid record 
	// (where record start address is not 0xFFFF and next record IS 0xFFFF)
	StartRecord = 1;
	EndRecord = LastRecord;
	
	// OK last record is somewhere in between...
	while(!Found) {
		
		Iteration++;
		
		MiddleRecord = (EndRecord + StartRecord) / 2;
		
		gotoRecord(MiddleRecord);
		DataCR = getField(DataCR, FieldID);
		gotoRecord(MiddleRecord-1);
		DataPR = getField(DataPR, FieldID);

		if ((DataCR == TargetData) && (DataPR == (TargetData - 1))){
			// we found it...
			Found = true;
			return MiddleRecord;
		}
		if (Iteration > 50) { // 23 bits is max iteration for 23 bit for this chip, 1 more for good luck
			NewChip = true;
			CurrentRecord = 0;
			EndRecord = 0;
			ReadComplete = true;
			return CHIP_FORCE_RESTART;
		}
		
		// determine if we search upper half or lower half
		if ((DataCR == NULL_RECORD) && (DataPR == NULL_RECORD)){
			EndRecord = MiddleRecord;
		}	
		// data in upper half
		else if (DataCR < TargetData){
			StartRecord = MiddleRecord;
		}
		
		// data in lower half
		else if (DataCR > TargetData){
			EndRecord = MiddleRecord;
		}
		// data in lower half
		else if ((DataCR == TargetData) && (DataCR == TargetData)) {
			EndRecord = MiddleRecord;
		}
		else {
			Serial.println("Line 348 in BulletDB.cpp, Missing Search Case.");
		}

			
	}
	return MiddleRecord;

}

// data field addField methods
uint8_t BulletDB::addField(uint8_t *Data) {
		
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_U8;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	u8data[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

uint8_t BulletDB::addField(int *Data) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_INT;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	intdata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
	}
	
uint8_t BulletDB::addField(int16_t *Data) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_I16;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	i16data[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
	}

uint8_t BulletDB::addField(uint16_t *Data) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_U16;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	u16data[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

uint8_t BulletDB::addField(int32_t *Data) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_U32;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	i32data[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

uint8_t BulletDB::addField(uint32_t *Data) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_U32;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	u32data[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}
uint8_t BulletDB::addField(float *Data) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_FLOAT;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	fdata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

uint8_t BulletDB::addField(double *Data) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_DOUBLE;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	ddata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

uint8_t BulletDB::addField(char *Data, uint8_t len) {
	if (FieldCount >= MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_CHAR;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  len;
	RecordLength = RecordLength + len;
	cdata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

// header addHeaderData methods
uint8_t BulletDB::addHeaderField(uint8_t *Data) {
		
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){
		return 0;
	}
	
	Header_DataType[Header_FieldCount] = DT_U8;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	u8hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
	
}
	
uint8_t BulletDB::addHeaderField(int *Data) {
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){
		return 0;
	}
	Header_DataType[Header_FieldCount] = DT_INT;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	inthdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}
	
uint8_t BulletDB::addHeaderField(int16_t *Data) {
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){
		return 0;
	}
	Header_DataType[Header_FieldCount] = DT_I16;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	i16hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
	
}

uint8_t BulletDB::addHeaderField(uint16_t *Data) {
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){
		return 0;
	}
	Header_DataType[Header_FieldCount] = DT_U16;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	u16hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}


uint8_t BulletDB::addHeaderField(int32_t *Data) {
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){
		return 0;
	}
	Header_DataType[Header_FieldCount] = DT_U32;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	i32hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}

uint8_t BulletDB::addHeaderField(uint32_t *Data) {
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){
		return 0;
	}
	Header_DataType[Header_FieldCount] = DT_U32;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	u32hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}

uint8_t BulletDB::addHeaderField(float *Data) {
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){
		return 0;
	}
	Header_DataType[Header_FieldCount] = DT_FLOAT;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	fhdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}

uint8_t BulletDB::addHeaderField(double *Data) {
	
	if (Header_RecordLength + sizeof(*Data) >= RecordLength){		
		return 0;
	}		
	Header_DataType[Header_FieldCount] = DT_DOUBLE;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	dhdata[Header_FieldCount] = Data;	
	Header_FieldCount++;		
	return Header_FieldCount - 1;
		
}

void BulletDB::listFields() {

	for (i = 0; i < FieldCount; i++){
		Serial.print("Number: ");
		Serial.print(i);
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
		else if( DataType[i] == DT_DOUBLE){
			Serial.print("double");
		}
		else if( DataType[i] == DT_CHAR){
			Serial.print("char");
		}
		Serial.print(", start: ");
		Serial.print(FieldStart[i]);
		Serial.print(", Length: ");
		Serial.println(FieldLength[i]);			
	}	
	Serial.print("Record length: ");Serial.println(RecordLength);
	Serial.print("Field count: ");Serial.println(FieldCount);	
}

void BulletDB::listHeaderFields() {

	for (i = 0; i < Header_FieldCount; i++){
				
		Serial.print("Number: ");
		Serial.print(i);
		Serial.print(", type: ");
		
		if( Header_DataType[i] == DT_U8){
			Serial.print("uint8_t");
		}
		else if( Header_DataType[i] == DT_INT){
			Serial.print("int");
		}
		else if( Header_DataType[i] == DT_I16){
			Serial.print("int16_t");
		}
		else if( Header_DataType[i] == DT_U16){
			Serial.print("uint16_t");
		}
		else if( Header_DataType[i] == DT_I32){
			Serial.print("int32_t");
		}
		else if( Header_DataType[i] == DT_U32){
			Serial.print("uint32_t");
		}
		else if( Header_DataType[i] == DT_FLOAT){
			Serial.print("float");
		}
		else if( Header_DataType[i] == DT_DOUBLE){
			Serial.print("double");
		}
		
		Serial.print(", start: ");
		Serial.print(Header_FieldStart[i]);
		Serial.print(", Length: ");
		Serial.println(Header_FieldLength[i]);
			
	}
	
	Serial.print("Header Record length: ");Serial.println(Header_RecordLength);
	Serial.print("Header field count: ");Serial.println(Header_FieldCount);
	
}

uint32_t BulletDB::gotoLastRecord(){	
	CurrentRecord = LastRecord;
	return CurrentRecord;		
}

uint8_t BulletDB::getFieldCount(){
	return FieldCount;
}

uint8_t BulletDB::getHeaderFieldCount(){
	return Header_FieldCount;
}

uint16_t BulletDB::getRecordLength(){
	return RecordLength;
}

uint16_t BulletDB::getHeaderRecordLength(){
	return Header_RecordLength;
}

uint32_t BulletDB::getUsedSpace(){
	return LastRecord * RecordLength;
}

uint32_t BulletDB::getTotalSpace(){
	return ChipCapacity;
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

void BulletDB::FloatToBytes(uint8_t *bytes, float var) {
  memcpy(bytes, (unsigned char*) (&var), 4);
}

void BulletDB::DoubleToBytes(uint8_t *bytes, double var) {
  memcpy(bytes, (unsigned char*) (&var), 8);
}

bool BulletDB::addRecord(){
	
	// special case for address = 0 (new card)
	// write to address 0, otherwise advance address to next record
	// save record does not advance address it returns to it's initial address	

	if (!ReadComplete){
		findLastRecord();
		ReadComplete = true;
	}
	
	if (CurrentRecord >= MaxRecords) {
		RecordAdded = false;
		return false;
	}
	
	// now that record is written, bump the address to the next writable
	// address
	
	CurrentRecord++;
	LastRecord++;
	RecordAdded = true;
	
	return true;
		
}

void BulletDB::dumpBytes(uint32_t StartRecord, uint32_t TotalRecords) {
	
	uint32_t TempRecord = 0;
	uint8_t data = 0;
	uint32_t InvalidRecords = 0;
	uint32_t MaxRecords = 0;
	
	TempRecord = CurrentRecord;
	
	CurrentRecord = StartRecord;
	MaxRecords = CurrentRecord + TotalRecords;
	
	Serial.println("Dump bytes-------------------- "); 
	
	// keeping dumping memory until we get 0xFFFF too many times
	// this will account for any skips (assuming don't have more than 100 skipped records)
	
	while (InvalidRecords < 1000){
		
		Address = CurrentRecord * RecordLength;
		
		Serial.print("Address: "); Serial.print(Address);
		Serial.print(", Record: "); Serial.print(CurrentRecord); 
		Serial.print(" - ");
			
		for (j = 0; j< RecordLength; j++){
			ReadBytes(1);
			data = aBytes[0];
			
			if ((j == 0) && (data == NULL_RECORD)){
				InvalidRecords++;
			}
			Serial.print(data);
			Serial.print("-");
		}
		Serial.println("");
		
		if (CurrentRecord >= MaxRecords) {
			gotoRecord(TempRecord);
			return;
		}
		
		CurrentRecord++;

	}
	
	gotoRecord(TempRecord);

}
		
bool BulletDB::saveRecord() {
	
	if (!RecordAdded) {
		if (!addRecord()){
			return false;
		}
	}

	for (i= 0; i < FieldCount; i++){		
		
		if (DataType[i] == DT_U8){			
			aBytes[0] = *u8data[i];
			saveField(FieldLength[i], i);			
		}
		else if (DataType[i] == DT_INT){			
			B4ToBytes(aBytes, *intdata[i]);
			saveField(FieldLength[i], i);
		}
		else if (DataType[i] == DT_I16){
			B2ToBytes(aBytes, *i16data[i]);
			saveField(FieldLength[i], i);
		}
		else if (DataType[i] == DT_U16){
			B2ToBytes(aBytes, *u16data[i]);			
			saveField(FieldLength[i], i);
		}
		else if (DataType[i] == DT_I32){			
			B4ToBytes(aBytes, *i32data[i]);
			saveField(FieldLength[i], i);
		}
		else if (DataType[i] == DT_U32){	
			B4ToBytes(aBytes, *u32data[i]);
			saveField(FieldLength[i], i);
		}
		else if (DataType[i] == DT_FLOAT){		
			FloatToBytes(aBytes, *fdata[i]);
			saveField(FieldLength[i], i);			
		}	
		else if (DataType[i] == DT_DOUBLE){		
			DoubleToBytes(aBytes, *ddata[i]);
			saveField(FieldLength[i], i);			
		}
	
		else if (DataType[i] == DT_CHAR){				
			strcpy(buf,cdata[i]);
			saveField((uint8_t*)buf, sizeof(buf), i);
		}
	}
	RecordAdded = false;
	return true;
	
}

bool BulletDB::saveHeader() {

	if (!RecordAdded) {
		if (!addRecord()){		
			return false;
		}
	}	
	
	for (i= 0; i < Header_FieldCount; i++){
			
		if (Header_DataType[i] == DT_U8){
			aBytes[0] = *u8hdata[i];				
			saveHeaderField(Header_FieldLength[i], i);
		}
		else if (Header_DataType[i] == DT_INT){		
			Header_DataType[Header_FieldCount] = DT_INT;
			B4ToBytes(aBytes, *inthdata[i]);
			saveHeaderField(Header_FieldLength[i], i);	
		}
		else if (Header_DataType[i] == DT_I16){
			B2ToBytes(aBytes, *i16hdata[i]);
			saveHeaderField(Header_FieldLength[i], i);	
		}
		else if (Header_DataType[i] == DT_U16){
			B2ToBytes(aBytes, *u16hdata[i]);
			saveHeaderField(Header_FieldLength[i], i);	
		}
		else if (Header_DataType[i] == DT_I32){
			B4ToBytes(aBytes, *i32hdata[i]);
			saveHeaderField(Header_FieldLength[i], i);	
		}
		else if (Header_DataType[i] == DT_U32){
			B4ToBytes(aBytes, *u32hdata[i]);
			saveHeaderField(Header_FieldLength[i], i);	
		}
		else if (Header_DataType[i] == DT_FLOAT){
			FloatToBytes(aBytes, *fhdata[i]);
			saveHeaderField(Header_FieldLength[i], i);		
		}
		else if (Header_DataType[i] == DT_DOUBLE){
			DoubleToBytes(aBytes, *dhdata[i]);
			saveHeaderField(Header_FieldLength[i], i);	
		}		
	}
	
	return true;
	
}

void BulletDB::eraseAll(){
	
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));	

	flash_wait_for_write = 1;
	write_pause();
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH);
	write_pause();
	delay(25);
	digitalWrite(cspin, LOW);
	SPI.transfer(CHIPERASE);
	digitalWrite(cspin, HIGH);
	flash_wait_for_write = 1;
	write_pause();
	SPI.endTransaction();
	
	NewChip = true;	
	findLastRecord();
	putDatabaseRecordLength();
	gotoRecord(1);	

}

uint8_t BulletDB::getUsedBlocks(){
	uint8_t usedBlocks = 0;
	
	usedBlocks = (Address / BLOCK_SIZE);
	
	if ((usedBlocks == 0) && (Address > 0)){
		usedBlocks = 0;
	}
	if (usedBlocks > 128){
		usedBlocks = 128;
	}
	return usedBlocks;	
	
}

void BulletDB::eraseBlock(uint8_t BlockNumber){

	Address = BlockNumber * BLOCK_SIZE;
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE); // write instruction
	digitalWrite(cspin, HIGH);
	SPI.endTransaction();
	flash_wait_for_write = 1;
	write_pause();
	
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(CMD_BLOCK64K_ERASE); // write instruction
	SPI.transfer((Address >> 16) & 0xFF);
	SPI.transfer((Address >> 8) & 0xFF);
	SPI.transfer(Address & 0xFF);
	digitalWrite(cspin, HIGH);
	write_pause();
	SPI.endTransaction();
	flash_wait_for_write = 1;	
	
	if (BlockNumber == getUsedBlocks()){
		NewChip = true;	
		findLastRecord();
		putDatabaseRecordLength();
		gotoRecord(1);	
	}
	
}


//////////////////////////////////////////////////////////////////////////////////
// get data
	
uint8_t BulletDB::getField(uint8_t Data, uint8_t Field){


	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
	ReadBytes(1);
	return (uint8_t) aBytes[0];

}

int BulletDB::getField(int Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
	
	ReadBytes(sizeof(Data));
	
	return (int) ( (aBytes[0] << 24) | (aBytes[1] << 16) | (aBytes[2] << 8) | (aBytes[3]));

}

int16_t BulletDB::getField(int16_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
	
	ReadBytes(sizeof(Data));

	return (int16_t) (aBytes[0] << 8) | (aBytes[1]);

}

uint16_t BulletDB::getField(uint16_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
	
	ReadBytes(sizeof(Data));

	return (uint16_t) (aBytes[0] << 8) | (aBytes[1]);

}

int32_t BulletDB::getField(int32_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
	
	ReadBytes(sizeof(Data));

	return (int32_t) ( (aBytes[0] << 24) | (aBytes[1] << 16) | (aBytes[2] << 8) | (aBytes[3]));

}

uint32_t BulletDB::getField(uint32_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
	
	ReadBytes(sizeof(Data));

	return (uint32_t) ( (aBytes[0] << 24) | (aBytes[1] << 16) | (aBytes[2] << 8) | (aBytes[3]));

}

float BulletDB::getField(float Data, uint8_t Field){
	float a;
	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
	
	ReadBytes(sizeof(Data));
	
	memcpy(&a, aBytes, sizeof(float));
	
	return a;
	
}

double BulletDB::getField(double Data, uint8_t Field){

	double a;

	Address = (CurrentRecord * RecordLength) + FieldStart[Field];
		
	ReadBytes(sizeof(Data));

	memcpy(&a, aBytes, sizeof(double));

	return a;
}

char  *BulletDB::getCharField(uint8_t Field){
	
	Address = (CurrentRecord * RecordLength) + FieldStart[Field];

	ReadBytes(BULLETDB_MAXCHARLEN);

	memset(stng,0,BULLETDB_MAXCHARLEN);
	
	memcpy(stng, aBytes, BULLETDB_MAXCHARLEN);
	
	return stng;
}


/////////////////////////////////////////////////
// getHeaderField()
// overloaded...
uint8_t BulletDB::getHeaderField(uint8_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];
	
	ReadBytes(sizeof(Data));
	
	return (uint8_t) aBytes[0];

}

int BulletDB::getHeaderField(int Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];

	ReadBytes(sizeof(Data));

	return (int) ( (aBytes[0] << 24) | (aBytes[1] << 16) | (aBytes[2] << 8) | (aBytes[3]));

}

int16_t BulletDB::getHeaderField(int16_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];
	
	ReadBytes(sizeof(Data));
	
	return (int16_t) (aBytes[0] << 8) | (aBytes[1]);

}

uint16_t BulletDB::getHeaderField(uint16_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];

	ReadBytes(sizeof(Data));

	return (uint16_t) (aBytes[0] << 8) | (aBytes[1]);

}

int32_t BulletDB::getHeaderField(int32_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];

	ReadBytes(sizeof(Data));

	return (int32_t) ( (aBytes[0] << 24) | (aBytes[1] << 16) | (aBytes[2] << 8) | (aBytes[3]));

}

uint32_t BulletDB::getHeaderField(uint32_t Data, uint8_t Field){

	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];
	
	ReadBytes(sizeof(Data));

	return (uint32_t) ( (aBytes[0] << 24) | (aBytes[1] << 16) | (aBytes[2] << 8) | (aBytes[3]));

}

float BulletDB::getHeaderField(float Data, uint8_t Field){
	
	float a;
	
	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];

	ReadBytes(sizeof(Data));
	
	memcpy(&a, aBytes, sizeof(float));
	
	return a;
	
	
}

double BulletDB::getHeaderField(double Data, uint8_t Field){

	double a;

	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];

	ReadBytes(sizeof(Data));
	
	memcpy(&a, aBytes, sizeof(double));

	return a;

}

char  *BulletDB::getCharHeaderField(uint8_t Field){
	
	Address = (CurrentRecord * RecordLength) + Header_FieldStart[Field];
	
	ReadBytes(sizeof(len));
	
	memset(stng,0,BULLETDB_MAXCHARLEN);
	
	memcpy(stng, bytes, len);
	
	return stng;
}


uint8_t BulletDB::getFieldDataType(uint8_t Index){
	return DataType[Index];
}

uint32_t BulletDB::getCurrentRecord(){
	return CurrentRecord;	
}

uint32_t BulletDB::getLastRecord(){
	return LastRecord;	
}

uint32_t BulletDB::getMaxRecords(){
	return MaxRecords;	
}


void BulletDB::setAddress(uint32_t Address){
	Address = Address;	
}

uint32_t BulletDB::getAddress(){
	return Address;	
}

void BulletDB::gotoRecord(uint32_t RecordNumber){	
	if (RecordNumber > MaxRecords){
		CurrentRecord = MaxRecords;
	}
	else {
		CurrentRecord = RecordNumber;
	}
}

void BulletDB::write_pause(void)
{
  if (flash_wait_for_write) {
    while (flash_read_status() & STAT_WIP);
    flash_wait_for_write = 0;
  }
}

unsigned char BulletDB::flash_read_status(void){
  // This can't do a write_pause
  digitalWrite(cspin, LOW);
  SPI.transfer(CMD_READ_STATUS_REG);
  c = SPI.transfer(0x00);
  digitalWrite(cspin, HIGH);
  return (c);
}

uint8_t BulletDB::ReadData() {
  SPI.beginTransaction(SPISettings(SPEED_READ, MSBFIRST, SPI_MODE0));
  digitalWrite(cspin, LOW);
  SPI.transfer(READ); // read instruction
  SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
  SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
  SPI.transfer((uint8_t) (Address & 0xFF));
  readvalue = SPI.transfer(0x00);
  digitalWrite(cspin, HIGH); 

  SPI.endTransaction();  
  
  flash_wait_for_write = 1;
  write_pause();

  Address = Address + 1;
  return readvalue;
  
}


void BulletDB::ReadBytes(uint8_t Length) {
	
  uint8_t i = 0;
  SPI.beginTransaction(SPISettings(SPEED_READ, MSBFIRST, SPI_MODE0));
  digitalWrite(cspin, LOW);
  SPI.transfer(READ); 
  SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
  SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
  SPI.transfer((uint8_t) (Address & 0xFF));
  
  for (i = 0; i < Length; i++){	  
	aBytes[i] = SPI.transfer(0x00);
  }
  digitalWrite(cspin, HIGH); 

  SPI.endTransaction();  
  
  flash_wait_for_write = 1;
  write_pause();
  Address = Address + Length;
  
}

/*

not used but maybe implement at some point
bool BulletDB::WriteBytes(uint8_t Array[], uint8_t Length) {

	uint8_t i = 0, LastByte = Length;
	bool PageSpan = false;
	pageOffset = Address % PAGE_SIZE;
	
	if ((pageOffset + Length) > PAGE_SIZE){
		PageSpan = true;		
		LastByte = PAGE_SIZE - pageOffset;
	}

	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	digitalWrite(cspin, LOW);
	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = 0; i < LastByte; i++){
		SPI.transfer(Array[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	// since we are writing byte by byte we need to advance address
	// writing byte arrays is unreliable--not sure why
	Address = Address + LastByte;
	
	if (!PageSpan){
		Address = Address + Length;
		return true;
	}
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	digitalWrite(cspin, LOW);
	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = LastByte; i < Length; i++){
		SPI.transfer(Array[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	// since we are writing byte by byte we need to advance address
	// writing byte arrays is unreliable--not sure why
	Address = Address + Length;
	
	return true;
}
*/
void BulletDB::WriteByte(uint8_t data) {
	
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE); // write instruction
	digitalWrite(cspin, HIGH); 
	SPI.endTransaction();
	
	flash_wait_for_write = 1;
	write_pause();
   
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITE); 
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));
	SPI.transfer(data);
 
	digitalWrite(cspin, HIGH);  
	SPI.endTransaction();

	flash_wait_for_write = 1; 
	write_pause();

	Address = Address + 1;

}

void BulletDB::saveField(uint8_t Bytes, uint8_t Field) {
		
	TempAddress = Address;
 	Address = CurrentRecord * RecordLength;
    Address = Address + FieldStart[Field];
		
	uint8_t i = 0, LastByte = Bytes;
	bool PageSpan = false;
	pageOffset = Address % PAGE_SIZE;
	
	if ((pageOffset + Bytes) > PAGE_SIZE){
		PageSpan = true;		
		LastByte = PAGE_SIZE - pageOffset;
	}

	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	
	digitalWrite(cspin, LOW);	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = 0; i < LastByte; i++){
		SPI.transfer(aBytes[i]);		
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + LastByte;
	
	if (!PageSpan){
		Address = Address + Bytes;
		return;
	}
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	digitalWrite(cspin, LOW);
	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = LastByte; i < Bytes; i++){
		SPI.transfer(aBytes[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + Bytes;

}

void BulletDB::saveField(uint8_t Array[], uint8_t Bytes, uint8_t Field) {
		
	TempAddress = Address;

	Address = CurrentRecord * RecordLength;
	Address = Address + FieldStart[Field];
		
	uint8_t i = 0, LastByte = Bytes;
	bool PageSpan = false;
	pageOffset = Address % PAGE_SIZE;
	
	if ((pageOffset + Bytes) > PAGE_SIZE){
		PageSpan = true;		
		LastByte = PAGE_SIZE - pageOffset;
	}

	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	
	digitalWrite(cspin, LOW);	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = 0; i < LastByte; i++){
		SPI.transfer(Array[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + LastByte;
	
	if (!PageSpan){
		Address = Address + Bytes;
		return;
	}
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	digitalWrite(cspin, LOW);
	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = LastByte; i < Bytes; i++){
		SPI.transfer(Array[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + Bytes;

}

void BulletDB::saveHeaderField(uint8_t Bytes, uint8_t Field) {

	TempAddress = Address;
 	Address = CurrentRecord * RecordLength;
    Address = Address + Header_FieldStart[Field];
		
	uint8_t i = 0, LastByte = Bytes;
	bool PageSpan = false;
	pageOffset = Address % PAGE_SIZE;
	
	if ((pageOffset + Bytes) > PAGE_SIZE){
		PageSpan = true;		
		LastByte = PAGE_SIZE - pageOffset;
	}

	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	
	digitalWrite(cspin, LOW);	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = 0; i < LastByte; i++){
		SPI.transfer(aBytes[i]);		
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + LastByte;
	
	if (!PageSpan){
		Address = Address + Bytes;
		return;
	}
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	digitalWrite(cspin, LOW);
	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = LastByte; i < Bytes; i++){
		SPI.transfer(aBytes[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + Bytes;

}

void BulletDB::saveHeaderField(uint8_t Array[], uint8_t Bytes, uint8_t Field) {
		
	TempAddress = Address;
	Address = CurrentRecord * RecordLength;
	Address = Address + Header_FieldStart[Field];
		
	uint8_t i = 0, LastByte = Bytes;
	bool PageSpan = false;
	pageOffset = Address % PAGE_SIZE;
	
	if ((pageOffset + Bytes) > PAGE_SIZE){
		PageSpan = true;		
		LastByte = PAGE_SIZE - pageOffset;
	}

	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	
	digitalWrite(cspin, LOW);	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = 0; i < LastByte; i++){
		SPI.transfer(Array[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + LastByte;
	
	if (!PageSpan){
		Address = Address + Bytes;
		return;
	}
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWrite(cspin, HIGH); 
	
	digitalWrite(cspin, LOW);
	
	SPI.transfer(WRITE);	
	SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
	SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
	SPI.transfer((uint8_t) (Address & 0xFF));	
	
	for (i = LastByte; i < Bytes; i++){
		SPI.transfer(Array[i]);
	}
	 
	digitalWrite(cspin, HIGH); 
	
	SPI.endTransaction();	
	
	flash_wait_for_write = 1; 
	write_pause();	

	Address = Address + Bytes;

}



//////////////////////////////////////////////////////////
