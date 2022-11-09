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
	Serial.print(" Next Add: "); Serial.print(NextWritableAddress); 
	Serial.print(" c Rec: "); Serial.print(CurrentRecord); 
	Serial.print(" t Rec's: "); Serial.println(TotalRecords); 

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

	Return = readChipJEDEC();

	return Return;
}

 int16_t BulletDB::readChipJEDEC(){
	 
	uint8_t byteID[6];

	////////////////////////////////////////////////////////////////////////
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(JEDEC);
	
	byteID[0] = SPI.transfer(0x00);
	byteID[1] = SPI.transfer(0x00);
	byteID[2] = SPI.transfer(0x00);
  
	digitalWrite(cspin, HIGH);
	SPI.endTransaction();
	
	//Serial.println("Chip JEDEC"); 
	//Serial.print("byteID[0] "); Serial.println(byteID[0], HEX);
	//Serial.print("byteID[1] "); Serial.println(byteID[1], HEX);
	//Serial.print("byteID[2] "); Serial.println(byteID[2], HEX);

	
	if ((byteID[0] == 0) || (byteID[0] == NULL_RECORD)) {
		strcpy(ChipJEDEC,"INVALID CHIP");
		//return false;
	}
	if ((byteID[1] == 0) || (byteID[1] == NULL_RECORD)) {
		strcpy(ChipJEDEC,"INVALID CHIP");
		//return false;
	}
	if ((byteID[2] == 0) || (byteID[2] == NULL_RECORD)) {
		strcpy(ChipJEDEC,"INVALID CHIP");
		//return false;
	}
	
	sprintf(ChipJEDEC,"%02x:%02x:%02x",byteID[0],byteID[1],byteID[2]);
	//Serial.print("Chip data: ");Serial.println(ChipID);
	
	return true;
	
 }
 
 char *BulletDB::getChipJEDEC(){
	
	return ChipJEDEC;
	
}

int32_t BulletDB::readTotalRecords(){
	
	uint32_t StartAddress = 0;
	uint32_t  EndAddress = CARD_SIZE;
	uint32_t MiddleAddress = 0;
	bool Found = false;
	uint8_t RecType = 0;
	uint8_t NextRecType = 0;
	uint32_t Iteration = 0;
	
	// test start
	Address = StartAddress;
	RecType = ReadData();
	Address-=1;
		
	if (RecType == NULL_RECORD){
		// no DATA
		Serial.println("no data");
		NewCard = true;
		TotalRecords = 0;
		CurrentRecord = 0;
		Records = 0;
		Address = 0;
		NextWritableAddress = 0;
		UsedAddress = Address;
		//DebugData(134);
		ReadComplete = true;
		return CHIP_NEW;
	}
	
	// test end
	Address = EndAddress;
	RecType = ReadData();
	Address-=1;
	if (RecType != NULL_RECORD){
		// card full
		Serial.println("card full");
		TotalRecords = 0;
		CurrentRecord = 0;
		Address = CARD_SIZE;
		NextWritableAddress = CARD_SIZE;
		NewCard = false;
		ReadComplete = true;
		UsedAddress = CARD_SIZE;
		return CHIP_FULL;
	}
	
	// OK last record is somewhere in between...
	while(!Found) {
		Iteration++;
		
		//	Serial.print("Iteration: "); Serial.println(Iteration); 
			
		MiddleAddress = (EndAddress + StartAddress) / 2;
		Address = MiddleAddress;
		RecType = ReadData();
		NextRecType = ReadData();
		Address-=2;

	
		if ((RecType == NULL_RECORD) && (NextRecType == NULL_RECORD)){
			//Serial.println("Its lower");
			EndAddress = MiddleAddress;
		}
		if ((RecType != NULL_RECORD) && (NextRecType != NULL_RECORD)){
			//Serial.println("Its higher");
			StartAddress = MiddleAddress;
		}
		if ((RecType != NULL_RECORD) && (NextRecType == NULL_RECORD)){
			// we found the end
			//Serial.print("We found the end: "); Serial.println(Address);
			//Serial.print("Iterations: "); Serial.println(Iteration);
			Found = true;
		}
		
		if (Iteration > 2000) {
			Serial.println("timeout issue");
			NewCard = true;
			TotalRecords = 0;
			CurrentRecord = 0;
			Records = 0;
			Address = 0;
			NextWritableAddress = 0;
			UsedAddress = Address;
			//DebugData(134);
			ReadComplete = true;
			return CHIP_FORCE_RESTART;
		}
			
	}
	
	/*
	TempAddress = Address;
	for (Address = 0; Address < 300; Address+=RecordLength){
		RecType = ReadData();
		Address--;
		Serial.print("Address ");Serial.print(Address );
		Serial.print(" data ");Serial.println(RecType );
	
	}

	Address = TempAddress;
	*/
	// end of record, so bump 1 byte to get to 0 byte of next record
	Address++;
	
	UsedAddress =  Address;
	NextWritableAddress = Address;
	
	TotalRecords = (Address / RecordLength);
	//Serial.print("Address ");Serial.println(Address );
	//Serial.print("RecordLength ");Serial.println(RecordLength );
	//Serial.print("TotalRecords ");Serial.println(TotalRecords );
	ReadComplete = true;
	NewCard = false;
	//DebugData(199);
	return TotalRecords;
	
}

