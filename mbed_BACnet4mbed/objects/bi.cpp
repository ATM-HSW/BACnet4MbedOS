/**************************************************************************
*
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
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

/* Binary Input Objects customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "config_bacnet.h"
#include "bi.h"
#include "handlers.h"
#include "mbed.h"

#include "EvRec_BACnet4mbed.h"

extern EventQueue bacQueue;


extern BINARY_INPUT_DESCR BI_Descr[];

uint32_t NUM_BINARY_INPUTS;

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Binary_Input_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_POLARITY,
    -1
};

static const int Binary_Input_Properties_Optional[] = {
    -1
};

static const int Binary_Input_Properties_Proprietary[] = {
    -1
};

void Binary_Input_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired) {
        *pRequired = Binary_Input_Properties_Required;
    }
    if (pOptional) {
        *pOptional = Binary_Input_Properties_Optional;
    }
    if (pProprietary) {
        *pProprietary = Binary_Input_Properties_Proprietary;
    }

    return;
}

void Binary_Input_Init(
    void)
{
    unsigned i;

    for (i = 0; i < Binary_Input_Count(); i++) {
      BI_Descr[i].Changed = false;
			if(BI_Descr[i].Polarity == POLARITY_NORMAL)
				BI_Descr[i].Present_Value = BINARY_INACTIVE;
			else
				BI_Descr[i].Present_Value = BINARY_ACTIVE;
			BI_Descr[i].Event_State = EVENT_STATE_NORMAL;
			BI_Descr[i].Out_Of_Service = false;
			BI_Descr[i].Reliability = RELIABILITY_NO_FAULT_DETECTED;
    }
}

/* we simply have 0-n object instances. */
bool Binary_Input_Valid_Instance(
    uint32_t object_instance)
{
    unsigned int index;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < NUM_BINARY_INPUTS)
        return true;

    return false;
}

/* we simply have 0-n object instances. */
unsigned Binary_Input_Count(
    void)
{
	uint32_t i=0;
	
	while(BI_Descr[i].Object_Instance <= BACNET_MAX_INSTANCE)
		i++;
	
	NUM_BINARY_INPUTS = i;
  return i;
}

/* we simply have 0-n object instances.*/
uint32_t Binary_Input_Index_To_Instance(
    unsigned index)
{
    return BI_Descr[index].Object_Instance;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Binary_Input_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = 0;

    for(index=0; index<NUM_BINARY_INPUTS; index++)
		{
			if(BI_Descr[index].Object_Instance == object_instance)
				return index;
		}

    return index;
}

BACNET_BINARY_PV Binary_Input_Present_Value(
    uint32_t object_instance)
{
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < NUM_BINARY_INPUTS) {
        value = BI_Descr[index].Present_Value;
    }

    return value;
}

bool Binary_Input_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    unsigned int index;
    bool status = false;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < NUM_BINARY_INPUTS) 
		{
        status = characterstring_init_ansi(object_name, BI_Descr[index].Object_Name);
    }

    return status;
}

bool Binary_Input_Object_Description(uint32_t                 object_instance,
                                      BACNET_CHARACTER_STRING *object_description)
{
    bool status = false;
		uint32_t index;
	
    index = Binary_Input_Instance_To_Index(object_instance);
	
    if (index < NUM_BINARY_INPUTS)
	{ status = characterstring_init_ansi(object_description, BI_Descr[index].Object_Description); }

    return status;
}

bool Binary_Input_Out_Of_Service(
    uint32_t object_instance)
{
    bool value = false;
    unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < NUM_BINARY_INPUTS) {
        value = BI_Descr[index].Out_Of_Service;
    }

    return value;
}

/* returns true if value has changed */
bool Binary_Input_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;

    if (value_list) {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_ENUMERATED;
        value_list->value.next = NULL;
        value_list->value.type.Enumerated =
            Binary_Input_Present_Value(object_instance);
        value_list->priority = BACNET_NO_PRIORITY;
        value_list = value_list->next;
    }
    if (value_list) {
        value_list->propertyIdentifier = PROP_STATUS_FLAGS;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_BIT_STRING;
        value_list->value.next = NULL;
        bitstring_init(&value_list->value.type.Bit_String);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_IN_ALARM, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_FAULT, false);
        bitstring_set_bit(&value_list->value.type.Bit_String,
            STATUS_FLAG_OVERRIDDEN, false);
        if (Binary_Input_Out_Of_Service(object_instance)) {
            bitstring_set_bit(&value_list->value.type.Bit_String,
                STATUS_FLAG_OUT_OF_SERVICE, true);
        } else {
            bitstring_set_bit(&value_list->value.type.Bit_String,
                STATUS_FLAG_OUT_OF_SERVICE, false);
        }
        value_list->priority = BACNET_NO_PRIORITY;
        value_list->next = NULL;
    }
    status = Binary_Input_Change_Of_Value(object_instance);

    return status;
}

bool Binary_Input_Change_Of_Value(
        uint32_t object_instance)
{
	  unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < NUM_BINARY_INPUTS) {
			return BI_Descr[index].Changed;
		}
		
		return false;
}

void Binary_Input_Change_Of_Value_Clear(
        uint32_t object_instance)
{
	  unsigned index = 0;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < NUM_BINARY_INPUTS) {
			BI_Descr[index].Changed = false;
		}	
}

/* return apdu length, or -1 on error */
/* assumption - object already exists, and has been bounds checked */
int Binary_Input_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    BACNET_BINARY_PV value = BINARY_INACTIVE;
    uint8_t *apdu = NULL;
	  uint32_t index;


    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
		index = Binary_Input_Instance_To_Index(rpdata->object_instance);
				
		if(BI_Descr[index].read_callback)
		{
		  uint8_t id = bacQueue.call(BI_Descr[index].read_callback, (uint32_t)rpdata->object_property);
		  
		  if(id > 0)
		  { EVRECORD2(BACNET_EVQ_BI_RDCB_CALLED, id, 0); }
		  else
		  { EVRECORD2(BACNET_EVQ_BI_RDCB_FAILED, id, 0); }
		}
		
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len =
                encode_application_object_id(&apdu[0], OBJECT_BINARY_INPUT,
                rpdata->object_instance);
            break;
        case PROP_OBJECT_NAME:
        case PROP_DESCRIPTION:
            Binary_Input_Object_Name(rpdata->object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len =
                encode_application_enumerated(&apdu[0], OBJECT_BINARY_INPUT);
            break;
        case PROP_PRESENT_VALUE:
            value = Binary_Input_Present_Value(rpdata->object_instance);
            apdu_len = encode_application_enumerated(&apdu[0], value);
            break;
        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, false);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            /* note: see the details in the standard on how to use this */
            apdu_len = encode_application_enumerated(&apdu[0], BI_Descr[index].Event_State);
            break;
        case PROP_OUT_OF_SERVICE:
            apdu_len = encode_application_boolean(&apdu[0], false);
            break;
        case PROP_POLARITY:
            apdu_len = encode_application_enumerated(&apdu[0], BI_Descr[index].Polarity);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

bool Binary_Input_Present_Value_Set(
    uint32_t object_instance,
    BACNET_BINARY_PV value)
{
    unsigned index = 0;
    bool status = false;

    index = Binary_Input_Instance_To_Index(object_instance);
    if (index < NUM_BINARY_INPUTS) {
			if (BI_Descr[index].Present_Value != value)
				BI_Descr[index].Changed = true;
			
			BI_Descr[index].Present_Value = value;
			status = true;
    }

    return status;
}
