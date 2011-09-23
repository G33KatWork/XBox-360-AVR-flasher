#include "Flasher.h"

typedef void (*FillTxBufferFunction)(uint8_t len, uint8_t* buffer);

#define TX_TRANSMITTING	1
#define TX_IDLE			0

#define	RX_RECEIVING	1
#define	RX_IDLE			0

#define TX_BUFFER_SIZE  FLASHER_EPSIZE
#define RX_BUFFER_SIZE  FLASHER_EPSIZE

uint8_t TX_Buffer[TX_BUFFER_SIZE];
uint32_t TX_ToSend = 0;
FillTxBufferFunction TX_Function = NULL;
uint8_t TX_TransmitState = TX_IDLE;

uint8_t RX_Buffer[RX_BUFFER_SIZE];
uint32_t RX_ToReceive = 0;
FillTxBufferFunction RX_Function = NULL;
uint8_t RX_ReceiveState = RX_IDLE;
uint8_t commandProcess;

uint8_t FlashConfig[4];
uint16_t Status = 0;

uint32_t wordsLeft;
uint32_t nextBlock;

//#define DEBUG
#ifdef DEBUG
    #define dbg_P(...)   printf_P(__VA_ARGS__)
#else
	#define dbg_P(...)
#endif

int main(void)
{
	SetupHardware();
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

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
#ifdef DEBUG
	SerialStream_Init(9600, false);
#endif
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
			
			if(!TX_Function)
    			dbg_P(PSTR("ERROR: TX_Function is NULL\r\n"));
			
			TX_Function(bytesToSend, TX_Buffer);	//Fill TX-Buffer
			
            Endpoint_Write_Stream_LE(TX_Buffer, bytesToSend, NO_STREAM_CALLBACK);
			TX_ToSend -= bytesToSend;
			
			Endpoint_ClearIN();
			
			if(TX_ToSend == 0)
				TX_TransmitState = TX_IDLE;
		}
	}
	/*else
	{
	    if(TX_TransmitState == TX_TRANSMITTING)
		{
		    dbg_P(PSTR("Want to transmit, but cant\r\n"));
	    }
	}*/
	
	
	Endpoint_SelectEndpoint(FLASHER_STREAM_OUT_EPNUM);

	if (Endpoint_IsConfigured() && Endpoint_IsOUTReceived() && Endpoint_IsReadWriteAllowed())
	{
		if(RX_ReceiveState == RX_RECEIVING)
		{
			uint8_t bytesToReceive = (RX_ToReceive > sizeof(RX_Buffer)) ? sizeof(RX_Buffer) : RX_ToReceive;
            
            Endpoint_Read_Stream_LE(RX_Buffer, bytesToReceive, NO_STREAM_CALLBACK);
			
			if(!RX_Function)
    			dbg_P(PSTR("ERROR: RX_Function is NULL\r\n"));
			
			RX_Function(bytesToReceive, RX_Buffer);	//Hand received data to handling function
			
			RX_ToReceive -= bytesToReceive;
			Endpoint_ClearOUT();
			
			if(RX_ToReceive == 0)
				RX_ReceiveState = RX_IDLE;
		}
	}
	/*else
	{
	    if(RX_ReceiveState == RX_RECEIVING)
		{
		    dbg_P(PSTR("Want to receive, but cant\r\n"));
	    }
	}*/
}

void EVENT_USB_Device_Connect(void)
{
	/* Indicate USB enumerating */
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
	
	dbg_P(PSTR("Device connected\r\n"));
}

void EVENT_USB_Device_Disconnect(void)
{
	/* Indicate USB not ready */
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	dbg_P(PSTR("Device disconnected\r\n"));
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_READY);
	dbg_P(PSTR("Configuration changed\r\n"));
	
	/* Setup MIDI stream endpoints */
	if (!(Endpoint_ConfigureEndpoint(FLASHER_STREAM_OUT_EPNUM, EP_TYPE_BULK,
		                             ENDPOINT_DIR_OUT, FLASHER_EPSIZE,
	                                 ENDPOINT_BANK_SINGLE)))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		dbg_P(PSTR("Output endpoint configuration failed\r\n"));
	}	
	
	if (!(Endpoint_ConfigureEndpoint(FLASHER_STREAM_IN_EPNUM, EP_TYPE_BULK,
		                             ENDPOINT_DIR_IN, FLASHER_EPSIZE,
	                                 ENDPOINT_BANK_SINGLE)))
	{
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		dbg_P(PSTR("Input endpoint configuration failed\r\n"));
	}
}

