/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
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

/* Analog Output Objects - customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config_bacnet.h"     /* the custom stuff */
#include "wp.h"
#include "ao.h"
#include "handlers.h"
#include "mbed.h"

#include "EvRec_BACnet4mbed.h"

extern EventQueue bacQueue;


/* we choose to have a NULL level in our system represented by */
/* a particular value.  When the priorities are not in use, they */
/* will be relinquished (i.e. set to the NULL level). */
#define AO_LEVEL_NULL NULL
/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define AO_RELINQUISH_DEFAULT 0

extern ANALOG_OUTPUT_DESCR AO_Descr[];

uint32_t NUM_ANALOG_OUTPUTS;

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_UNITS,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    -1
};

static const int Properties_Optional[] = {
    PROP_DESCRIPTION,
    -1
};

static const int Properties_Proprietary[] = {
    -1
};

void Analog_Output_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;

    return;
}

void Analog_Output_Init(
    void)
{
    unsigned i, j;

		/* initialize all the analog output priority arrays to NULL */
		for (i = 0; i < Analog_Output_Count(); i++) {
				for (j = 0; j < BACNET_MAX_PRIORITY; j++) {
						AO_Descr[i].Present_Value[j] = AO_LEVEL_NULL;
				}
		}

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Analog_Output_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < NUM_ANALOG_OUTPUTS)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Analog_Output_Count(
    void)
{
	uint32_t i=0;
	
	while(AO_Descr[i].Object_Instance <= BACNET_MAX_INSTANCE)
		i++;
	
	NUM_ANALOG_OUTPUTS = i;
  return i;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Analog_Output_Index_To_Instance(
    unsigned index)
{
    return AO_Descr[index].Object_Instance;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Analog_Output_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = 0;

    for(index=0; index<NUM_ANALOG_OUTPUTS; index++)
		{
			if(AO_Descr[index].Object_Instance == object_instance)
				return index;
		}

    return index;
}

float Analog_Output_Present_Value(
    uint32_t object_instance)
{
    float value = AO_RELINQUISH_DEFAULT;
    unsigned index = 0;
    unsigned i = 0;

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < NUM_ANALOG_OUTPUTS) {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (AO_Descr[index].Present_Value[i] != AO_LEVEL_NULL) {
                value = AO_Descr[index].Present_Value[i];
                break;
            }
        }
    }

    return value;
}

unsigned Analog_Output_Present_Value_Priority(
    uint32_t object_instance)
{
    unsigned index = 0; /* instance to index conversion */
    unsigned i = 0;     /* loop counter */
    unsigned priority = 0;      /* return value */

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < NUM_ANALOG_OUTPUTS) {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (AO_Descr[index].Present_Value[i] != AO_LEVEL_NULL) {
                priority = i + 1;
                break;
            }
        }
    }

    return priority;
}

bool Analog_Output_Present_Value_Set(
    uint32_t object_instance,
    float value,
    unsigned priority)
{
    unsigned index = 0;
    bool status = false;

    index = Analog_Output_Instance_To_Index(object_instance);
    if (index < NUM_ANALOG_OUTPUTS) {
        if (priority && (priority <= BACNET_MAX_PRIORITY) &&
            (priority != 6 /* reserved */ )) {
            AO_Descr[index].Present_Value[priority - 1] = value;
            status = true;
        }
    }

    return status;
}


/* note: the object name must be unique within this device */
bool Analog_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    unsigned index = 0;
    bool status = false;

    index = Analog_Output_Instance_To_Index(object_instance);

    if (index < NUM_ANALOG_OUTPUTS) {
        status = characterstring_init_ansi(object_name, AO_Descr[index].Object_Name);
    }

    return status;
}

