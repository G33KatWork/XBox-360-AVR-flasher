#include "Flasher.h"

typedef void (*FillTxBufferFunction)(uint8_t len, uint8_t* buffer);

#define TX_TRANSMITTING	1
#define TX_IDLE			0

#define	RX_RECEIVING	1
#define	RX_IDLE			0

#define TX_BUFFER_SIZE  FLASHER_EPSIZE

uint8_t TX_Buffer[TX_BUFFER_SIZE];
uint32_t TX_ToSend = 0;
FillTxBufferFunction TX_Function = NULL;
uint8_t TX_TransmitState = TX_IDLE;

uint8_t RX_ReceiveState = RX_IDLE;
uint32_t RX_ToReceive = 0;

uint8_t FlashConfig[4];
uint16_t Status = 0;

uint32_t wordsLeft;
uint32_t nextBlock;

StreamCallbackPtr_t RX_Function;

//#define RS232_DEBUG
#ifdef RS232_DEBUG
	#define MySerial_TxString_P(x) Serial_TxString_P(x)
	#define MySerial_TxInt(x) Serial_TxInt(x)
#else
	#define MySerial_TxString_P(x)
	#define MySerial_TxInt(x)
#endif

void Serial_TxInt(uint32_t i)
{
	char bla[10] = {0};
	itoa(i, bla, 16);
	Serial_TxString(bla);
}

int main(void)
{
	SetupHardware();
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();
	
    printf_P(PSTR("Hello\r\n"));
	MySerial_TxString_P(PSTR("Hello\r\n"));

	for (;;)
	{
		Flasher_Task();
		USB_USBTask();
	}
}

void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
	
	/* Hardware Initialization */
	LEDs_Init();
	SerialStream_Init(9600, false);
	USB_Init();
	XSPI_Init();
}

void Flasher_Task()
{
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;
	
	Endpoint_SelectEndpoint(FLASHER_STREAM_IN_EPNUM);

	if (Endpoint_IsConfigured() && Endpoint_IsINReady() && Endpoint_IsReadWriteAllowed())
	{
		if(TX_TransmitState == TX_TRANSMITTING)
		{
			uint8_t bytesToSend = (TX_ToSend > sizeof(TX_Buffer)) ? sizeof(TX_Buffer) : TX_ToSend;
			
			/*MySerial_TxString_P(PSTR("Writing 0x"));
			MySerial_TxInt(bytesToSend);
			MySerial_TxString_P(PSTR(" Bytes to Endpoint\r\n"));*/
			
			if(!TX_Function)
    			MySerial_TxString_P(PSTR("ERROR: TX_Function is NULL\r\n"));
			TX_Function(bytesToSend, TX_Buffer);	//Fill TX-Buffer
			
            //memset(TX_Buffer, 0xff, bytesToSend);
			uint8_t err = Endpoint_Write_Stream_LE(TX_Buffer, bytesToSend, NO_STREAM_CALLBACK);
			if(err)
			{
    			MySerial_TxString_P(PSTR("Error at Endpoint_Write_Stream_LE: "));
    			MySerial_TxInt(err);
    			MySerial_TxString_P(PSTR("\r\n"));
			    
			}
			TX_ToSend -= bytesToSend;
			/*MySerial_TxString_P(PSTR("TX Buffer written to stream. Bytes left: "));
			MySerial_TxInt(TX_ToSend);
			MySerial_TxString_P(PSTR(" Bytes to Endpoint\r\n"));*/
			
			Endpoint_ClearIN();
			//MySerial_TxString_P(PSTR("Endpoint cleared\r\n"));
			
			if(TX_ToSend == 0)
			{
				MySerial_TxString_P(PSTR("Transmission seems finished\r\n"));
				TX_TransmitState = TX_IDLE;
			}
			
		}
	}
	else
	{
	    if(TX_TransmitState == TX_TRANSMITTING)
		{
		    MySerial_TxString_P(PSTR("Want to transmit, but cant\r\n"));
	    }
	}
	
	/*Endpoint_SelectEndpoint(FLASHER_STREAM_OUT_EPNUM);
	
	if (Endpoint_IsOUTReceived())
	{
		Endpoint_ClearOUT();
	}*/
}

void EVENT_USB_Device_Connect(void)
{
	/* Indicate USB enumerating */
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
	
	MySerial_TxString_P(PSTR("Device connected\r\n"));
}

void EVENT_USB_Device_Disconnect(void)
{
	/* Indicate USB not ready */
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	MySerial_TxString_P(PSTR("Device disconnected\r\n"));
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_READY);
	MySerial_TxString_P(PSTR("Configuration changed\r\n"));
	
	/* Setup MIDI stream endpoints */
	if (!(Endpoint_ConfigureEndpoint(FLASHER_STREAM_OUT_EPNUM, EP_TYPE_BULK,
		                             ENDPOINT_DIR_OUT, FLASHER_EPSIZE,
	                                 ENDPOINT_BANK_SINGLE)))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		MySerial_TxString_P(PSTR("Output endpoint configuration failed\r\n"));
	}	
	
	if (!(Endpoint_ConfigureEndpoint(FLASHER_STREAM_IN_EPNUM, EP_TYPE_BULK,
		                             ENDPOINT_DIR_IN, FLASHER_EPSIZE,
	                                 ENDPOINT_BANK_SINGLE)))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		MySerial_TxString_P(PSTR("Input endpoint configuration failed\r\n"));
	}
}

