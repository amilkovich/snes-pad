CFLAGS = -Wall -Os -Iusbdrv -mmcu=atmega328p -DF_CPU=16000000
OBJECTS = usbdrv/usbdrv.o usbdrv/usbdrvasm.o snes-pad-firmware.o

all: snes-pad-firmware.hex

snes-pad-firmware.hex: snes-pad-firmware.elf
	avr-objcopy -j .text -j .data -O ihex $< $@

snes-pad-firmware.elf: $(OBJECTS)
	avr-gcc $(CFLAGS) $(OBJECTS) -o $@

%.o: %.c
	avr-gcc $(CFLAGS) -c $< -o $@

%.o: %.S
	avr-gcc $(CFLAGS) -x assembler-with-cpp -c $< -o $@

$(OBJECTS): usbdrv/usbconfig.h

flash: snes-pad-firmware.hex
	avrdude -p atmega328p -c usbasp -U flash:w:snes-pad-firmware.hex

clean:
	$(RM) *.o *.hex *.elf usbdrv/*.o

.PHONY: all flash clean
