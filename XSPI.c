#include "XSPI.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void XSPI_Init(void)
{
	PINOUT(SPIDDR, KSK);
	PINOUT(SPIDDR, EJ);
	PINOUT(SPIDDR, XX);
	PINOUT(SPIDDR, SS);
	PINOUT(SPIDDR, SCK);
	PINOUT(SPIDDR, MOSI);
	
	PINHIGH(SPIPORT, EJ);
	PINHIGH(SPIPORT, SS);
	PINHIGH(SPIPORT, XX);
	PINHIGH(SPIPORT, SCK);
	PINHIGH(SPIPORT, KSK);
	PINLOW(SPIPORT, MOSI);
}

void XSPI_Powerup(void)
{
	PINLOW(SPIPORT, KSK);
	_delay_ms(5);
	PINHIGH(SPIPORT, KSK);
	_delay_ms(5);
	
	PINLOW(SPIPORT, KSK);
	_delay_ms(5);
	PINHIGH(SPIPORT, KSK);
	_delay_ms(5);
	
	PINLOW(SPIPORT, KSK);
	_delay_ms(5);
	PINHIGH(SPIPORT, KSK);
	_delay_ms(5);
	
	PINLOW(SPIPORT, KSK);
	_delay_ms(5);
	PINHIGH(SPIPORT, KSK);
}

void XSPI_Shutdown(void)
{
	PINHIGH(SPIPORT, SS);
	PINLOW(SPIPORT, XX);
	PINLOW(SPIPORT, EJ);
	
	_delay_ms(50);
	
	PINHIGH(SPIPORT, EJ);
}

void XSPI_EnterFlashmode(void)
{
	PINLOW(SPIPORT, XX);
	
	_delay_ms(50);
	
	PINLOW(SPIPORT, SS);
	PINLOW(SPIPORT, EJ);
	
	_delay_ms(50);
	
	PINHIGH(SPIPORT, XX);
	PINHIGH(SPIPORT, EJ);
	
	_delay_ms(50);
}

void XSPI_LeaveFlashmode(void)
{
	PINHIGH(SPIPORT, SS);
	PINLOW(SPIPORT, EJ);
	
	_delay_ms(50);
	
	PINLOW(SPIPORT, XX);
	PINHIGH(SPIPORT, EJ);
}

void XSPI_Read(uint8_t reg, uint8_t* buf)
{
	PINLOW(SPIPORT, SS);
	
	XSPI_PutByte((reg << 2) | 1);
	XSPI_PutByte(0xFF);
	
	*buf++ = XSPI_FetchByte();
	*buf++ = XSPI_FetchByte();
	*buf++ = XSPI_FetchByte();
	*buf++ = XSPI_FetchByte();
	
	PINHIGH(SPIPORT, SS);
}

uint16_t XSPI_ReadWord(uint8_t reg)
{
    uint16_t res;
    
    PINLOW(SPIPORT, SS);
    
    XSPI_PutByte((reg << 2) | 1);
	XSPI_PutByte(0xFF);
	
	res = XSPI_FetchByte();
    res |= ((uint16_t)XSPI_FetchByte()) << 8;
    
    PINHIGH(SPIPORT, SS);
    
    return res;
}

uint8_t XSPI_ReadByte(uint8_t reg)
{
    uint8_t res;
    
    PINLOW(SPIPORT, SS);
    
    XSPI_PutByte((reg << 2) | 1);
	XSPI_PutByte(0xFF);
	
	res = XSPI_FetchByte();
    
    PINHIGH(SPIPORT, SS);
    
    return res;
}

void XSPI_Write(uint8_t reg, uint8_t* buf)
{
    PINLOW(SPIPORT, SS);
	
	XSPI_PutByte((reg << 2) | 2);
	
	XSPI_PutByte(*buf++);
	XSPI_PutByte(*buf++);
	XSPI_PutByte(*buf++);
	XSPI_PutByte(*buf++);
	
	PINHIGH(SPIPORT, SS);
}

void XSPI_WriteByte(uint8_t reg, uint8_t byte)
{
    uint8_t data[] = {0,0,0,0};
    data[0] = byte;
    
    XSPI_Write(reg, data);
}

void XSPI_WriteDword(uint8_t reg, uint32_t dword)
{
    XSPI_Write(reg, (uint8_t*)&dword);
}

void XSPI_Write0(uint8_t reg)
{
    uint8_t tmp[] = {0,0,0,0};
    XSPI_Write(reg, tmp);
}

uint8_t XSPI_FetchByte()
{
	uint8_t in = 0;
	PINLOW(SPIPORT, MOSI);
	
	for(uint8_t i = 0; i < 8; i++)
	{
		PINHIGH(SPIPORT, SCK);
		in |= PINGET(SPIPIN, MISO) ? (1 << i) : 0x00;
		PINLOW(SPIPORT, SCK);
	}
	
	return in;
}

void XSPI_PutByte(uint8_t out)
{
	for(uint8_t i = 0; i < 8; i++)
	{
		if(out & (1 << i)) PINHIGH(SPIPORT, MOSI); else PINLOW(SPIPORT, MOSI);
		PINHIGH(SPIPORT, SCK);
		PINLOW(SPIPORT, SCK);
	}
}
