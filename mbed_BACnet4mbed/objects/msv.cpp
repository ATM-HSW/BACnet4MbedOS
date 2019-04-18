/**************************************************************************
*
* Copyright (C) 2012 Steve Karg <skarg@users.sourceforge.net>
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

/* Multi-state Value Objects */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "config_bacnet.h" /* the custom stuff */
#include "msv.h"
#include "handlers.h"
#include "mbed.h"

#include "EvRec_BACnet4mbed.h"

extern EventQueue bacQueue;

/* we choose to have a NULL level in our system represented by */
/* a particular value.  When the priorities are not in use, they */
/* will be relinquished (i.e. set to the NULL level). */
#define MSV_LEVEL_NULL NULL
/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define MSV_RELINQUISH_DEFAULT 0

#define MULTISTATE_MAX_NUMBER_OF_STATES 255

extern MULTISTATE_VALUE_DESCR MSV_Descr[];

uint32_t NUM_MULTISTATE_VALUES;

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_NUMBER_OF_STATES,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    -1};

static const int Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_STATE_TEXT,
    -1};

static const int Properties_Proprietary[] = {
    -1};

void Multistate_Value_Property_Lists(
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

void Multistate_Value_Init(
    void)
{
    unsigned int i;
    unsigned j;
    /* initialize all the analog output priority arrays to NULL */
    for (i = 0; i < Multistate_Value_Count(); i++)
    {
        MSV_Descr[i].Event_State = EVENT_STATE_NORMAL;
        MSV_Descr[i].Out_Of_Service = false;
        MSV_Descr[i].Reliability = RELIABILITY_NO_FAULT_DETECTED;
        MSV_Descr[i].Changed = false;

        for (j = 0; j < BACNET_MAX_PRIORITY; j++)
            MSV_Descr[i].Present_Value[j] = MSV_LEVEL_NULL;
    }

    return;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Multistate_Value_Instance_To_Index(
    uint32_t object_instance)
{
    unsigned index = 0;

    for (index = 0; index < NUM_MULTISTATE_VALUES; index++)
    {
        if (MSV_Descr[index].Object_Instance == object_instance)
            return index;
    }

    return index;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Multistate_Value_Index_To_Instance(
    unsigned index)
{
    return MSV_Descr[index].Object_Instance;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Multistate_Value_Count(
    void)
{
    uint32_t i = 0;

    while (MSV_Descr[i].Object_Instance <= BACNET_MAX_INSTANCE)
        i++;

    NUM_MULTISTATE_VALUES = i;
    return i;
}

bool Multistate_Value_Valid_Instance(
    uint32_t object_instance)
{
    unsigned index = 0; /* offset from instance lookup */

    index = Multistate_Value_Instance_To_Index(object_instance);
    if (index < NUM_MULTISTATE_VALUES)
    {
        return true;
    }

    return false;
}

uint32_t Multistate_Value_Present_Value(
    uint32_t object_instance)
{
    uint8_t value = MSV_RELINQUISH_DEFAULT;
    uint8_t current_value = MSV_RELINQUISH_DEFAULT;
    unsigned i = 0;
    uint32_t index;

    index = Multistate_Value_Instance_To_Index(object_instance);

    if (index < NUM_MULTISTATE_VALUES)
    {
        for (i = 0; i < BACNET_MAX_PRIORITY; i++)
        {
            current_value = MSV_Descr[index].Present_Value[i];
            if (current_value != MSV_LEVEL_NULL)
            {
                value = MSV_Descr[index].Present_Value[i];
                break;
            }
        }
    }

    return value;
}

bool Multistate_Value_Present_Value_Set(
    uint32_t object_instance,
    uint32_t value,
    unsigned priority)
{
    bool status = false;
    uint8_t current_value = MSV_RELINQUISH_DEFAULT;
    unsigned index = 0; /* offset from instance lookup */

    index = Multistate_Value_Instance_To_Index(object_instance);

    if (index < NUM_MULTISTATE_VALUES)
    {
        if ((value <= MSV_Descr[index].Number_Of_States) && (priority <= BACNET_MAX_PRIORITY))
        {
            current_value = Multistate_Value_Present_Value(object_instance);

            if (current_value != (uint8_t)value)
                MSV_Descr[index].Changed = true;

            MSV_Descr[index].Present_Value[priority - 1] = (uint8_t)value;
            status = true;
        }
    }

    return status;
}

bool Multistate_Value_Out_Of_Service(
    uint32_t object_instance)
{
    bool value = false;
    unsigned index = 0;

    index = Multistate_Value_Instance_To_Index(object_instance);
    if (index < NUM_MULTISTATE_VALUES)
    {
        value = MSV_Descr[index].Out_Of_Service;
    }

    return value;
}

void Multistate_Value_Out_Of_Service_Set(
    uint32_t object_instance,
    bool value)
{
    unsigned index = 0;

    index = Multistate_Value_Instance_To_Index(object_instance);
    if (index < NUM_MULTISTATE_VALUES)
    {
        MSV_Descr[index].Out_Of_Service = value;
    }

    return;
}

bool Multistate_Value_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING *object_name)
{
    unsigned index = 0; /* offset from instance lookup */
    bool status = false;

    index = Multistate_Value_Instance_To_Index(object_instance);
    if (index < NUM_MULTISTATE_VALUES)
    {
        status = characterstring_init_ansi(object_name, MSV_Descr[index].Object_Name);
    }

    return status;
}

bool Multistate_Value_Object_Description(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING *object_description)
{
    unsigned index = 0; /* offset from instance lookup */
    bool status = false;

    index = Multistate_Value_Instance_To_Index(object_instance);

    if (index < NUM_MULTISTATE_VALUES)
    {
        status =
            characterstring_init_ansi(object_description,
                                      MSV_Descr[index].Object_Description);
    }

    return status;
}

bool Multistate_Value_State_Text(
    uint32_t object_instance,
    uint32_t state_index,
    BACNET_CHARACTER_STRING *state_text)
{
    bool status = false;
    unsigned index = 0; /* offset from instance lookup */

    index = Multistate_Value_Instance_To_Index(object_instance);

    if ((index < NUM_MULTISTATE_VALUES) &&
        (state_index > 0) &&
        (state_index <= MSV_Descr[index].Number_Of_States))
    {
        state_index--; // StateIndexes are enumerated according to BACnet-Standards [1-...]
        status = characterstring_init_ansi(state_text, MSV_Descr[index].State_Texts[state_index]);
    }

    return status;
}

/* returns true if value has changed */
bool Multistate_Value_Encode_Value_List(
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE *value_list)
{
    bool status = false;

    if (value_list)
    {
        value_list->propertyIdentifier = PROP_PRESENT_VALUE;
        value_list->propertyArrayIndex = BACNET_ARRAY_ALL;
        value_list->value.context_specific = false;
        value_list->value.tag = BACNET_APPLICATION_TAG_UNSIGNED_INT;
        value_list->value.next = NULL;
        value_list->value.type.Unsigned_Int =
            Multistate_Value_Present_Value(object_instance);
        value_list->priority = BACNET_NO_PRIORITY;
        value_list = value_list->next;
    }
    if (value_list)
    {
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
        if (Multistate_Value_Out_Of_Service(object_instance))
        {
            bitstring_set_bit(&value_list->value.type.Bit_String,
                              STATUS_FLAG_OUT_OF_SERVICE, true);
        }
        else
        {
            bitstring_set_bit(&value_list->value.type.Bit_String,
                              STATUS_FLAG_OUT_OF_SERVICE, false);
        }
        value_list->priority = BACNET_NO_PRIORITY;
        value_list->next = NULL;
    }
    status = Multistate_Value_Change_Of_Value(object_instance);

    return status;
}

bool Multistate_Value_Change_Of_Value(
    uint32_t object_instance)
{
    unsigned index = 0;

    index = Multistate_Value_Instance_To_Index(object_instance);
    if (index < NUM_MULTISTATE_VALUES)
    {
        return MSV_Descr[index].Changed;
    }

    return false;
}

void Multistate_Value_Change_Of_Value_Clear(
    uint32_t object_instance)
{
    unsigned index = 0;

    index = Multistate_Value_Instance_To_Index(object_instance);
    if (index < NUM_MULTISTATE_VALUES)
    {
        MSV_Descr[index].Changed = false;
    }
}

/* return apdu len, or BACNET_STATUS_ERROR on error */
int Multistate_Value_Read_Property(
    BACNET_READ_PROPERTY_DATA *rpdata)
{
    int len = 0;
    int apdu_len = 0; /* return value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    uint32_t present_value = 0;
    unsigned object_index = 0;
    unsigned i;
    bool state = false;
    uint8_t *apdu = NULL;

    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0))
    {
        return 0;
    }

    object_index = Multistate_Value_Instance_To_Index(rpdata->object_instance);

    if (MSV_Descr[object_index].read_callback)
    {
        uint8_t id = bacQueue.call(MSV_Descr[object_index].read_callback, (uint32_t)rpdata->object_property);

        if (id > 0)
        {
            EVRECORD2(BACNET_EVQ_MSV_RDCB_CALLED, id, 0);
        }
        else
        {
            EVRECORD2(BACNET_EVQ_MSV_RDCB_FAILED, id, 0);
        }
    }

    apdu = rpdata->application_data;
    switch (rpdata->object_property)
    {
    case PROP_OBJECT_IDENTIFIER:
        apdu_len =
            encode_application_object_id(&apdu[0],
                                         OBJECT_MULTI_STATE_VALUE, rpdata->object_instance);
        break;
        /* note: Name and Description don't have to be the same.
               You could make Description writable and different */
    case PROP_OBJECT_NAME:
        Multistate_Value_Object_Name(rpdata->object_instance, &char_string);
        apdu_len = encode_application_character_string(&apdu[0], &char_string);
        break;
    case PROP_DESCRIPTION:
        Multistate_Value_Object_Description(rpdata->object_instance,
                                            &char_string);
        apdu_len =
            encode_application_character_string(&apdu[0], &char_string);
        break;
    case PROP_OBJECT_TYPE:
        apdu_len =
            encode_application_enumerated(&apdu[0],
                                          OBJECT_MULTI_STATE_VALUE);
        break;
    case PROP_PRESENT_VALUE:
        present_value =
            Multistate_Value_Present_Value(rpdata->object_instance);
        apdu_len = encode_application_unsigned(&apdu[0], present_value);
        break;
    case PROP_STATUS_FLAGS:
        /* note: see the details in the standard on how to use these */
        bitstring_init(&bit_string);
        bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
        bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, false);
        if (Multistate_Value_Out_Of_Service(rpdata->object_instance))
        {
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                              true);
        }
        else
        {
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE,
                              false);
        }
        apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
        break;
    case PROP_EVENT_STATE:
        /* note: see the details in the standard on how to use this */
        apdu_len =
            encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
        break;
    case PROP_OUT_OF_SERVICE:
        state = Multistate_Value_Out_Of_Service(rpdata->object_instance);
        apdu_len = encode_application_boolean(&apdu[0], state);
        break;
    case PROP_NUMBER_OF_STATES:
        object_index = Multistate_Value_Instance_To_Index(rpdata->object_instance);
        apdu_len =
            encode_application_unsigned(&apdu[apdu_len],
                                        MSV_Descr[object_index].Number_Of_States);
        break;
    case PROP_PRIORITY_ARRAY:
        /* Array element zero is the number of elements in the array */
        if (rpdata->array_index == 0)
            apdu_len =
                encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
        /* if no index was specified, then try to encode the entire list */
        /* into one packet. */
        else if (rpdata->array_index == BACNET_ARRAY_ALL)
        {
            object_index = Multistate_Value_Instance_To_Index(rpdata->object_instance);

            for (i = 0; i < BACNET_MAX_PRIORITY; i++)
            {
                /* FIXME: check if we have room before adding it to APDU */
                present_value = (BACNET_BINARY_PV)MSV_Descr[object_index].Present_Value[i];
                if (present_value == BINARY_NULL)
                {
                    len = encode_application_null(&apdu[apdu_len]);
                }
                else
                {
                    len =
                        encode_application_unsigned(&apdu[apdu_len],
                                                    present_value);
                }
                /* add it if we have room */
                if ((apdu_len + len) < MAX_APDU)
                    apdu_len += len;
                else
                {
                    rpdata->error_class = ERROR_CLASS_SERVICES;
                    rpdata->error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                    apdu_len = BACNET_STATUS_ERROR;
                    break;
                }
            }
        }
        else
        {
            object_index = Multistate_Value_Instance_To_Index(rpdata->object_instance);

            if (rpdata->array_index <= BACNET_MAX_PRIORITY)
            {
                present_value = MSV_Descr[object_index].Present_Value[rpdata->array_index - 1];
                if (present_value == MSV_LEVEL_NULL)
                {
                    apdu_len = encode_application_null(&apdu[apdu_len]);
                }
                else
                {
                    apdu_len =
                        encode_application_unsigned(&apdu[apdu_len],
                                                    present_value);
                }
            }
            else
            {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                apdu_len = BACNET_STATUS_ERROR;
            }
        }
        break;
    case PROP_RELINQUISH_DEFAULT:
        present_value = MSV_RELINQUISH_DEFAULT;
        apdu_len = encode_application_unsigned(&apdu[0], present_value);
        break;

    case PROP_STATE_TEXT:
        if (rpdata->array_index == 0)
        {
            /* Array element zero is the number of elements in the array */
            apdu_len = encode_application_unsigned(&apdu[0], MSV_Descr[object_index].Number_Of_States);
        }
        else if (rpdata->array_index == BACNET_ARRAY_ALL)
        {
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            object_index = Multistate_Value_Instance_To_Index(rpdata->object_instance);

            for (i = 1; i <= MSV_Descr[object_index].Number_Of_States; i++)
            {
                Multistate_Value_State_Text(rpdata->object_instance, i, &char_string);

                /* FIXME: this might go beyond MAX_APDU length! */
                len = encode_application_character_string(&apdu[apdu_len], &char_string);

                /* add it if we have room */
                if ((apdu_len + len) < MAX_APDU)
                {
                    apdu_len += len;
                }
                else
                {
                    rpdata->error_class = ERROR_CLASS_SERVICES;
                    rpdata->error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                    apdu_len = BACNET_STATUS_ERROR;
                    break;
                }
            }
        }
        else
        {
            object_index = Multistate_Value_Instance_To_Index(rpdata->object_instance);

            if (rpdata->array_index <= MSV_Descr[object_index].Number_Of_States)
            {
                Multistate_Value_State_Text(rpdata->object_instance, rpdata->array_index, &char_string);
                apdu_len = encode_application_character_string(&apdu[0], &char_string);
            }
            else
            {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                apdu_len = BACNET_STATUS_ERROR;
            }
        }
        break;

    default:
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        apdu_len = BACNET_STATUS_ERROR;
        break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_STATE_TEXT) &&
        (rpdata->object_property != PROP_PRIORITY_ARRAY) &&
        (rpdata->array_index != BACNET_ARRAY_ALL))
    {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

/* returns true if successful */
bool Multistate_Value_Write_Property(
    BACNET_WRITE_PROPERTY_DATA *wp_data)
{
    bool status = false; /* return value */
    unsigned int object_index = 0;
    unsigned int priority = 0;
    uint32_t level = 0;
    int len = 0;
    BACNET_APPLICATION_DATA_VALUE value;

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
                                       wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0)
    {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_STATE_TEXT) &&
        (wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL))
    {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }

    object_index = Multistate_Value_Instance_To_Index(wp_data->object_instance);

    switch (wp_data->object_property)
    {
    case PROP_PRESENT_VALUE:
        priority = wp_data->priority;
        status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_UNSIGNED_INT, &wp_data->error_class, &wp_data->error_code);
        if (status)
        {
            if (MSV_Descr[object_index].PV_WriteProtected)
            {
              wp_data->error_class = ERROR_CLASS_PROPERTY;
              wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
            else
            {
              /* Command priority 6 is reserved for use by Minimum On/Off
               * algorithm and may not be used for other purposes in any
               * object. */
              if (priority && (priority <= BACNET_MAX_PRIORITY) &&
                  (priority != 6 /* reserved */) &&
                  (value.type.Unsigned_Int > 0) &&
                  (value.type.Unsigned_Int <= MSV_Descr[object_index].Number_Of_States))
              {
                  level = value.type.Unsigned_Int;
                  status = Multistate_Value_Present_Value_Set(wp_data->object_instance, (uint8_t)level, priority);
              }
              else if (priority == 6)
              {
                  status = false;
                  /* Command priority 6 is reserved for use by Minimum On/Off
                   * algorithm and may not be used for other purposes in any
                   * object. */
                  wp_data->error_class = ERROR_CLASS_PROPERTY;
                  wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
              }
              else
              {
                  status = false;
                  wp_data->error_class = ERROR_CLASS_PROPERTY;
                  wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
              }
            }
        }
        else
        {
            status = WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL, &wp_data->error_class, &wp_data->error_code);
            if (status)
            {

                if (priority && (priority <= BACNET_MAX_PRIORITY) && (priority != 6 /* reserved */))
                {
                    status = Multistate_Value_Present_Value_Set(wp_data->object_instance, MSV_LEVEL_NULL, priority);
                    /* Note: you could set the physical output here to the next
                           highest priority, or to the relinquish default if no
                           priorities are set.
                           However, if Out of Service is TRUE, then don't set the
                           physical output.  This comment may apply to the
                           main loop (i.e. check out of service before changing output) */
                }
                else if (priority == 6)
                {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                }
            }
            else
            {
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
        if (status)
        {
            Multistate_Value_Out_Of_Service_Set(wp_data->object_instance, value.type.Boolean);
        }
        break;
    case PROP_OBJECT_IDENTIFIER:
    case PROP_OBJECT_NAME:
    case PROP_OBJECT_TYPE:
    case PROP_STATUS_FLAGS:
    case PROP_EVENT_STATE:
    case PROP_NUMBER_OF_STATES:
    case PROP_DESCRIPTION:
    case PROP_PRIORITY_ARRAY:
    case PROP_RELINQUISH_DEFAULT:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
        break;
    default:
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
        break;
    }

    if (MSV_Descr[object_index].write_callback)
    {
        BACNET_APPLICATION_DATA_VALUE value;
				len = bacapp_decode_application_data(wp_data->application_data,
																						 wp_data->application_data_len,
																						 &value);
      
        uint8_t id = bacQueue.call(MSV_Descr[object_index].write_callback, (uint32_t)wp_data->object_property, (uint8_t) value.type.Unsigned_Int);

        if (id > 0)
        { EVRECORD2(BACNET_EVQ_MSV_WRCB_CALLED, id, 0); }
        else
        { EVRECORD2(BACNET_EVQ_MSV_WRCB_FAILED, id, 0); }
    }

    return status;
}

#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE *pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS *pErrorClass,
    BACNET_ERROR_CODE *pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

void testMultistateInput(
    Test *pTest)
{
    uint8_t apdu[MAX_APDU] = {0};
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Multistate_Value_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_MULTI_STATE_VALUE;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Multistate_Value_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_MULTISTATE_VALUE
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Multi-state Input", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testMultistateInput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void)ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif
#endif /* TEST */
