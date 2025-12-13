.PHONY: led key eeprom calibrate

all: led key eeprom calibrate

led: led.c
	../../../../tools/ucfront-gcc arm-elf-gcc  -Os -fomit-frame-pointer -fno-common -fno-builtin -Wall -DEMBED -Dlinux -D__linux__ -Dunix -D__uClinux__ -Wl,-elf2flt="-r" -o led led.c

key: key.c
	../../../../tools/ucfront-gcc arm-elf-gcc  -Os -fomit-frame-pointer -fno-common -fno-builtin -Wall -DEMBED -Dlinux -D__linux__ -Dunix -D__uClinux__ -Wl,-elf2flt="-r" -o key key.c

eeprom: eeprom.c
	../../../../tools/ucfront-gcc arm-elf-gcc  -Os -fomit-frame-pointer -fno-common -fno-builtin -Wall -DEMBED -Dlinux -D__linux__ -Dunix -D__uClinux__ -Wl,-elf2flt="-r" -o eeprom eeprom.c

calibrate: calibrate.c
	../../../../tools/ucfront-gcc arm-elf-gcc  -Os -fomit-frame-pointer -fno-common -fno-builtin -Wall -DEMBED -Dlinux -D__linux__ -Dunix -D__uClinux__ -Wl,-elf2flt="-r" -o calibrate calibrate.c

clean:
	rm -f *~ *.gdb led key eeprom calibrate
