DO NOT USE THIS LIBRARY, IT'S UNDER CONSTRUCTION AND METHODS AND SUCH ARE CHANING DAILY

ALMOST DONE....


# BulletDB
Database system for SPI-based flash memory chips

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
