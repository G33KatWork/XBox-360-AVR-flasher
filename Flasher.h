#ifndef _FLASHER_H_
#define _FLASHER_H_

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <LUFA/Version.h>
#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/Peripheral/SerialStream.h>
#include <LUFA/Drivers/Board/Buttons.h>

#include "Descriptors.h"
#include "XSPI.h"
#include "XNAND.h"

/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
#define LEDMASK_USB_NOTREADY      LEDS_LED1

/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)


/* Vendor specific requests */
#define REQ_DATAREAD			0x01
#define REQ_DATAWRITE			0x02
#define REQ_DATAINIT			0x03
#define REQ_DATADEINIT			0x04
#define REQ_DATASTATUS			0x05
#define REQ_DATAERASE			0x06
#define REQ_POWERUP				0x10
#define REQ_SHUTDOWN			0x11
#define REQ_UPDATE				0xF0


typedef void (*BootloaderPtr)(void) __attribute__ ((noreturn));
BootloaderPtr RunBootloader = (BootloaderPtr)BOOT_START_ADDR;

void SetupHardware(void);
void Flasher_Task(void);

void StartFlashRead(uint32_t blockNum, uint32_t len);
void StartFlashWrite(uint32_t blockNum, uint32_t len);
void FlashInit(void);
void FlashDeinit(void);
void FlashStatus(void);
void FlashErase(uint32_t blockNum);
void PowerUp(void);
void Shutdown(void);
void Update(void);

/*** Transmit functions ***/
void TX_Status(uint8_t len, uint8_t* buffer);
void TX_FlashConfig(uint8_t len, uint8_t* buffer);
void TX_ReadData(uint8_t len, uint8_t* buffer);
void TX_ZeroBytes(uint8_t len, uint8_t* buffer);

void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_UnhandledControlRequest(void);

#endif
