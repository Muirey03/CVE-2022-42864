//
//  HIDFakeDevice.cpp
//  HIDDriver
//
//  Created by Tommy Muir on 20/08/2022.
//  Copyright © 2022 Apple. All rights reserved.
//

#include <HIDDriverKit/HIDDriverKit.h>
#include "HIDFakeDevice.h"
#include "log.h"

const uint8_t reportDescriptor[] = {
	0x06, 0x00, 0xff,  				// USAGE_PAGE (Vendor Defined)
	0x09, 0x01,        				// USAGE (1)
	0xa1, 0x01,        				// COLLECTION (Application)
	0x09, 0x21,        				// USAGE (Output Report Data)
	0x15, 0x00,        				// LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  				// LOGICAL_MAXIMUM (255)
	0x75, 0x08,        				// REPORT_SIZE (8)
	0x97, 0xff, 0x0f, 0x00, 0x00,	// REPORT_COUNT
	0x91, 0x02,        				// OUTPUT (Data,Var,Abs)

	0x09, 0x21,        				// USAGE (Output Report Data)
	0x15, 0x00,        				// LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,  				// LOGICAL_MAXIMUM (255)
	0x75, 0x08,        				// REPORT_SIZE (8)
	0x95, 0x00,						// REPORT_COUNT
	0x91, 0x02,        				// OUTPUT (Data,Var,Abs)

	0xc0               				// END_COLLECTION
};

struct HIDFakeDevice_IVars {
	OSData* reportData;
};
#define _reportData ivars->reportData

bool HIDFakeDevice::init()
{
	bool result = super::init();
	if (result)
		ivars = IONewZero(HIDFakeDevice_IVars, 1);
	return result;
}

bool HIDFakeDevice::handleStart(IOService* provider)
{
	return super::handleStart(provider);
}

OSDictionary* HIDFakeDevice::newDeviceDescription(void)
{
	OSDictionary* dictionary = OSDictionary::withCapacity(10);
	OSDictionarySetValue(dictionary, "RegisterService", kOSBooleanTrue);
	OSDictionarySetValue(dictionary, "HIDDefaultBehavior", kOSBooleanTrue);

	if (OSString* manufacturer = OSString::withCString("Muirey03")) {
		OSDictionarySetValue(dictionary, kIOHIDManufacturerKey, manufacturer);
		manufacturer->release();
	}

	if (auto product = OSString::withCString("HIDFakeDevice")) {
		OSDictionarySetValue(dictionary, kIOHIDProductKey, product);
		product->release();
	}

	return dictionary;
}

OSData* HIDFakeDevice::newReportDescriptor(void)
{
	return OSData::withBytes(reportDescriptor, sizeof(reportDescriptor));
}

kern_return_t HIDFakeDevice::getReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options, uint32_t completionTimeout, OSAction *action)
{
	kern_return_t ret = kIOReturnSuccess;
	IOMemoryMap* map = nullptr;
	uint64_t reportLength;
	uint32_t savedLength = _reportData ? (uint32_t)_reportData->getLength() : 0;

	ret = report->CreateMapping(0, 0, 0, 0, 0, &map);
	ASSERT_KR(ret);

	reportLength = map->GetLength();
	ASSERT_TRUE(reportLength >= savedLength);

	if (_reportData)
		OSDataGetBytes(_reportData, (void*)map->GetAddress(), 0, savedLength);
	
	CompleteReport(action, kIOReturnSuccess, savedLength);

Exit:
	OSSafeReleaseNULL(map);
	return ret;
}

kern_return_t HIDFakeDevice::setReport(IOMemoryDescriptor *report, IOHIDReportType reportType, IOOptionBits options, uint32_t completionTimeout, OSAction *action)
{
	kern_return_t ret = kIOReturnSuccess;
	IOMemoryMap* map;
	uint64_t reportLength;

	ret = report->CreateMapping(kIOMemoryMapReadOnly, 0, 0, 0, 0, &map);
	ASSERT_KR(ret);

	OSSafeReleaseNULL(_reportData);
	reportLength = map->GetLength();
	_reportData = OSData::withBytes((void*)map->GetAddress(), reportLength);

	map->release();
	
	CompleteReport(action, kIOReturnSuccess, reportLength);

Exit:
	return ret;
}

kern_return_t IMPL(HIDFakeDevice, Stop)
{
	OSSafeReleaseNULL(_reportData);
	return Stop(provider, SUPERDISPATCH);
}

void HIDFakeDevice::free()
{
	IOSafeDeleteNULL(ivars, HIDFakeDevice_IVars, 1);
	super::free();
}