// data field addField methods
uint8_t BulletDB::addField(const char *FieldName, uint8_t *Data) {
		
	if (FieldCount == MAX_FIELDS){
		return 0;
	}
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
	if (FieldCount == MAX_FIELDS){
		return 0;
	}
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
	if (FieldCount == MAX_FIELDS){
		return 0;
	}
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
	if (FieldCount == MAX_FIELDS){
		return 0;
	}

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
	if (FieldCount == MAX_FIELDS){
		return 0;
	}
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
	if (FieldCount == MAX_FIELDS){
		return 0;
	}
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
	if (FieldCount == MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_FLOAT;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	fdata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}

uint8_t BulletDB::addField(const char *FieldName, double *Data) {
	if (FieldCount == MAX_FIELDS){
		return 0;
	}
	DataType[FieldCount] = DT_DOUBLE;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  sizeof(*Data);
	RecordLength = RecordLength + sizeof(*Data);
	strcpy(fieldname[FieldCount], FieldName);
	ddata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}


uint8_t BulletDB::addField(const char *FieldName, char *Data, uint8_t len) {

	DataType[FieldCount] = DT_CHAR;
	FieldStart[FieldCount] = RecordLength;
	FieldLength[FieldCount] =  len;
	RecordLength = RecordLength + len;
	strcpy(fieldname[FieldCount], FieldName);
	cdata[FieldCount] = Data;
	FieldCount++;
	return FieldCount - 1;
}


// header addHeaderData methods
uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, uint8_t *Data) {
	
	
	Header_DataType[Header_FieldCount] = DT_U8;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	u8hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
	
	}
	
uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, int *Data) {
	
	Header_DataType[Header_FieldCount] = DT_INT;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	inthdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}
	
uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, int16_t *Data) {

	Header_DataType[Header_FieldCount] = DT_I16;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	i16hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
	
}

uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, uint16_t *Data) {

	Header_DataType[Header_FieldCount] = DT_U16;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	u16hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}


uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, int32_t *Data) {

	Header_DataType[Header_FieldCount] = DT_U32;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	i32hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}

uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, uint32_t *Data) {

	Header_DataType[Header_FieldCount] = DT_U32;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	u32hdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}
uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, float *Data) {

	Header_DataType[Header_FieldCount] = DT_FLOAT;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	fhdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	return Header_FieldCount - 1;
}
uint8_t BulletDB::addHeaderField(const char *HeaderFieldName, double *Data) {

	Header_DataType[Header_FieldCount] = DT_FLOAT;
	Header_FieldStart[Header_FieldCount] = Header_RecordLength;
	Header_FieldLength[Header_FieldCount] =  sizeof(*Data);
	Header_RecordLength = Header_RecordLength + sizeof(*Data);
	strcpy(Header_FieldName[Header_FieldCount], HeaderFieldName);
	dhdata[Header_FieldCount] = Data;
	Header_FieldCount++;
	
	return Header_FieldCount - 1;
		
}

