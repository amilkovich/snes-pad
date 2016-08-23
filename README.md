## snes-pad

USB adapter firmware for SNES type controllers.

![snes-pad_top.png](pictures/snes-pad_top.png?raw=true)
![snes-pad_bottom.png](pictures/snes-pad_bottom.png?raw=true)

## FAQ

### What is snes-pad?

snes-pad is a USB adapter firmware implementation for SNES type controllers. It
is based on Atmel's ATmega328P microcontroller and Objective Development's V-USB
library.

### What license is snes-pad release under?

snes-pad is released under the MIT/X Consortium License, see LICENSE file
for details.

Currently, the library dependencies are:

V-USB (GPLv2, source included; see usbdrv/License.txt for details)
	http://www.obdev.at/vusb/

### Where can I find the schematic for this project?

I actually do not have a schematic drawn up for this yet. The project was hacked
together one night during a weekend off. If the decision is made to manufacture
a PCB then I will add the schematics here. See the source code for the general
idea of the pin wiring.

### What bootloader is used for snes-pad?

I ended up building USBaspLoader from source for this project. However, it
should be straight forward to use some ISP to flash this firmware if there is
interest.
