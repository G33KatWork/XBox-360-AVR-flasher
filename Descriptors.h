#ifndef _DESCRIPTORS_H_
#define _DESCRIPTORS_H_

#include <LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>

typedef struct
{
	USB_Descriptor_Configuration_Header_t Config;
	USB_Descriptor_Interface_t            Interface;
       USB_Descriptor_Endpoint_t             INEndpoint;
       USB_Descriptor_Endpoint_t             OUTEndpoint;
} USB_Descriptor_Configuration_t;

#define FLASHER_EPSIZE              64
#define FLASHER_STREAM_OUT_EPNUM    5
#define FLASHER_STREAM_IN_EPNUM     2

uint16_t CALLBACK_USB_GetDescriptor(const uint16_t wValue,
                                    const uint8_t wIndex,
                                    void** const DescriptorAddress) ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);

#endif
