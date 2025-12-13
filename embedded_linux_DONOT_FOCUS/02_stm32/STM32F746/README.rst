==================
How to write firmware
==================

1. Copy the folder: board/stmicroelectronics/stm32f746-disco to  board/stmicroelectronics/ 
2. Copy the configuration: stm32f746_disco_sd_defconfig to buildroot/configs/ 
3. Load the configuration to buildroot:: 
    $ make stm32f746_disco_sd_defconfig
4. Make the build::
    $ make

5. Under output/images/ directory: u-boot.bin file will be created. Copy the file and under windows drop it to Drive: that shows up.
6. Copy the output/images/sdcard.img file to memory card::
    $ sudo bmaptool copy --nobmap output/images/sdcard.img /dev/sdc

    If bmaptool is not installed, install it this way:
    $ sudo apt-get install bmaptool
    
    Also ensure that the drive is dorrect size using::
    $fdisk -l

7. Pop the SD card and boot the board.    


