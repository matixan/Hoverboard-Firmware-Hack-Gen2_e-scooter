This fork of Hoverboard Firmware Generation 2 for generic hoverboards using two mainboards has the following changes compared to the original firmware:
````
- Seperate control of each motor
- Added PID controllers allowing for each motor to be controlled using a real speed instead of a power level (P, I and D gains still have to be tweaked better, power buildup at lower speeds is pretty slow)
- All important data is now send to the steering devices (battery voltage, master and slave current and master and slave realSpeed)
````

To control the hoverboard drivers, a development board, like for example an Arduino (3.3V logic levels!!!) has to be connected to the master controller using a UART port. It's recommended to have a board with multiple UART ports like for example the Arduino Due (connected directly) or Arduino Mega (using logic level converters). Check out the reversed-engineered schematics for "REMOTE" down below. 

The steering signal consists out of 8 bytes and is as follows seperated by colons: 
'/', <leftSpeed_MSbyte>, <leftSpeed_LSbyte>, <rightSpeed_MSbyte>, <rightSpeed_LSbyte>, <CRC_MSbyte>, <CRC_LSbyte>, '\n'. 
LeftSpeed and rightSpeed are 16 bit signed integer values and the function for generating the 16 bit CRC can be found in HoverBoardGigaDevice/src/comms.c.

The data the master controller sends consists out 24 bytes and is as follows seperated by colons: 
'/', <batteryVoltage_byte1>, <batteryVoltage_byte2>, <batteryVoltage_byte3>, <batteryVoltage_byte4>, <currentMaster_byte1>, <currentMaster_byte2>, <currentMaster_byte3>, <currentMaster_byte4>, <speedMaster_byte1>, <speedMaster_byte2>, <speedMaster_byte3>, <speedMaster_byte4>, <currentSlave_byte1>, <currentSlave_byte2>, <currentSlave_byte3>, <currentSlave_byte4>, <speedSlave_byte1>, <speedSlave_byte2>, <speedSlave_byte3>, <speedSlave_byte4>, <CRC_MSbyte>, <CRC_LSbyte>, '\n'. 
BatteryVoltage, currentMaster, speedMaster, currentSlave and speedSlave are 4 byte float values. The typedef (FLOATUNION_t) in C for converting these bytes to float variables can be found in HoverBoardGigaDevice/inc/defines.h. 

It is also possible to retrieve the same data from the slave controller but only with one value at a time. This called the "BluetoothComms" in the source code. Some settings can also be changed this way like for example enabling or disabling the beeping when driving backwards or changing the LED colors or blinking speed. To retrieve data or to change some settings, a data packet of 11 bytes has to be send. This packet consists out of the following data:
'/', <digit1>, <digit2>, <readWrite>, <sign>, <digit3>, <digit4>, <digit5>, '\n'.
You can either read or write data to the by setting the readWrite byte respectively to '0' or '1'. When in read mode, the first two digits are used as an identifier for which data to read. <digit1> are the decades and <digit2> the units. The function for the identifier can be found in the first switch statement in HoverBoardGigaDevice/src/commsBluetooth.c. 
When in write mode the first two digits work the same as for the read mode but the corresponding function can be found in the second switch statement. <digit3>, <digit4> and <digit5> are respectively the hundreds, the decades and the units for the value that has to be written. 
All bytes in the data packets have to be ASCII values, so not a value represented by a decimal number.

#### Updates:
````
- Firmware is ready.
- Sadly I can not support any issues. I'm not a student, less free time :)
````

### Hoverboard-Firmware-Hack-Gen2

Hoverboard Hack Firmware Generation 2 for the Hoverboard with the two Mainboards instead of the Sensorboards (See Pictures).

This repo contains open source firmware for generic Hoverboards with two mainboards. It allows you to control the hardware of the new version of hoverboards (like the Mainboard, Motors and Battery) with an arduino or some other steering device for projects like driving armchairs.

The structure of the firmware is based on the firmware hack of Niklas Fauth (https://github.com/NiklasFauth/hoverboard-firmware-hack/). Because of a different model of processor (GD32F130C8 instead of STM32F103) it was not possible to use the same firmware and it has to be written from scratch (Different hardware, different, pins, different registers :( )

- This project requires knowledge of the initial project linked above.
- At the current point I am not able to support any questions or issues - sorry!

---

#### Hardware
![otter](https://github.com/flo199213/Hoverboard-Firmware-Hack-Gen2/blob/master/Hardware_Overview_small.png)

The hardware has two main boards, which are different equipped. They are connected via USART. Additionally there are some LED PCB connected at X1 and X2 which signalize the battery state and the error state. There is an programming connector for ST-Link/V2 and they break out GND, USART/I2C, 5V on a second pinhead.

The reverse-engineered schematics of the mainboards can be found here:
https://github.com/flo199213/Hoverboard-Firmware-Hack-Gen2/blob/master/Schematics/HoverBoard_CoolAndFun.pdf


---

#### Flashing
The firmware is built with Keil (free up to 32KByte). To build the firmware, open the Keil project file which is includes in repository. Right to the STM32, there is a debugging header with GND, 3V3, SWDIO and SWCLK. Connect GND, SWDIO and SWCLK to your SWD programmer, like the ST-Link found on many STM devboards.

- If you never flashed your mainboard before, the controller is locked. To unlock the flash, use STM32 ST-LINK Utility or openOCD.
- To flash the STM32, use the STM32 ST-LINK Utility as well, ST-Flash utility or Keil by itself.
- Hold the powerbutton while flashing the firmware, as the controller releases the power latch and switches itself off during flashing
