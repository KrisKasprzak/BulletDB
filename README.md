DO NOT USE THIS LIBRARY, IT'S UNDER CONSTRUCTION AND METHODS AND SUCH ARE CHANING DAILY

ALMOST DONE....

# BulletDB
Remember the infamous DBase III? Here it is for your MCU's. This library is a database system for SPI-based flash memory chips intended for microcontrollers like the mighty Teensy. This database driver uses a field/record approach in saving data to a chip. While you can save data to and SD card, the classic open/write/save has a huge overhead the can take in the 100's if ms to execute. This driver can save ~50 bytes in around 2ms. Data on the chip can be downloaded to and SD card for portable transfer to a PC. 

This driver is intended for data acquistion systems where known data is to be stored. As it uses a field / record approach, data variables are stored in fields, and each measurement is stored as a record. The intent is to save measurements such as volts in a volt field, temperature in a temp field, etc. Hence, it's not intended for saving video, images, or "random" data. 

If you are not familiar with fields and records, fields are the columns, and records are the rows. Similar to:

<table>
  <tr>
    <th>Record</th>
    <th>Time</th>
    <th>Temp1</th>
    <th>Temp2</th>
    <th>Temp2</th>
    
  </tr>
  <tr>
    <td>1</td>
    <td>10:00</td>
    <td>23.4</td>
    <td>45.2</td>
    <td>63.1</td>
  </tr>
  <tr>
    <td>2</td>
    <td>10:01</td>
    <td>23.6</td>
    <td>45.1</td>
    <td>65.4</td>
  </tr>
    <tr>
    <td>3</td>
    <td>10:02</td>
    <td>23.7</td>
    <td>45.0</td>
    <td>67.2</td>
  </tr>
</table>

This driver lets you create fields of specified data types, then in some measurement loop add a new record, save a record, and repeat. As with many flash chips you CANNOT write to an address unless it's in the erased state. This driver will find the next available writable address so if you power up your system, and start saving data, you can be sure you will be writing to valid addresses. The field definition process passes pointers into the library so the save process simpply looks at the data you already have in memory. This design keeps you from having to save a bunch of fields and the save performance hit and all the calls. Once saveRecord() is all that is needed to save all your data.
<br>
<b><h3>Goals</b></h3>
1. build a fast data storge system that writes to SPI type flash memory chips
2. operate like a database driver, add fields, addrecord, save record, etc
3. have ability to set a record and read a field for extracting data from the chip to save to an SD card
<br>
<b><h3>Library highlights</b></h3>
1. relatively small footprint
2. very fast write times (50 bytes in 1.6 ms)
3. ability to add up to 255 fields
4. ability to add a very special header to store settings such as calibration data.
5. ability to add a new record
6. ability to save a record with a single call
7. ability to goto a record and read fields
8. ability to get total records and start writing at the end
9. ability to add a "recordset" field to distinguish one read session from another. This mimics a file.
10. ability to save byte, int, long, float, char[fixed_length], doubles, more... But sorry STRING is not supported. 
11. ability to get chips stats (JDEC#, and used space)
12. ability to erase a sector or the entire chip
<br>
<b><h3>Library status</b></h3>
1. works and tested with Winbond W25Q64JVSSIQ
2. can write ~50 bytes in 1.5 ms
3. tested by writing 4mb and zero loss of data
<br>
<b><h3>General implementation</b></h3>
<br>
1. include the library
<br>
#include "BulletDB.h"
<br>
<br>
2. create variables
<br>
float MyVolts = 0.0;
<br>
int MyVoltsID = 0;
<br>
uint32_t LastRecord = 0, i = 0;
<br>
<br>
3. create data fields
<br>
MyVoltsID = SSD.addHeaderField("Volts", &MyVolts);
<br>
<br>
4. create the chip object
<br>
BulletDB YOUR_CHIP_OBJECT(THE_CHIP_SELECT_PIN);
<br>
<br>
5. In setup, get the last writable record
<br>
LastRecord = YOUR_CHIP_OBJECT.findLastRecord();
<br>
YOUR_CHIP_OBJECT.gotoRecord(LastRecord);
<br>
<br>
6. In some measurement loop
<br>
MyVolts = analogRead(A0);
<br>
<br>
7. Add a new record
<br>
YOUR_CHIP_OBJECT.addRecord();
<br>
<br>
8. Save the record
<br>
YOUR_CHIP_OBJECT.saveRecord();
<br>
<br>
9. when you are ready to read the data...
<br>
LastRecord = SSD.getLastRecord();
<br>
for (i = 1; i <= LastRecord; i++) {
<br>
&nbsp Serial.print("Record: ");
<br>
&nbsp Serial.print(i);
<br>
&nbsp Serial.print(" - ");
<br>
&nbsp Serial.print(YOUR_CHIP_OBJECT.getField(MyVolts, MyVoltsID ));
<br>
&nbsp Serial.print(", ");
<br>
}
<br>

<b><h3>ToDo...</b></h3>
1. test more
2. document the examples and code
3. Add and example on how to read data and write to and SD card)
5. create YouTube vid in how to use
6. test with other chips
7. remove hard code for chip size, pages, etc.

