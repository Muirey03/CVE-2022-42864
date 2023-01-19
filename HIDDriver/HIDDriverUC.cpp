//
//  HIDDriverUC.cpp
//  HIDDriver
//
//  Created by Tommy Muir on 19/08/2022.
//  Copyright © 2022 Apple. All rights reserved.
//

#include <DriverKit/OSCollections.h>
#include <HIDDriverKit/HIDDriverKit.h>
#include "HIDDriverUC.h"
#include "HIDDriver.h"
#include "HIDDriverUser.h"
#include "log.h"

struct HIDDriverUC_IVars {
	IOHIDInterface* interface = nullptr;
	OSArray* buffers = nullptr;
};
#define _interface ivars->interface
#define _buffers ivars->buffers

#define GetBuffer(idx) ((IOBufferMemoryDescriptor*)_buffers->getObject(idx))

bool HIDDriverUC::init(void)
{
	bool result = super::init();
	if (result)
		ivars = IONewZero(HIDDriverUC_IVars, 1);
	return result;
}

kern_return_t IMPL(HIDDriverUC, Start)
{
	kern_return_t ret = kIOReturnError;
	HIDDriver* driver;

	ret = Start(provider, SUPERDISPATCH);
	ASSERT_KR(ret);

	driver = OSDynamicCast(HIDDriver, provider);
	ASSERT_TRUE(driver);

	_interface = driver->GetInterface();
	ASSERT_TRUE(_interface);

	_buffers = OSArray::withCapacity(10);
	ASSERT_TRUE(_buffers);

	ret = kIOReturnSuccess;
	return ret;

Exit:
	Stop(provider);
	return ret;
}

kern_return_t HIDDriverUC::ExternalMethod(uint64_t selector, IOUserClientMethodArguments* arguments, const IOUserClientMethodDispatch* dispatch, OSObject* target, void* reference)
{
	switch (selector) {
		case hiddriver_method_type_getvalues:
			return GetValues(arguments);
		case hiddriver_method_type_setvalues:
			return SetValues(arguments);
		default:
			Err("Invalid external method selector");
			goto Exit;
	}

Exit:
	return super::ExternalMethod(selector, arguments, dispatch, target, reference);
}

kern_return_t IMPL(HIDDriverUC, CopyClientMemoryForType)
{
	kern_return_t ret = kIOReturnError;
	IOBufferMemoryDescriptor* mem = nullptr;

	uint32_t request_type = (uint32_t)type & hidbuffer_request_type_mask;
	if (request_type == hidbuffer_request_type_index) {
		//called to unmap memory, remove it from array to free it when done:
		uint32_t idx = (uint32_t)type & ~hidbuffer_request_type_mask;
		mem = GetBuffer(idx);
		ASSERT_TRUE(mem);
		mem->retain();
		OSString* tombstoneObj = OSString::withCString("<null>");
		_buffers->replaceObject(idx, tombstoneObj);
		tombstoneObj->release();
	} else {
		//called to create a new buffer
		uint32_t sz = (uint32_t)type & ~hidbuffer_request_type_mask;
		ret = IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOutIn, sz, 0, &mem);
		ASSERT_KR(ret);

		mem->SetLength(sz);
		_buffers->setObject(mem);
	}
	
	*memory = mem;
	ret = kIOReturnSuccess;

Exit:
	return ret;
}

kern_return_t HIDDriverUC::GetValues(IOUserClientMethodArguments* args)
{
	kern_return_t ret = kIOReturnBadArgument;
	IOBufferMemoryDescriptor* mem = nullptr;

	if (args->scalarInputCount == 2) {
		mem = GetBuffer((uint32_t)args->scalarInput[0]);
		ASSERT_TRUE(mem);
		
		ret = _interface->GetElementValues((uint32_t)args->scalarInput[1], mem);
		ASSERT_KR(ret);
	}

Exit:
	return ret;
}

kern_return_t HIDDriverUC::SetValues(IOUserClientMethodArguments* args)
{
	kern_return_t ret = kIOReturnBadArgument;
	IOBufferMemoryDescriptor* mem = nullptr;

	if (args->scalarInputCount == 1) {
		mem = GetBuffer((uint32_t)args->scalarInput[0]);
		ASSERT_TRUE(mem);

		ret = _interface->SetElementValues(1, mem);
	}

Exit:
	return ret;
}

kern_return_t IMPL(HIDDriverUC, Stop)
{
	OSSafeReleaseNULL(_buffers);
	return Stop(provider, SUPERDISPATCH);
}

void HIDDriverUC::free(void)
{
	IOSafeDeleteNULL(ivars, HIDDriverUC_IVars, 1);
	super::free();
}
