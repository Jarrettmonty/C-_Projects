///////////
//Readme.txt
//////////



AVR-based Alarm Clock (Finite State Machine, embedded C/C++) WIP
Implemented using the Arduino LiquidCrystal Library, all other code is compatible with C. 
Idea was to create a framework for future FSMs, and to get experience developing a time-critical system. 


--WORK IN PROGRESS-- 
//unfinished//
-this github repo
-Error switching from the alarm state 
-implementing an amplifier + speaker (vs piezo) for much louder alarms
-event handling through interrupts 

//to-do 
-currently limitless but particularly:
-seamless state additions/deletions 
-cleaner code -a better handler/buffer for LCD text 
-Porting my PIC 16x2 LCD driver to AVR

Have not explored exact run-time but will offer sub-linear state switching via pointer-to-function handling and states as function.
