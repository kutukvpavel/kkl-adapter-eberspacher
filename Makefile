BUILDDIR=$(CURDIR)/build

MCU=attiny13a
F_CPU=1200000
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-std=c99 -Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
TARGET=$(BUILDDIR)/main
SRCS=main.c


all:
		${CC} ${CFLAGS} -o ${TARGET}.elf ${SRCS}
		${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.elf ${TARGET}.hex

flash:
		avrdude -p ${MCU} -c usbasp -U flash:w:${TARGET}.hex:i -F -P usb

clean:
		rm -f *.elf *.hex
