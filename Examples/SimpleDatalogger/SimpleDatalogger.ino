#include "BulletDB.h"
#define SSD_PIN 6
#define SD_PIN 10
char CharTestData[MAXDATACHARLEN];
uint8_t rID = 0, rPoint = 0, rA0Volts = 0, rA1Volts = 0;
uint8_t rD2State = 0, rCharTest = 0;;
uint16_t b0Bits = 0, b1Bits = 0;
float A0Volts = 0, A1Volts = 0;
uint8_t D2State = 0;
uint32_t Counter = 0, oldTime = 0;
uint16_t Time = 0;
uint32_t Point = 0, Goal = 0;
bool StartLogging = false;
int16_t recID = 0;
int32_t Ret = 0;

BulletDB SSD(SSD_PIN);

void setup() {

  Serial.begin(115200);
  while (!Serial) {}



  Ret = SSD.init();

  if (Ret) {
    Serial.println("Chip found");
  }
  else {
    Serial.println("Chip may be invalid");
  }


  rCharTest = SSD.addField("Name", CharTestData, sizeof(CharTestData));
  rID = SSD.addField("Set ID", &recID);
  rPoint = SSD.addField("Point", &Point);
  rA0Volts = SSD.addField("A0 Volts", &A0Volts);
  rA1Volts = SSD.addField("A1 Volts", &A1Volts);
  rD2State = SSD.addField("Pin 2", &D2State);

  Ret = SSD.readTotalRecords();

  if (Ret == CHIP_NEW) {
    Serial.println("Unused chip, no records");
  }
  else if (Ret == CHIP_FULL) {
    Serial.println("Chip is full.");
  }
  else {
    Serial.print("Records found: "); Serial.println(Ret);
  }

  DrawMenu();

}

void loop() {

  CheckKeyPress();

  if ((StartLogging ) && (millis() - oldTime) > Time) {

    Point++;

    b0Bits = analogRead(A0);
    b1Bits =  analogRead(A1);
    D2State = digitalRead(2);

    A0Volts = (b0Bits * 3.3) / 1024.0;
    A1Volts = (b1Bits * 3.3) / 1024.0;

    Serial.print("Saving ID: "); Serial.print(recID);
    Serial.print(", Point: "); Serial.print(Point);

    //Serial.print("creating string ");
    sprintf(CharTestData, "Record %04d", (int) Point);
    //Serial.println(CharTestData);

    unsigned long st = micros();
    SSD.addRecord();
    SSD.saveRecord();
    Serial.print(". Save time us ");Serial.println(micros() - st);

    if (Point >= Goal) {
      StartLogging = false;
      Serial.println("Complete");
      DrawMenu();
    }

    oldTime = millis();

  }

}

void CheckKeyPress() {

  if ( Serial.available() ) {

    char key;
    key = Serial.read();
    switch (key) {
      case 'M':
        StartLogging = false;
        DrawMenu();
        break;
      case 'L':
        StartLogging = false;
        ChipStats();
        DrawMenu();
        break;
      case 'I':
        StartLogging = false;
        SSD.listFields();
        DrawMenu();
        break;
      case 'E':
        StartLogging = false;
        Serial.println("This may take a few minutes...");
        SSD.eraseAll();
        recID = 1;
        Serial.println("Erase complete");
        DrawMenu();
        break;
      case 'P':
        StartLogging = false;
        PrintRecords();
        DrawMenu();
        break;
      case 'F':
        Serial.println("Logging...");
        Point = 0;
        recID++;
        Time = 10;
        Goal = 1000;
        SSD.gotoLastRecord();
        StartLogging = true;
        break;
      case 'S':
        Serial.println("Logging...");
        Point = 0;
        recID++;
        Time = 1000;
        Goal = 10;
        StartLogging = true;
        Serial.println(SSD.getLastRecord());
        SSD.gotoLastRecord();
        break;
      case 'D':
        //DownloadData();
        break;
      case 'Z':
        SSD.dumpBytes();
        break;
    }

    while (Serial.read() != -1) ; // remove rest of characters.
  }
}

void DrawMenu() {

  Serial.println("____________BulletDB Demo menu");
  Serial.println("\r\r\r");
  Serial.println("Enter(M) for main menu");
  Serial.println("Enter(L) for list chip stats");
  Serial.println("Enter(I) for field list");
  Serial.println("Enter(E) to erase chip");
  Serial.println("Enter(P) to print records to Serial monitor");
  Serial.println("Enter(F) for logging 1000 data points every 10 ms");
  Serial.println("Enter(S) for logging 10 data points every 1 s");
  Serial.println("Enter(D) for download data to and SD card");

}

void ChipStats() {

  Serial.println("Chip Stats");
  Serial.println("\r\r\r");
  Serial.print("Chip ID: "); Serial.println(SSD.getChipID() );
  Serial.print("Used Space (b): "); Serial.println(SSD.getUsedSpace() );
  Serial.print("Total Space (b): "); Serial.println(SSD.getTotalSpace() );
  Serial.print("Total Records "); Serial.println(SSD.getTotalRecords() );
  Serial.print("Number of fields: "); Serial.println(SSD.getFieldCount() );
  Serial.print("Record length: "); Serial.println(SSD.getRecordLength() );
  Serial.print("Address or last real record: "); Serial.println(SSD.getLastRecord() );
  Serial.println("\r\r");
  Serial.println("Enter(M) for main menu\r\r");

}

void PrintRecords() {

  uint32_t i = 0, j = 0;
  uint32_t cr = SSD.getCurrentRecord();

  SSD.gotoAbsoluteFirstRecord();

  // print field headers
  for (j = 0; j < SSD.getFieldCount(); j++) {
    Serial.print(SSD.getFieldName(j));
    Serial.print(", ");
  }
  Serial.println(" ");
  Serial.println(SSD.getTotalRecords());
  Serial.println(" ");

  // print records
  for (i = 0; i < SSD.getTotalRecords(); i++) {

    SSD.gotoRecord(i);
    Serial.print(i);
    Serial.print(", ");
    Serial.print(SSD.getCharField(rCharTest));
    Serial.print(", ");
    Serial.print(SSD.getField(recID, rID));
    Serial.print(", ");
    Serial.print(SSD.getField(Point, rPoint));
    Serial.print(", ");
    Serial.print(SSD.getField(A0Volts, rA0Volts), 5);
    Serial.print(", ");
    Serial.print(SSD.getField(A1Volts, rA1Volts), 5);
    Serial.print(", ");
    Serial.print(SSD.getField(D2State, rD2State));
    Serial.println(" ");
  }
  Serial.println("Record printing complete.");

  SSD.gotoRecord(cr);

}
