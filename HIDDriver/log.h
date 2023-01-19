//
//  log.h
//  HIDDriver
//
//  Created by Tommy Muir on 19/08/2022.
//  Copyright © 2022 Apple. All rights reserved.
//

#ifndef log_h
#define log_h

#include <os/log.h>
#include <DriverKit/IOLib.h>
#include <DriverKit/IOService.h>
#include <DriverKit/OSString.h>

#define Log(fmt, ...) os_log(OS_LOG_DEFAULT, "(HIDLog) [%{public}s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#define Err(fmt, ...) Log("ERROR: " fmt, ##__VA_ARGS__)
#define ASSERT_KR(kr) if ((kr) != KERN_SUCCESS) { \
	OSString* retStr = NULL; \
	IOService::StringFromReturn(kr, &retStr); \
	Err("ret: %{public}s (0x%x)", retStr ? (retStr->getCStringNoCopy()) : "<UNKNOWN ERROR>", kr); \
	OSSafeReleaseNULL(retStr); goto Exit; }
#define ASSERT_TRUE(x) if (!(x)) { Err("Assertion failed: %{public}s", #x); ret = kIOReturnError; goto Exit; }

#endif /* log_h */
