DO NOT USE THIS LIBRARY, IT'S UNDER CONSTRUCTION AND METHODS AND SUCH ARE CHANING DAILY

ALMOST DONE....


# BulletDB
Remember the infamous DBase III? Here it is. A database system for SPI-based flash memory chips intended for microcontrollers like the mighty Teensy. This database driver uses a field/record approach in saving data to a chip. While you can save data to and SD card, the classic open/write/save has a huge overhead the can take in the 100's if ms to execute. This driver can save ~50 bytes in around 2ms. Data on the chip can be downloaded to and SD card for portable transfer to a PC. 

This driver is intended for data acquistion systems where known data is to be stored. As it uses a field / record approach, data variables are stored in fields, and each measurement is stored as a record. The intent is to save measurements such as volts in a volt field, temperature in a temp field, etc. Hence, it's not intended for saving video, images, or "random" data. 

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
    <td>1</td>
    <td>10:01</td>
    <td>23.6</td>
    <td>45.1</td>
    <td>65.4</td>
  </tr>
</table>

under construction

Goals
1. build a fast data storge system that writes to SPI type flash memory chips
2. operate like a database driver, add fields, addrecord, save record, etc
3. have ability to set a record and read a field for extracting data from the chip to save to an SD card

current status
1. works in basic test case
2. can write 58 bytes in 1.6 ms
3. add fields, save data, loop through records and save to SD


to do
1. test
2. document
3. remove some debug and test code
4. write examples
5. create YouTube vid in how to use
6. test with other chips
7. remove hard code for chip size, pages, etc.