//

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
		Serial.print(", Name: ");
		Serial.print(Header_FieldName[i]);
		Serial.print(", type: ");
		if( DataType[i] == DT_U8){
			Serial.print("uint8_t");
		}
		else if( DataType[i] == DT_INT){
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
	
	Serial.print("Record length: ");Serial.println(RecordLength);
}



int32_t BulletDB::gotoLastRecord(){
	
	
	if (TotalRecords == 0){
			Address = 0;
		return Address;
	}
	else{
		Address = (TotalRecords - 1)  * RecordLength;
		return (Address);
	}
	
	return 0;
		
}


int32_t BulletDB::getLastRecord(){
	
	if (TotalRecords == 0){
		return 0;
	}
	else{
		return (TotalRecords -1);
	}
	
	return 0;
	
}

uint8_t BulletDB::getFieldCount(){
	return FieldCount;
}

uint8_t BulletDB::getHeaderFieldCount(){
	return Header_FieldCount;
}

char * BulletDB::getFieldName(uint8_t Index){
	return fieldname[Index];
}

char * BulletDB::getHeaderFieldName(uint8_t Index){
	return Header_FieldName[Index];
}

uint16_t BulletDB::getRecordLength(){
	return RecordLength;
	
}

uint16_t BulletDB::getHeaderRecordLength(){
	return Header_RecordLength;
	
}

uint32_t BulletDB::getUsedSpace(){
	return UsedAddress;
	
}

uint32_t BulletDB::getTotalSpace(){
	return CARD_SIZE;
}


/*
void BulletDB::write_pause(void){
	
	bt = millis();
	while (true){

		// This can't do a write_pause
		digitalWriteFast(cspin, LOW);
		SPI.transfer(CMD_READ_STATUS_REG);
		c = SPI.transfer(0x00);
		digitalWriteFast(cspin, HIGH);
		Serial.print(c);		Serial.print("-");		Serial.println(STAT_WIP);
		
		if ((millis() - bt) > 2000){
			// runaway time
			Serial.println("997 timeout");
			return;
		}
		if (c == STAT_WIP){
			Serial.println("c = STAT, OK");
			return;
		}
	}
}

*/





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

uint32_t BulletDB::addRecord(){
	
	Records++;
	CurrentRecord++;
	TotalRecords++;

	// special case for address = 0 (new card)
	// write to address 0, otherwise advance address to next record
	// save record does not advance address it returns to it's initial address
	
	if (NewCard == true){
		NewCard = false;
		//Serial.println("header adding nothing to address");
		//DebugData(639);
		return CurrentRecord;		

	}

	
	Address = Address + RecordLength;

	return CurrentRecord;
}


void BulletDB::dumpBytes(uint8_t RecLen, uint32_t Recs) {
	
	TempAddress = Address;
	Address = 0;
	uint32_t rec = 0;
	uint8_t data = 0;
	Serial.println("Dump bytes-------------------- "); 
	for (i = 0; i < (Recs* RecordLength); i += RecordLength) {
		Serial.print("Address "); Serial.print(Address); Serial.print(" - ");
		Serial.print(", record "); Serial.print(rec); Serial.print(" - ");
		rec++;
		
		for (j = 0; j< RecLen; j++){
			data = ReadData();
			
			if (j == 0){
				if (data == NULL_RECORD){
					//Serial.println("END OF DATA.\r\r");
					//return;
				}
			}
			Serial.print(data);
			Serial.print("-");
		}
		Serial.println("");
	}
	
	Address = TempAddress;
}
	
