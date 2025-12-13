.PHONY: lpc2468mmc adc pwm sfr

CFLAGS=
LDFLAGS=

all: lpc2468mmc adc pwm sfr

lpc2468mmc:
	make -C lpc2468mmc

adc:
	make -C adc

pwm:
	make -C pwm

sfr:
	make -C sfr

clean:
	make -C lpc2468mmc clean
	make -C adc clean
	make -C pwm clean
	make -C sfr clean