void EVENT_USB_Device_UnhandledControlRequest(void)
{
	if(USB_ControlRequest.bmRequestType != (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD))
		return;
	
	Endpoint_ClearSETUP();
	
	MySerial_TxString_P(PSTR("Received vendor request: "));
	
	MySerial_TxInt(USB_ControlRequest.bRequest);
	MySerial_TxString_P(PSTR("\r\n"));
	
	uint32_t data[2];
	Endpoint_Read_Control_Stream_LE(&data, sizeof(data));
	Endpoint_ClearOUT();
	
	MySerial_TxString_P(PSTR("Argument B: "));
	MySerial_TxInt(data[0]);
	MySerial_TxString_P(PSTR("\r\n"));
	
	MySerial_TxString_P(PSTR("Argument A: "));
	MySerial_TxInt(data[1]);
	MySerial_TxString_P(PSTR("\r\n"));
	
	switch(USB_ControlRequest.bRequest)
	{
		case REQ_DATAREAD: StartFlashRead(data[0] << 5, data[1]); break;
		case REQ_DATAWRITE: break;
		case REQ_DATAINIT: FlashInit(); break;
		case REQ_DATADEINIT: FlashDeinit(); break;
		case REQ_DATASTATUS: FlashStatus(); break;
		case REQ_DATAERASE: FlashErase(data[0] << 5); break;
		case REQ_POWERUP: PowerUp(); break;
		case REQ_SHUTDOWN: Shutdown(); break;
		case REQ_UPDATE: Update(); break;
	}
	
	Endpoint_ClearStatusStage();	//needed?
}

void StartFlashRead(uint32_t blockNum, uint32_t len)
{
	MySerial_TxString_P(PSTR("Starting to read flash starting with block 0x"));
	MySerial_TxInt(blockNum);
	MySerial_TxString_P(PSTR(" with length 0x"));
	MySerial_TxInt(len);
	MySerial_TxString_P(PSTR("\r\n"));
	
    Status = 0;
    wordsLeft = 0;
    nextBlock = blockNum;
	
	TX_Function = TX_ReadData;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = len;
}

void StartFlashWrite(uint32_t blockNum, uint32_t len)
{
	MySerial_TxString_P(PSTR("Starting to write flash starting with block 0x"));
	MySerial_TxInt(blockNum);
	MySerial_TxString_P(PSTR(" with length 0x"));
	MySerial_TxInt(len);
	MySerial_TxString_P(PSTR("\r\n"));
	
	//TODO: Issue write start command to NAND and set real status!
	Status = 0x0000;
	
	RX_ReceiveState = RX_RECEIVING;
	RX_ToReceive = len;
}

void FlashInit()
{
	MySerial_TxString_P(PSTR("Initializing Flash\r\n"));
	XSPI_EnterFlashmode();
	XSPI_Read(0, FlashConfig);	//2 times?
	XSPI_Read(0, FlashConfig);
	
	TX_Function = TX_FlashConfig;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = 4;
}

void FlashDeinit()
{
	MySerial_TxString_P(PSTR("Deinitializing Flash\r\n"));
	XSPI_LeaveFlashmode();
}

void FlashStatus()
{
	MySerial_TxString_P(PSTR("Retrieving flash status\r\n"));
	
	TX_Function = TX_Status;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = 4;
}

void FlashErase(uint32_t blockNum)
{
	MySerial_TxString_P(PSTR("Erasing flash block 0x"));
	MySerial_TxInt(blockNum);
	MySerial_TxString_P(PSTR("\r\n"));

	//set status
	Status = XNAND_Erase(blockNum << 5);

    //send 4 0-bytes. FIXME: necessary? PIC does that, too
    TX_Function = TX_ZeroBytes;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = 4;
}

void PowerUp()
{
	MySerial_TxString_P(PSTR("Powering Xbox up\r\n"));
	XSPI_LeaveFlashmode();
	XSPI_Powerup();
}

void Shutdown()
{
	MySerial_TxString_P(PSTR("Shutting Xbox down\r\n"));
	XSPI_LeaveFlashmode();
	XSPI_Shutdown();
}

void Update()
{
	MySerial_TxString_P(PSTR("Jumping to bootloader. I hope we'll meet again!\r\n"));
	USB_ShutDown();
	RunBootloader();
}


/*** Transmit functions ***/
void TX_Status(uint8_t len, uint8_t* buffer)
{
	memset(buffer, 0, len);
	buffer[0] = ((uint8_t*)&Status)[0];
	buffer[1] = ((uint8_t*)&Status)[1];
}

void TX_FlashConfig(uint8_t len, uint8_t* buffer)
{
    memcpy(buffer, FlashConfig, len);
}

void TX_ReadData(uint8_t len, uint8_t* buffer)
{
    len /= 4;
    
    while(len)
    {
        uint8_t readNow;
        
        if(!wordsLeft)
        {
            //printf_P(PSTR("Starting read for block %x\r\n"), nextBlock);
            
            Status = XNAND_StartRead(nextBlock);
            nextBlock++;
            wordsLeft = 0x84;
        }
        
        readNow = (len < wordsLeft) ? len : wordsLeft;
        
        XNAND_ReadFillBuffer(buffer, readNow);
        
        /*printf_P(PSTR("Read %x bytes\r\n"), readNow*4);
        for(int i = 0; i < readNow*4; i++)
            printf_P(PSTR("%x "), buffer[i]);
        
        printf_P(PSTR("\r\n"));*/
        
        buffer += (readNow*4);
        wordsLeft -= readNow;
        len -= readNow;
    }
}

void TX_ZeroBytes(uint8_t len, uint8_t* buffer)
{
    memset(buffer, 0, len);
}