uint32_t BulletDB::saveRecord() {
	
	// we absoutely must have TotalRecords otherwise
	// we will not know where the first writable address
	// is, user can call this to manage the small performance
	// from this call, but if they forget to call it. it must be called
	// before we start writing
	// note that the performance hit is usually 20microseconds
	
	//DebugData(778);

	if (!ReadComplete){
		//unsigned long st = micros();
		Serial.print("readTotalRecords not done. so doing it now...");
		//Serial.println(micros() - st);
		
		readTotalRecords();
		//Serial.print("readTotalRecords ");Serial.println(TotalRecords);
		ReadComplete = true;
	}
	if (NextWritableAddress == CARD_SIZE){
		// no room
		return CARD_SIZE;
	}	
	
	Address = NextWritableAddress;
	//DebugData(800);
	
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
		else if (DataType[i] == DT_DOUBLE){		
			DoubleToBytes(a8Bytes, *ddata[i]);
			WriteData(a8Bytes[0]);
			WriteData(a8Bytes[1]);
			WriteData(a8Bytes[2]);
			WriteData(a8Bytes[3]);	
			WriteData(a8Bytes[4]);
			WriteData(a8Bytes[5]);
			WriteData(a8Bytes[6]);
			WriteData(a8Bytes[7]);	
		}		
	
		else if (DataType[i] == DT_CHAR){
				
			strcpy(buf,cdata[i]);
			
			for (j = 0; j < MAXDATACHARLEN; j ++){
				WriteData(buf[j]);	
			//Serial.print(j);Serial.print("-"); Serial.println(buf[j]);				
			}		
		}
	}


	Address = TempAddress;
	
	// now that record is written, bump the address to the next writable
	// address
	Records++;
	CurrentRecord++;
	TotalRecords++;

	Address = NextWritableAddress + RecordLength;
	NextWritableAddress	= Address;
	
	//DebugData(865);
	return CurrentRecord;
	
}

