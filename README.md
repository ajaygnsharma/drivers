# uClinux Device Drivers
This repository contains uClinux Device drivers for temperature sensors. The board used is LPC2468 ARM7 based from Embedded Artists at [www.embeddedartists.com](https://www.embeddedartists.com/products/lpc2468-developers-kit/)

 There are two temperature sensors: 
 1. TMP102 - This supports I2C protocol. 
 2. TMP36 - This needs to be interfaced to an ADC MCP3002 which supports SPI protocol.
 
 #### Why is this useful?
Because someone can create this layout and interface to any other Embedded Linux board. 

There are many documentation available in the repository. Besides below link also includes schematic. 

#### TMP102 Circuit Diagram
This is simple reference schematic from SparkFun. 
[TMP102 schematic](https://cdn.sparkfun.com/datasheets/Sensors/Temp/SparkFun_Digital_Temperature_Sensor_Breakout_-_TMP102.pdf)

#### TMP36 Circuit Diagram
The board for TMP36 is shown here. The schematic is also available on request. Let me know.
![TMP36 schematic](https://github.com/ajaygnsharma/drivers/blob/master/doc/spi/spi_driver_board.png "Schematic")

