/**************************************************************************
*
* Copyright (C) 2010 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/
#ifndef BACNET_H
#define BACNET_H

#include <stdint.h>
#include "math.h"

/* mBed */
#include "mbed.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "handlers.h"
/* BACnet objects */
#include "device_obj.h"
#include "bo.h"
#include "bi.h"
#include "bv.h"
#include "ao.h"	
#include "ai.h"	
#include "av.h"
#include "msv.h"
	
#define BACNET_INSTANCE_DELIMITER BACNET_MAX_INSTANCE+1

void bacnet_init(char *ip, Thread *bacnetThread);
void bacnet_task(void);
void bacnet_timer(void);
		
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