uint32_t BulletDB::saveHeader() {


	if (!ReadComplete){
		//unsigned long st = micros();
		//Serial.print("readTotalRecords not done. so doing it now...");
		//Serial.println(micros() - st);
		
		readTotalRecords();
		//Serial.print("readTotalRecords ");Serial.println(TotalRecords);
		ReadComplete = true;
	}
	if (NextWritableAddress == CARD_SIZE){
		// no room
		return CARD_SIZE;
	}	
	
	
	
	
	TempAddress = Address;
	//Serial.print("TempAddress ");Serial.println(TempAddress);
	//Serial.println(Address);
	
	for (i= 0; i < Header_FieldCount; i++){
		
		//Serial.print("Writing header for: ");Serial.print(Header_FieldName[i]);
		
		if (Header_DataType[i] == DT_U8){
			//Serial.print(", 864 ");Serial.println(*u8hdata[i]);
			WriteData(*u8hdata[i]);
		}
		else if (Header_DataType[i] == DT_INT){
		
			//Serial.print(", 869 ");Serial.println(*inthdata[i]);
			Header_DataType[Header_FieldCount] = DT_INT;
			B4ToBytes(a4Bytes, *inthdata[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);
			
		}
		else if (Header_DataType[i] == DT_I16){
			//Serial.print(", 879 ");Serial.println(*i16hdata[i]);
			B2ToBytes(a2Bytes, *i16hdata[i]);
			WriteData(a2Bytes[0]);
			WriteData(a2Bytes[1]);
		}
		else if (Header_DataType[i] == DT_U16){
			//Serial.print(", 885 ");Serial.println(*u16hdata[i]);
			B2ToBytes(a2Bytes, *u16hdata[i]);
			WriteData(a2Bytes[0]);
			WriteData(a2Bytes[1]);
			
		}
		else if (Header_DataType[i] == DT_I32){
			//Serial.print(", 892 ");Serial.println(*i32hdata[i]);
			B4ToBytes(a4Bytes, *i32hdata[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);
		}
		else if (Header_DataType[i] == DT_U32){
			//Serial.print(", 900 ");Serial.println(*u32hdata[i]);
			B4ToBytes(a4Bytes, *u32hdata[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);
		}
		else if (Header_DataType[i] == DT_FLOAT){
			//Serial.print(", 908 ");Serial.println(*fhdata[i]);
			FloatToBytes(a4Bytes, *fhdata[i]);
			WriteData(a4Bytes[0]);
			WriteData(a4Bytes[1]);
			WriteData(a4Bytes[2]);
			WriteData(a4Bytes[3]);	

		}
		else if (Header_DataType[i] == DT_DOUBLE){
			//Serial.print(", 917 ");Serial.println(*dhdata[i]);
			DoubleToBytes(a8Bytes, *dhdata[i]);
			WriteData(a8Bytes[0]);
			WriteData(a8Bytes[1]);
			WriteData(a8Bytes[2]);
			WriteData(a8Bytes[3]);	
			WriteData(a8Bytes[4]);
			WriteData(a8Bytes[5]);
			WriteData(a8Bytes[6]);
			WriteData(a8Bytes[7]);	
		}		
	}


	Address = TempAddress;
	
	// now that record is written, bump the address to the next writable
	// address
	Records++;
	CurrentRecord++;
	TotalRecords++;

	Address = NextWritableAddress + RecordLength;
	NextWritableAddress	= Address;
	
	//DebugData(865);
	return CurrentRecord;
	
}

void BulletDB::eraseAll(){
	
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	
	//Serial.println("Erasing chip");
	flash_wait_for_write = 1;
	write_pause();
	digitalWriteFast(cspin, LOW);
	SPI.transfer(WRITEENABLE);
	digitalWriteFast(cspin, HIGH);
	write_pause();
	delay(25);
	digitalWriteFast(cspin, LOW);
	SPI.transfer(CHIPERASE);
	digitalWriteFast(cspin, HIGH);
	flash_wait_for_write = 1;
	write_pause();
	SPI.endTransaction();
	NewCard = true;
	readTotalRecords();
	Address = 0;
	NextWritableAddress = 0;
	UsedAddress = 0;

}
	
void BulletDB::erasePage(uint32_t PageNumber){

	Address = PageNumber * PAGE_SIZE;
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE); // write instruction
	digitalWrite(cspin, HIGH);
	SPI.endTransaction();
	flash_wait_for_write = 1;
	write_pause();
	
	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(CMD_SECTOR_ERASE); // write instruction
	SPI.transfer((Address >> 16) & 0xFF);
	SPI.transfer((Address >> 8) & 0xFF);
	SPI.transfer(Address & 0xFF);
	digitalWrite(cspin, HIGH);
	write_pause();
	SPI.endTransaction();
	flash_wait_for_write = 1;
	
	
}


//////////////////////////////////////////////////////////////////////////////////
// get data
	
uint8_t BulletDB::getField(uint8_t Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + FieldStart[Field];
	a1Byte = ReadData();
	Address = TempAddress;
	return (uint8_t) a1Byte;

}

int BulletDB::getField(int Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + FieldStart[Field];
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return (int) ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

int16_t BulletDB::getField(int16_t Data, uint8_t Field){

	TempAddress = Address;
	Address = Address + FieldStart[Field];
	a2Bytes[0] = ReadData();
	a2Bytes[1] = ReadData();
	Address = TempAddress;
	return (int16_t) (a2Bytes[0] << 8) | (a2Bytes[1]);

}

uint16_t BulletDB::getField(uint16_t Data, uint8_t Field){

	TempAddress = Address;
	Address = Address + FieldStart[Field];
	a2Bytes[0] = ReadData();
	a2Bytes[1] = ReadData();
	Address = TempAddress;
	return (uint16_t) (a2Bytes[0] << 8) | (a2Bytes[1]);

}

int32_t BulletDB::getField(int32_t Data, uint8_t Field){

	TempAddress = Address;
	Address = Address + FieldStart[Field];
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return (int32_t) ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

uint32_t BulletDB::getField(uint32_t Data, uint8_t Field){

	TempAddress = Address;
	Address = Address + FieldStart[Field];
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return (uint32_t) ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

float BulletDB::getField(float Data, uint8_t Field){

	TempAddress = Address;
	Address = Address + FieldStart[Field];
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData();	
	Address = TempAddress;	
	return *(float *)a4Bytes;
}

double BulletDB::getField(double Data, uint8_t Field){

	TempAddress = Address;
	Address = Address + FieldStart[Field];
	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData();	
	Address = TempAddress;	
	return *(double *)a4Bytes;
}

char  *BulletDB::getCharField(uint8_t Field){
	
	TempAddress = Address;
	Address = Address + FieldStart[Field];
	uint8_t len = MAXCHARLEN;
	
	int8_t bytes[len];
	
	for (i = 0; i < len;i++){
		bytes[i] = ReadData();
	}
	
	memset(stng,0,MAXDATACHARLEN);
	
	memcpy(stng, bytes, len);
	//Serial.print("1101 cahr "); Serial.println(stng);
	
	Address = TempAddress;

	return stng;
}


/////////////////////////////////////////////////
// getHeaderField()
// overloaded...
uint8_t BulletDB::getHeaderField(uint8_t Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];

	a1Byte = ReadData();
	Address = TempAddress;
	return (uint8_t) a1Byte;

}

int BulletDB::getHeaderField(int Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];

	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return (int) ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

int16_t BulletDB::getHeaderField(int16_t Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];

	a2Bytes[0] = ReadData();
	a2Bytes[1] = ReadData();
	Address = TempAddress;
	return (int16_t) (a2Bytes[0] << 8) | (a2Bytes[1]);

}

uint16_t BulletDB::getHeaderField(uint16_t Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];

	a2Bytes[0] = ReadData();
	a2Bytes[1] = ReadData();
	Address = TempAddress;
	return (uint16_t) (a2Bytes[0] << 8) | (a2Bytes[1]);

}

int32_t BulletDB::getHeaderField(int32_t Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];

	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return (int32_t) ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

uint32_t BulletDB::getHeaderField(uint32_t Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];

	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData(); //6
	Address = TempAddress;
	return (uint32_t) ( (a4Bytes[0] << 24) | (a4Bytes[1] << 16) | (a4Bytes[2] << 8) | (a4Bytes[3]));

}

float BulletDB::getHeaderField(float Data, uint8_t Field){

	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];

	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData();	
	Address = TempAddress;	
	return *(float *)a4Bytes;
}