void EVENT_USB_Device_UnhandledControlRequest(void)
{
	if(USB_ControlRequest.bmRequestType != (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD))
		return;
	
	Endpoint_ClearSETUP();
	
	uint32_t data[2];
	Endpoint_Read_Control_Stream_LE(&data, sizeof(data));
	Endpoint_ClearOUT();
	
	dbg_P(PSTR("Received vendor request: %x - ArgA: %x, ArgB: %x"), USB_ControlRequest.bRequest, data[0], data[1]);
	
	switch(USB_ControlRequest.bRequest)
	{
		case REQ_DATAREAD: StartFlashRead(data[0] << 5, data[1]); break;
        case REQ_DATAWRITE: StartFlashWrite(data[0] << 5, data[1]); break;
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
	dbg_P(PSTR("Starting to read flash starting with block 0x%x with length 0x%x\r\n"), blockNum, len);
	
    Status = 0;
    wordsLeft = 0;
    nextBlock = blockNum;
	
	TX_Function = TX_ReadData;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = len;
}

void StartFlashWrite(uint32_t blockNum, uint32_t len)
{
    dbg_P(PSTR("Starting to write flash starting with block 0x%x with length 0x%x\r\n"), blockNum, len);
	
	Status = 0;
    wordsLeft = 0;
    nextBlock = blockNum;
	
	RX_Function = RX_WriteFlash;
	RX_ReceiveState = RX_RECEIVING;
	RX_ToReceive = len;
	
    commandProcess = 0;
}

void FlashInit()
{
	dbg_P(PSTR("Initializing Flash\r\n"));
	XSPI_EnterFlashmode();
	XSPI_Read(0, FlashConfig);	//2 times?
	XSPI_Read(0, FlashConfig);
	
	TX_Function = TX_FlashConfig;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = 4;
}

void FlashDeinit()
{
	dbg_P(PSTR("Deinitializing Flash\r\n"));
	XSPI_LeaveFlashmode();
}

void FlashStatus()
{
	dbg_P(PSTR("Retrieving flash status\r\n"));
	
	TX_Function = TX_Status;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = 4;
}

void FlashErase(uint32_t blockNum)
{
	dbg_P(PSTR("Erasing flash block 0x%x\r\n"), blockNum);

	//set status
	Status = XNAND_Erase(blockNum);

    //send 4 0-bytes. FIXME: necessary? PIC does that, too
    TX_Function = TX_ZeroBytes;
	TX_TransmitState = TX_TRANSMITTING;
	TX_ToSend = 4;
}

void PowerUp()
{
	dbg_P(PSTR("Powering Xbox up\r\n"));
	XSPI_LeaveFlashmode();
	XSPI_Powerup();
}

void Shutdown()
{
	dbg_P(PSTR("Shutting Xbox down\r\n"));
	XSPI_LeaveFlashmode();
	XSPI_Shutdown();
}

void Update()
{
	dbg_P(PSTR("Jumping to bootloader. I hope we'll meet again!\r\n"));
	USB_ShutDown();
	RunBootloader();
}


/*** Receive functions ***/
void RX_WriteFlash(uint8_t len, uint8_t* buffer)
{
    len /= 4;
    
    if(commandProcess == 0)
    {
        Status = XNAND_Erase(nextBlock);
        XNAND_StartWrite();
        commandProcess = 1;
    }

    while(len)
    {
        uint8_t writeNow;
        
        if(!wordsLeft)
        {
            nextBlock++;
            wordsLeft = 0x84;
        }
        
        writeNow = (len < wordsLeft) ? len : wordsLeft;
        XNAND_WriteProcess(buffer, writeNow);
        
        buffer += (writeNow*4);
        wordsLeft -= writeNow;
        len -= writeNow;
        
        //execute write if buffer in NAND controller is filled
        if(!wordsLeft)
        {
            Status = XNAND_WriteExecute(nextBlock-1);
            XNAND_StartWrite();
        }
    }
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
            Status = XNAND_StartRead(nextBlock);
            nextBlock++;
            wordsLeft = 0x84;
        }
        
        readNow = (len < wordsLeft) ? len : wordsLeft;
        XNAND_ReadFillBuffer(buffer, readNow);
        
        buffer += (readNow*4);
        wordsLeft -= readNow;
        len -= readNow;
    }
}

void TX_ZeroBytes(uint8_t len, uint8_t* buffer)
{
    memset(buffer, 0, len);
}
