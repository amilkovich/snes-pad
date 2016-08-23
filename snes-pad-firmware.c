#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "usbdrv.h"

#define LED0   (1 << PB0)

#define CLOCK1 (1 << PC4)
#define LATCH1 (1 << PC5)
#define DATA1  (1 << PC3)
#define LED1   (1 << PB2)

#define CLOCK2 (1 << PC1)
#define LATCH2 (1 << PC2)
#define DATA2  (1 << PC0)
#define LED2   (1 << PB1)

#define BUTTON_B      0x001
#define BUTTON_Y      0x002
#define BUTTON_SELECT 0x004
#define BUTTON_START  0x008
#define BUTTON_UP     0x010
#define BUTTON_DOWN   0x020
#define BUTTON_LEFT   0x040
#define BUTTON_RIGHT  0x080
#define BUTTON_A      0x100
#define BUTTON_X      0x200
#define BUTTON_L      0x400
#define BUTTON_R      0x800

#define BUTTON_EVENT_DOWN(states, states_previous, button)\
	(((states & button) != (states_previous & button)) &&\
	states & button)
#define BUTTON_EVENT_UP(states, states_previous, button)\
	(((states & button) != (states_previous & button)) &&\
	!(states & button))

PROGMEM const char usbDescriptorConfiguration[] = {
	9,
	USBDESCR_CONFIG,
	9 + (2 * (9 + 9 + 7)), 0,
	2,
	1,
	0,
	(1 << 7),
	USB_CFG_MAX_BUS_POWER / 2,

	// interface 1 descriptor
	9,
	USBDESCR_INTERFACE,
	0,
	0,
	1,
	USB_CFG_INTERFACE_CLASS,
	USB_CFG_INTERFACE_SUBCLASS,
	USB_CFG_INTERFACE_PROTOCOL,
	3,

	9,
	USBDESCR_HID,
	0x01, 0x01,
	0x00,
	0x01,
	0x22,
	USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH, 0,

	// endpoint descriptor for endpoint 1
	7,
	USBDESCR_ENDPOINT,
	(char)0x81,
	0x03,
	8, 0,
	USB_CFG_INTR_POLL_INTERVAL,

	// interface 2 descriptor
	9,
	USBDESCR_INTERFACE,
	1,
	0,
	1,
	USB_CFG_INTERFACE_CLASS,
	USB_CFG_INTERFACE_SUBCLASS,
	USB_CFG_INTERFACE_PROTOCOL,
	4,

	9,
	USBDESCR_HID,
	0x01, 0x01,
	0x00,
	0x01,
	0x22,
	USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH, 0,

	// endpoint descriptor for endpoint 3
	7,
	USBDESCR_ENDPOINT,
	(char)(0x80 | USB_CFG_EP3_NUMBER),
	0x03,
	8, 0,
	USB_CFG_INTR_POLL_INTERVAL,
};

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	0x05, 0x01, // USAGE_PAGE (Generic Desktop)
	0x09, 0x05, // USAGE (Game Pad)
	0xa1, 0x01, // COLLECTION (Application)
	0x05, 0x09, //   USAGE_PAGE (Button)
	0x19, 0x01, //   USAGE_MINIMUM (Button 1)
	0x29, 0x0c, //   USAGE_MAXIMUM (Button 12)
	0x15, 0x00, //   LOGICAL_MINIMUM (0)
	0x25, 0x01, //   LOGICAL_MAXIMUM (1)
	0x75, 0x01, //   REPORT_SIZE (1)
	0x95, 0x0c, //   REPORT_COUNT (12)
	0x81, 0x02, //   INPUT (Data,Var,Abs)
	0xc0        // END_COLLECTION
};

static const char desc0[] = {
	0x04, 0x03, 0x09, 0x04
};

static const char vendor_name[] = {
	0x0e,
	0x03,
	'A', 0x00,
	'n', 0x00,
	'd', 0x00,
	'r', 0x00,
	'e', 0x00,
	'w', 0x00
};

static const char device_name[] = {
	0x12,
	0x03,
	'S', 0x00,
	'N', 0x00,
	'E', 0x00,
	'S', 0x00,
	'-', 0x00,
	'P', 0x00,
	'A', 0x00,
	'D', 0x00,
};

static char interface_name[] = {
	0x16,
	0x03,
	'S', 0x00,
	'N', 0x00,
	'E', 0x00,
	'S', 0x00,
	'-', 0x00,
	'P', 0x00,
	'A', 0x00,
	'D', 0x00,
	' ', 0x00,
	'#', 0x00
};

static uint8_t report_joystick_1[2];
static uint8_t report_joystick_2[2];

usbMsgLen_t usbFunctionDescriptor(struct usbRequest *rq) {
	switch (rq->wValue.bytes[0]) {
		case 0:
			usbMsgPtr = (usbMsgPtr_t)desc0;
			return 4;
		break;

		case 1:
			usbMsgPtr = (usbMsgPtr_t)vendor_name;
			return vendor_name[0];
		break;

		case 2:
			usbMsgPtr = (usbMsgPtr_t)device_name;
			return device_name[0];
		break;

		case 3:
			interface_name[20] = '1';
			usbMsgPtr = (usbMsgPtr_t)interface_name;
			return interface_name[0];
		break;

		case 4:
			interface_name[20] = '2';
			usbMsgPtr = (usbMsgPtr_t)interface_name;
			return interface_name[0];
		break;
	}

	return 0;
}

USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
	usbRequest_t *rq = (void *)data;

	switch (rq->bRequest) {
		case USBRQ_HID_GET_REPORT:
			if (rq->wIndex.word == 3) {
				usbMsgPtr = (usbMsgPtr_t)report_joystick_1;
				return sizeof(report_joystick_1);
			} else if (rq->wIndex.word == 4) {
				usbMsgPtr = (usbMsgPtr_t)report_joystick_2;
				return sizeof(report_joystick_2);
			}
		break;
	}

	return 0;
}

int main() {
	DDRC |= LATCH1 | LATCH2 | CLOCK1 | CLOCK2;
	DDRC &= ~DATA1 & ~DATA2;
	DDRB |= LED0 | LED1 | LED2;

	PORTB &= ~LED0 & ~LED1 & ~LED2;

	// check if a controller is plugged in, if not wait
	uint8_t status_previous_1;
	uint8_t status_previous_2;

	PORTB &= ~LED0;
	do {
		PORTC |= LATCH1;
		PORTC &= ~LATCH1;
		for (uint8_t i = 0; i < 16; i++) {
			PORTC &= ~CLOCK1;
			PORTC |= CLOCK1;
		}
		PORTC &= ~CLOCK1;
		status_previous_1 = ((PINC & DATA1) ? 0 : 1);
		PORTC |= CLOCK1;

		PORTC |= LATCH2;
		PORTC &= ~LATCH2;
		for (uint8_t i = 0; i < 16; i++) {
			PORTC &= ~CLOCK2;
			PORTC |= CLOCK2;
		}
		PORTC &= ~CLOCK2;
		status_previous_2 = ((PINC & DATA2) ? 0 : 1);
		PORTC |= CLOCK2;

		PORTB &= ~LED0;
		_delay_ms(250);
		PORTB |= LED0;
		_delay_ms(250);
	} while (status_previous_1 == 0 && status_previous_2 == 0);

	wdt_enable(WDTO_1S);

	usbInit();
	usbDeviceDisconnect();
	for (uint8_t i = 0; i < 250; i++) {
		wdt_reset();
		_delay_ms(2);
	}
	usbDeviceConnect();

	sei();

	while (1) {
		wdt_reset();
		usbPoll();

		PORTC |= LATCH1;
		PORTC &= ~LATCH1;

		uint16_t buttons_1 = 0;
		static uint16_t buttons_previous_1 = 0;

		for (uint8_t i = 0; i < 16; i++) {
			PORTC &= ~CLOCK1;
			buttons_1 |= ((PINC & DATA1) ? 0 : 1) << i;
			PORTC |= CLOCK1;
		}

		uint8_t status_1 = 0;
		PORTC &= ~CLOCK1;
		status_1 = ((PINC & DATA1) ? 0 : 1);
		PORTC |= CLOCK1;

		if (status_1 != status_previous_1) {
			cli();
			usbDeviceDisconnect();
			wdt_disable();
			asm volatile ("rjmp main;");
		}

		if (buttons_1 != 0) PORTB |= LED1;
		else PORTB &= ~LED1;

		if (usbInterruptIsReady()) {
			if (buttons_1 != buttons_previous_1) {
				report_joystick_1[0] = (buttons_1 & 0x00ff);
				report_joystick_1[1] = (buttons_1 & 0xff00) >> 8;
				usbSetInterrupt(report_joystick_1, sizeof(report_joystick_1));
			}
		}

		buttons_previous_1 = buttons_1;

		PORTC |= LATCH2;
		PORTC &= ~LATCH2;

		uint16_t buttons_2 = 0;
		static uint16_t buttons_previous_2 = 0;

		for (uint8_t i = 0; i < 16; i++) {
			PORTC &= ~CLOCK2;
			buttons_2 |= ((PINC & DATA2) ? 0 : 1) << i;
			PORTC |= CLOCK2;
		}

		uint8_t status_2 = 0;
		PORTC &= ~CLOCK1;
		status_2 = ((PINC & DATA2) ? 0 : 1);
		PORTC |= CLOCK1;

		if (status_2 != status_previous_2) {
			cli();
			usbDeviceDisconnect();
			wdt_disable();
			asm volatile ("rjmp main;");
		}

		if (buttons_2 != 0) PORTB |= LED2;
		else PORTB &= ~LED2;

		if (usbInterruptIsReady3()) {
			if (buttons_2 != buttons_previous_2) {
				report_joystick_2[0] = (buttons_2 & 0x00ff);
				report_joystick_2[1] = (buttons_2 & 0xff00) >> 8;
				usbSetInterrupt3(report_joystick_2, sizeof(report_joystick_2));
			}
		}

		buttons_previous_2 = buttons_2;
	}

	return 0;
}