double BulletDB::getHeaderField(double Data, uint8_t Field){

	TempAddress = Address;
	
	Address = Address + Header_FieldStart[Field];

	a4Bytes[0] = ReadData();
	a4Bytes[1] = ReadData();
	a4Bytes[2] = ReadData();
	a4Bytes[3] = ReadData();	
	Address = TempAddress;	
	return *(double *)a4Bytes;
}

char  *BulletDB::getCharHeaderField(uint8_t Field){
	
	TempAddress = Address;

	Address = Address + Header_FieldStart[Field];
	
	uint8_t len = MAXCHARLEN;
	
	int8_t bytes[len];
	for (i = 0; i < len;i++){
		bytes[i] = ReadData();
	}
	
	memset(stng,0,MAXDATACHARLEN);
	
	memcpy(stng, bytes, len);
	
	Address = TempAddress;

	return stng;
}

//


uint8_t BulletDB::getFieldDataType(uint8_t Index){
	return DataType[Index];
	
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


void BulletDB::setAddress(uint32_t Address){
	Address = Address;
	
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

void BulletDB::write_pause(void)
{
  if (flash_wait_for_write) {
    while (flash_read_status() & STAT_WIP);
    flash_wait_for_write = 0;
  }
}

unsigned char BulletDB::flash_read_status(void)

{
  
  // This can't do a write_pause
  digitalWrite(cspin, LOW);
  SPI.transfer(CMD_READ_STATUS_REG);
  c = SPI.transfer(0x00);
  digitalWrite(cspin, HIGH);
  return (c);
}

uint8_t BulletDB::ReadData() {
	
	
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

	//write_pause();

	SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
	digitalWrite(cspin, LOW);
	SPI.transfer(WRITEENABLE); // write instruction
	digitalWrite(cspin, HIGH); 
	SPI.endTransaction();
	
	flash_wait_for_write = 1;
	write_pause();
  
 
 
 
 
  SPI.beginTransaction(SPISettings(SPEED_WRITE, MSBFIRST, SPI_MODE0));
  digitalWrite(cspin, LOW);
  SPI.transfer(WRITE); // write instruction
  SPI.transfer((uint8_t) ((Address >> 16) & 0xFF));
  SPI.transfer((uint8_t) ((Address >> 8) & 0xFF));
  SPI.transfer((uint8_t) (Address & 0xFF));
  SPI.transfer(data);
  //SPI.transfer(255); // data sheet says to write this...   
  digitalWrite(cspin, HIGH);  
  SPI.endTransaction();
  
   flash_wait_for_write = 1; 
  write_pause();



 
  Address = Address + 1;
  UsedAddress = Address;


}


//////////////////////////////////////////////////////////
