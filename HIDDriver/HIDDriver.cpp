//
//  HIDDriver.cpp
//  HIDDriver
//
//  Created by Tommy Muir on 19/08/2022.
//  Copyright © 2022 Apple. All rights reserved.
//

#include <DriverKit/IOUserClient.h>
#include <DriverKit/OSCollections.h>
#include <HIDDriverKit/HIDDriverKit.h>
#include "HIDDriver.h"
#include "HIDFakeDevice.h"
#include "log.h"

struct HIDDriver_IVars {
	HIDFakeDevice* device = nullptr;
	IOHIDInterface* interface = nullptr;
};
#define _device ivars->device
#define _interface ivars->interface

bool HIDDriver::init(void)
{
	bool result = super::init();
	if (result)
		ivars = IONewZero(HIDDriver_IVars, 1);
	return result;
}

kern_return_t IMPL(HIDDriver, Start)
{
	kern_return_t ret;
	const char* fakeDeviceProduct = "HIDFakeDevice";
	IOService* device = nullptr;
	IOHIDInterface* interface = nullptr;
	OSDictionary* interfaceProps = nullptr;
	OSString* product = nullptr;

	ret = Start(provider, SUPERDISPATCH);
	ASSERT_KR(ret);

	interface = OSDynamicCast(IOHIDInterface, provider);
	ASSERT_TRUE(interface);

	ret = interface->CopyProperties(&interfaceProps);
	ASSERT_KR(ret);

	product = OSDynamicCast(OSString, OSDictionaryGetValue(interfaceProps, kIOHIDProductKey));
	if (product) {
		if (product->isEqualTo(fakeDeviceProduct)) {
			//interface is for our device, save it and expose ourself to userspace
			_interface = interface;

			ret = RegisterService();
			ASSERT_KR(ret);
		} else {
			//interface is not for our device, so let's create it
			ret = Create(this, "FakeDeviceProperties", &device);
			ASSERT_KR(ret);

			_device = OSDynamicCast(HIDFakeDevice, device);
			ASSERT_TRUE(_device);
		}
	}
	interfaceProps->release();

	return kIOReturnSuccess;

Exit:
	Stop(provider);
	return ret;
}

kern_return_t IMPL(HIDDriver, NewUserClient)
{
	kern_return_t ret = kIOReturnSuccess;
	IOService* client = nullptr;

	ret = Create(this, "UserClientProperties", &client);
	ASSERT_KR(ret);

	*userClient = OSDynamicCast(IOUserClient, client);
	ASSERT_TRUE(*userClient);

Exit:
	return ret;
}

kern_return_t IMPL(HIDDriver, Stop)
{
	OSSafeReleaseNULL(_device);
	_interface = nullptr;

	return Stop(provider, SUPERDISPATCH);
}

IOHIDInterface* HIDDriver::GetInterface() {
	return _interface;
}

void HIDDriver::free(void)
{
	IOSafeDeleteNULL(ivars, HIDDriver_IVars, 1);
	super::free();
}
