/**************************************************************************
*
* Copyright (C) 2011 Steve Karg <skarg@users.sourceforge.net>
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
*
*********************************************************************/

/*----------*/
/* Includes */
/*----------*/
#include <stdint.h>
#include <stdbool.h>
/* hardware layer includes */
/* BACnet Stack includes */
#include "datalink.h"
#include "npdu.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dcc.h"
#include "iam.h"

#include "bacnet.h"
#include "bip.h"

#include "EvRec_BACnet4mbed.h"

/*------------------*/
/* Extern Variabels */
/*------------------*/

/*------------------*/
/* Global Variabels */
/*------------------*/
static Ticker covTick;
static Thread bacnetCbThread(osPriorityNormal, 2000, NULL, "BACnetCB_Thread");
EventQueue bacQueue(EVENTS_QUEUE_SIZE);

/*---------------------*/
/* Functiondefinitions */
/*---------------------*/
void bacnet_init(char *ip, Thread *bacnetThread = NULL)
{
	bip_init(ip);

	/* initialize objects */
	Device_Init(NULL);

	/* set up our confirmed service unrecognized service handler - required! */
	apdu_set_unrecognized_service_handler_handler(handler_unrecognized_service);

	/* we need to handle who-is to support dynamic device binding */
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is_unicast);
	apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_HAS, handler_who_has);

	/* Set the handlers for any confirmed services that we support. */
	/* We must implement read property - it's required! */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_REINITIALIZE_DEVICE,
														 handler_reinitialize_device);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
														 handler_read_property);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROP_MULTIPLE,
														 handler_read_property_multiple);
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
														 handler_write_property);

	/* handle communication so we can shutup when asked */
	apdu_set_confirmed_handler(SERVICE_CONFIRMED_DEVICE_COMMUNICATION_CONTROL,
														 handler_device_communication_control);

	apdu_set_confirmed_handler(SERVICE_CONFIRMED_SUBSCRIBE_COV, handler_cov_subscribe);

	handler_cov_init();

	Send_I_Am(&Handler_Transmit_Buffer[0]);

	//
	// BACnet Thread Startup
	{
		// Attach 'bacnet_task' to bacnetThread
		EVRECORD2(BACNET_TASK_STARTING, 0, 0);
		osStatus retVal = osOK;

		if (bacnetThread != NULL)
		{
			if (bacnetThread->get_state() != Thread::Running)
			{
				retVal = bacnetThread->start(callback(bacnet_task));
			}
		}
		else
		{
			bacnetThread = new Thread(BACNET_THREAD_PRIORITY,
																BACNET_THREAD_SIZE,
																NULL,
																"_BACnet_Thread");
			retVal = bacnetThread->start(callback(bacnet_task));
		}

		if (retVal == osOK)
		{
			EVRECORD2(BACNET_TASK_STARTED, retVal, 0);
		}
		else
		{
			EVRECORD2(BACNET_TASK_FAILED, retVal, 0);
		}

		// Attach 'bacQueue' to bacnetThread
		EVRECORD2(BACNET_EVQ_STARTING, 0, 0);

		retVal = bacnetCbThread.start(callback(&bacQueue, &EventQueue::dispatch_forever));

		if (retVal == osOK)
		{
			EVRECORD2(BACNET_EVQ_STARTED, retVal, 0);
		}
		else
		{
			EVRECORD2(BACNET_EVQ_FAILED, retVal, 0);
		}
	}

	// Attach CoV CbTimer
	covTick.attach(callback(&bacnet_timer), BACNET_COV_HANDLER_UPDATE_INTERVAL);
}

void bacnet_timer(void)
{
	handler_cov_timer_seconds((uint32_t)BACNET_COV_HANDLER_UPDATE_INTERVAL);
}

void bacnet_task(void)
{
	uint16_t pdu_len;
	BACNET_ADDRESS src; /* source address */
	uint8_t PDUBuffer[MAX_MPDU];

	while (1)
	{
		/* handle the messaging */
		pdu_len = datalink_receive(&src, &PDUBuffer[0], sizeof(PDUBuffer), 0);

		if (pdu_len)
		{
			EVRECORD2(BACNET_PDU_RECEIVED, pdu_len, 0);
			npdu_handler(&src, &PDUBuffer[0], pdu_len);

// Trigger external Watchdog
#if WDG_TRIGGER_ENABLE
			wdg_extTrigger.trigger(WDG_BACNET_TRIGGER);
#endif

// Internal Watchdog triggern
#if WDG_STM32F7_IWDG_ENABLE
			wdg_iwdgTrigger.trigger(WDG_BACNET_TRIGGER);
#endif
		}

		handler_cov_task();

#if MBED_CONF_RTOS_PRESENT
#if MBED_VERSION >= MBED_ENCODE_VERSION(5, 10, 0)
		ThisThread::yield();
#else
		Thread::yield();
#endif
#endif /* MBED_CONF_RTOS_PRESENT */
	}
}
