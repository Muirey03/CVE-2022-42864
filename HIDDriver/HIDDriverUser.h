//
//  HIDDriverUser.h
//  HIDDriverPoC
//
//  Created by Tommy Muir on 19/08/2022.
//  Copyright © 2022 Apple. All rights reserved.
//

#ifndef HIDDriverUser_h
#define HIDDriverUser_h

typedef enum
{
	hiddriver_method_type_getvalues,
	hiddriver_method_type_setvalues,
	hiddriver_method_count
} hiddriver_method_type;

#define hidbuffer_request_type_mask (1 << 31)
#define hidbuffer_request_type_index hidbuffer_request_type_mask
#define hidbuffer_request_type_size 0
#define hidbuffer_request_payload_max (hidbuffer_request_type_mask - 1)

#endif /* HIDDriverUser_h */
