#include "Descriptors.h"

USB_Descriptor_Device_t PROGMEM DeviceDescriptor =
{
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},
		
	.USBSpecification       = VERSION_BCD(02.00),
	.Class                  = 0xFF,
	.SubClass               = 0x00,
	.Protocol               = 0x00,
				
	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,
		
	.VendorID               = 0xFFFF,
	.ProductID              = 0x0004,
	.ReleaseNumber          = 0x0100,
		
	.ManufacturerStrIndex   = 0x01,
	.ProductStrIndex        = 0x02,
	.SerialNumStrIndex      = 0x03,
		
	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor =
{
	.Config = 
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

			.TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
			.TotalInterfaces        = 1,
				
			.ConfigurationNumber    = 1,
			.ConfigurationStrIndex  = NO_DESCRIPTOR,
				
			.ConfigAttributes       = (USB_CONFIG_ATTR_BUSPOWERED | USB_CONFIG_ATTR_SELFPOWERED),
			
			.MaxPowerConsumption    = USB_CONFIG_POWER_MA(100)
		},
		
	.Interface = 
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

			.InterfaceNumber        = 0x00,
			.AlternateSetting       = 0x00,
			
			.TotalEndpoints         = 2,
				
			.Class                  = 0xFF,
			.SubClass               = 0x00,
			.Protocol               = 0x00,
				
			.InterfaceStrIndex      = NO_DESCRIPTOR
		},

	.INEndpoint = 
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},
										 
			.EndpointAddress        = (ENDPOINT_DESCRIPTOR_DIR_IN | FLASHER_STREAM_IN_EPNUM),
			.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = FLASHER_EPSIZE,
			.PollingIntervalMS      = 0x00
		},
	
	.OUTEndpoint = 
		{
			.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

			.EndpointAddress        = (ENDPOINT_DESCRIPTOR_DIR_OUT | FLASHER_STREAM_OUT_EPNUM),
    		.Attributes             = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
			.EndpointSize           = FLASHER_EPSIZE,
			.PollingIntervalMS      = 0x00
		},
};

USB_Descriptor_String_t PROGMEM LanguageString =
{
	.Header                 = {.Size = USB_STRING_LEN(1), .Type = DTYPE_String},
		
	.UnicodeString          = {LANGUAGE_ID_ENG}
};

USB_Descriptor_String_t PROGMEM ManufacturerString =
{
	.Header                 = {.Size = USB_STRING_LEN(5), .Type = DTYPE_String},
		
	.UnicodeString          = L"PCUSB"
};

USB_Descriptor_String_t PROGMEM ProductString =
{
	.Header                 = {.Size = USB_STRING_LEN(12), .Type = DTYPE_String},
		
	.UnicodeString          = L"MemoryAccess"
};

USB_Descriptor_String_t PROGMEM SerialNumberString =
{
	.Header                 = {.Size = USB_STRING_LEN(8), .Type = DTYPE_String},
		
	.UnicodeString          = L"DEADC0DE"
};

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    void** const DescriptorAddress)
{
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	void*    Address = NULL;
	uint16_t Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = (void*)&DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = (void*)&ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case 0x00:
					Address = (void*)&LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case 0x01:
					Address = (void*)&ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case 0x02:
					Address = (void*)&ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
				case 0x03:
					Address = (void*)&SerialNumberString;
					Size    = pgm_read_byte(&SerialNumberString.Header.Size);
					break;
			}
			break;
	}
	
	*DescriptorAddress = Address;		
	return Size;
}

