#ifndef _XSPI_H_
#define _XSPI_H_

#include <inttypes.h>

#define	SPIPORT	PORTE
#define SPIPIN	PINE
#define SPIDDR	DDRE

#define EJ		6
#define	XX		2
#define	SS		5
#define SCK		0
#define MOSI	1
#define MISO	4
#define KSK		7   //not used on my cable

#define PINOUT(DDR, PIN)	(DDR |= (1 << PIN))
#define PININ(DDR, PIN)		(DDR &= ~(1 << PIN))

#define PINHIGH(PORT, PIN)	(PORT |= (1 << PIN))
#define PINLOW(PORT, PIN)	(PORT &= ~(1 << PIN))

#define	PINGET(PORT, PIN)	(PORT & (1 << PIN))


void XSPI_Init(void);

void XSPI_Powerup(void);
void XSPI_Shutdown(void);

void XSPI_EnterFlashmode(void);
void XSPI_LeaveFlashmode(void);

void XSPI_Read(uint8_t reg, uint8_t* buf);
uint16_t XSPI_ReadWord(uint8_t reg);
uint8_t XSPI_ReadByte(uint8_t reg);

void XSPI_Write(uint8_t reg, uint8_t* buf);
void XSPI_WriteByte(uint8_t reg, uint8_t byte);
void XSPI_WriteDword(uint8_t reg, uint32_t dword);
void XSPI_Write0(uint8_t reg);

uint8_t XSPI_FetchByte(void);
void XSPI_PutByte(uint8_t out);

#endif