bool Analog_Output_Object_Description(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING *object_description)
{
    unsigned index = 0;
    bool status = false;

    index = Analog_Output_Instance_To_Index(object_instance);

    if (index < NUM_ANALOG_OUTPUTS)
    {
      status = 
        characterstring_init_ansi(object_description, AO_Descr[index].Object_Description);
    }

    return status;
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int Analog_Output_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int len = 0;
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    float real_value = (float) 1.414;
    unsigned object_index = 0;
    unsigned i = 0;
    bool state = false;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
		
    object_index = Analog_Output_Instance_To_Index(rpdata->object_instance);
		
		if(AO_Descr[object_index].read_callback)
		{
		  uint8_t id = bacQueue.call(AO_Descr[object_index].read_callback, (uint32_t)rpdata->object_property);
				
		  if(id > 0)
		  { EVRECORD2(BACNET_EVQ_AO_RDCB_CALLED, id, 0); }
		  else
		  { EVRECORD2(BACNET_EVQ_AO_RDCB_FAILED, id, 0); }
		}
				
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_ANALOG_OUTPUT,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
            Analog_Output_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_DESCRIPTION:
            Analog_Output_Object_Description(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_ANALOG_OUTPUT);
            break;
        case PROP_PRESENT_VALUE:
            real_value = Analog_Output_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        case PROP_STATUS_FLAGS:
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE:
            object_index =
                Analog_Output_Instance_To_Index(rpdata->object_instance);
            state = AO_Descr[object_index].Out_Of_Service;
            apdu_len = encode_application_boolean(&apdu[0], state);
            break;
        case PROP_UNITS:
            apdu_len = encode_application_enumerated(&apdu[0], UNITS_PERCENT);
            break;
        case PROP_PRIORITY_ARRAY:
            /* Array element zero is the number of elements in the array */
            if (rpdata->array_index == 0)
                apdu_len =
                    encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                object_index =
                    Analog_Output_Instance_To_Index(rpdata->object_instance);
                for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                    /* FIXME: check if we have room before adding it to APDU */
                    if (AO_Descr[object_index].Present_Value[i] == AO_LEVEL_NULL)
                        len = encode_application_null(&apdu[apdu_len]);
                    else {
                        real_value = AO_Descr[object_index].Present_Value[i];
                        len =
                            encode_application_real(&apdu[apdu_len],
                            real_value);
                    }
                    /* add it if we have room */
                    if ((apdu_len + len) < MAX_APDU)
                        apdu_len += len;
                    else {
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else {
                object_index =
                    Analog_Output_Instance_To_Index(rpdata->object_instance);
                if (rpdata->array_index <= BACNET_MAX_PRIORITY) {
                    if (AO_Descr[object_index].Present_Value[rpdata->array_index - 1] == AO_LEVEL_NULL)
                        apdu_len = encode_application_null(&apdu[0]);
                    else {
                        real_value = AO_Descr[object_index].Present_Value[rpdata->array_index - 1];
                        apdu_len =
                            encode_application_real(&apdu[0], real_value);
                    }
                } else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_RELINQUISH_DEFAULT:
            real_value = AO_RELINQUISH_DEFAULT;
            apdu_len = encode_application_real(&apdu[0], real_value);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Analog_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value */
    unsigned int object_index = 0;
    int len = 0;
    unsigned int priority = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    /*  only array properties can have array options */
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
		
		object_index = Analog_Output_Instance_To_Index(wp_data->object_instance);
		
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
						priority = wp_data->priority;
						status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_REAL, &wp_data->error_class, &wp_data->error_code);
            if (status) {
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
								if (priority && (priority <= BACNET_MAX_PRIORITY) &&
                    (priority != 6 /* reserved */ )) {
										status = Analog_Output_Present_Value_Set(wp_data->object_instance, value.type.Real, priority);
                } else if (priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
								status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL, &wp_data->error_class, &wp_data->error_code);
								if (status) {
										if (priority && (priority <= BACNET_MAX_PRIORITY) && (priority != 6 /* reserved */ )) {
												status = Analog_Output_Present_Value_Set(wp_data->object_instance, AO_LEVEL_NULL, priority);
										}	else if (priority == 6) {
												wp_data->error_class = ERROR_CLASS_PROPERTY;
												wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
										} else {
												wp_data->error_class = ERROR_CLASS_PROPERTY;
												wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
										}
								} else {
										status = false;
										wp_data->error_class = ERROR_CLASS_PROPERTY;
										wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
								}
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                object_index =
                    Analog_Output_Instance_To_Index(wp_data->object_instance);
                AO_Descr[object_index].Out_Of_Service =
                    value.type.Boolean;
            }
            break;
        case PROP_OBJECT_IDENTIFIER:
        case PROP_OBJECT_NAME:
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_EVENT_STATE:
        case PROP_UNITS:
        case PROP_PRIORITY_ARRAY:
        case PROP_RELINQUISH_DEFAULT:
        case PROP_DESCRIPTION:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }
		
		if(status)
		{
			if(AO_Descr[object_index].write_callback)
			{
				BACNET_APPLICATION_DATA_VALUE value;
				len = bacapp_decode_application_data(wp_data->application_data,
																						 wp_data->application_data_len,
																						 &value);
				
				if(len > 0)
				{ EVRECORD2(BACNET_EVQ_AO_VAL_DECODED, value.type.Real, len); }
				else
				{ EVRECORD2(BACNET_EVQ_AO_VAL_DECODE_FAIL, len, 0); }
				
				uint8_t id = bacQueue.call(AO_Descr[object_index].write_callback,
																	 (uint32_t)wp_data->object_property,
																	 (float)   value.type.Real);
					
				if(id > 0)
				{ EVRECORD2(BACNET_EVQ_AO_WRCB_CALLED, id, 0); }
				else
				{ EVRECORD2(BACNET_EVQ_AO_WRCB_FAILED, id, 0); }
			}
		}
		
    return status;
}

