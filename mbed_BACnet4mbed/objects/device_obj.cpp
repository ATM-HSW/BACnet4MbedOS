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

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "bacdef.h"
#include "bacdcode.h"
#include "bacstr.h"
#include "bacenum.h"
#include "apdu.h"
#include "dcc.h"
//#include "datalink.h"
//#include "rs485.h"
#include "version_bacnet.h"
#include "handlers.h"
/* objects */
#include "device_obj.h"
#include "bo.h"
#include "bi.h"
#include "bv.h"
#include "ao.h"
#include "ai.h"
#include "av.h"
#include "msv.h"
#include "mbed.h"

#include "valid_ip4.h"

#include "EvRec_BACnet4mbed.h"

extern DEVICE_OBJECT_DESCR Device_Descr;
extern EventQueue bacQueue;

/* forward prototype */
int Device_Read_Property_Local(
    BACNET_READ_PROPERTY_DATA * rpdata);
bool Device_Write_Property_Local(
    BACNET_WRITE_PROPERTY_DATA * wp_data);

static struct my_object_functions {
    BACNET_OBJECT_TYPE Object_Type;
    object_init_function Object_Init;
    object_count_function Object_Count;
    object_index_to_instance_function Object_Index_To_Instance;
    object_valid_instance_function Object_Valid_Instance;
    object_name_function Object_Name;
    read_property_function Object_Read_Property;
    write_property_function Object_Write_Property;
    rpm_property_lists_function Object_RPM_List;
    rr_info_function Object_RR_Info;
    object_iterate_function Object_Iterator;
    object_value_list_function Object_Value_List;
    object_cov_function Object_COV;
    object_cov_clear_function Object_COV_Clear;
    object_intrinsic_reporting_function Object_Intrinsic_Reporting;
} Object_Table[] = {
    {OBJECT_DEVICE,
				NULL,    /* don't init - recursive! */
				Device_Count,
				Device_Index_To_Instance,
				Device_Valid_Object_Instance_Number,
				Device_Object_Name,
				Device_Read_Property_Local, 
				Device_Write_Property_Local,
				Device_Property_Lists, 
				NULL, 
				NULL, 
				NULL, 
				NULL, 
				NULL, 
				NULL}, 
		{OBJECT_BINARY_OUTPUT, 
				Binary_Output_Init, 
				Binary_Output_Count,
				Binary_Output_Index_To_Instance, 
				Binary_Output_Valid_Instance,
				Binary_Output_Object_Name, 
				Binary_Output_Read_Property,
				Binary_Output_Write_Property, 
				Binary_Output_Property_Lists,
				NULL, 
				NULL, 
				NULL, 
				NULL, 
				NULL, 
				NULL}, 
		{OBJECT_BINARY_INPUT, 
				Binary_Input_Init, 
				Binary_Input_Count,
				Binary_Input_Index_To_Instance, 
				Binary_Input_Valid_Instance,
				Binary_Input_Object_Name, 
				Binary_Input_Read_Property,
				NULL, 
				Binary_Input_Property_Lists,
				NULL /* ReadRangeInfo */ ,
				NULL /* Iterator */ ,
				Binary_Input_Encode_Value_List,
				Binary_Input_Change_Of_Value,
				Binary_Input_Change_Of_Value_Clear,
				NULL /* Intrinsic Reporting */ },	
		{OBJECT_BINARY_VALUE, 
				Binary_Value_Init, 
				Binary_Value_Count,
				Binary_Value_Index_To_Instance, 
				Binary_Value_Valid_Instance,
				Binary_Value_Object_Name, 
				Binary_Value_Read_Property,
				Binary_Value_Write_Property, 
				Binary_Value_Property_Lists,
				NULL, 
				NULL, 
				Binary_Value_Encode_Value_List, 
				Binary_Value_Change_Of_Value, 
				Binary_Value_Change_Of_Value_Clear, 
				NULL},
		{OBJECT_ANALOG_OUTPUT,
				Analog_Output_Init, 
				Analog_Output_Count,
				Analog_Output_Index_To_Instance, 
				Analog_Output_Valid_Instance,
				Analog_Output_Object_Name, 
				Analog_Output_Read_Property,
				Analog_Output_Write_Property, 
				Analog_Output_Property_Lists,
				NULL, 
				NULL, 
				NULL,
				NULL,
				NULL, 
				NULL},		
		{OBJECT_ANALOG_INPUT,
				Analog_Input_Init, 
				Analog_Input_Count,
				Analog_Input_Index_To_Instance,
				Analog_Input_Valid_Instance,
				Analog_Input_Object_Name, 
				Analog_Input_Read_Property,
				Analog_Input_Write_Property,
				Analog_Input_Property_Lists,
				NULL,
				NULL, 
				Analog_Input_Encode_Value_List,
				Analog_Input_Change_Of_Value,
				Analog_Input_Change_Of_Value_Clear,
				NULL},
		{OBJECT_ANALOG_VALUE,
				Analog_Value_Init,
				Analog_Value_Count,
				Analog_Value_Index_To_Instance,
				Analog_Value_Valid_Instance,
				Analog_Value_Object_Name,
				Analog_Value_Read_Property,
				Analog_Value_Write_Property, 
				Analog_Value_Property_Lists,
				NULL, 
				NULL, 
				Analog_Value_Encode_Value_List,
				Analog_Value_Change_Of_Value,
				Analog_Value_Change_Of_Value_Clear, 
				NULL},
		{OBJECT_MULTI_STATE_VALUE, 
				Multistate_Value_Init, 
				Multistate_Value_Count,
				Multistate_Value_Index_To_Instance, 
				Multistate_Value_Valid_Instance,
				Multistate_Value_Object_Name, 
				Multistate_Value_Read_Property,
				Multistate_Value_Write_Property,
				Multistate_Value_Property_Lists,
				NULL, 
				NULL, 
				Multistate_Value_Encode_Value_List,
				Multistate_Value_Change_Of_Value,
				Multistate_Value_Change_Of_Value_Clear,
				NULL},
		{MAX_BACNET_OBJECT_TYPE,
				NULL /* Init */ ,
				NULL /* Count */ ,
				NULL /* Index_To_Instance */ ,
				NULL /* Valid_Instance */ ,
				NULL /* Object_Name */ ,
				NULL /* Read_Property */ ,
				NULL /* Write_Property */ ,
				NULL /* Property_Lists */ ,
				NULL /* ReadRangeInfo */ ,
				NULL /* Iterator */ ,
				NULL /* Value_Lists */ ,
				NULL /* COV */ ,
				NULL /* COV Clear */ ,
        NULL /* Intrinsic Reporting */ }
};

/* note: you really only need to define variables for
   properties that are writable or that may change.
   The properties that are constant can be hard coded
   into the read-property encoding. */
static uint32_t Database_Revision;
static BACNET_REINITIALIZED_STATE Reinitialize_State = BACNET_REINIT_IDLE;

/* These three arrays are used by the ReadPropertyMultiple handler */
static const int Device_Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_SYSTEM_STATUS,
    PROP_VENDOR_NAME,
    PROP_VENDOR_IDENTIFIER,
    PROP_MODEL_NAME,
    PROP_FIRMWARE_REVISION,
    PROP_APPLICATION_SOFTWARE_VERSION,
    PROP_PROTOCOL_VERSION,
    PROP_PROTOCOL_REVISION,
    PROP_PROTOCOL_SERVICES_SUPPORTED,
    PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED,
    PROP_OBJECT_LIST,
    PROP_MAX_APDU_LENGTH_ACCEPTED,
    PROP_SEGMENTATION_SUPPORTED,
    PROP_APDU_TIMEOUT,
    PROP_NUMBER_OF_APDU_RETRIES,
    PROP_DEVICE_ADDRESS_BINDING,
    PROP_DATABASE_REVISION,
    -1
};

static const int Device_Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_LOCATION,
		PROP_ACTIVE_COV_SUBSCRIPTIONS,
    -1
};

static const int Device_Properties_Proprietary[] = {
		PROP_GATEWAY_IP,			// DiestelGateway IP-Address
		PROP_GATEWAY_NMASK,		// DiestelGateway Netmask
		PROP_GATEWAY_GW,			// DiestelGateway Gateway
		PROP_GATEWAY_DHCP,		// DiestelGateway DHCP setting
    -1
};

static struct my_object_functions *Device_Objects_Find_Functions(
    BACNET_OBJECT_TYPE Object_Type)
{
    struct my_object_functions *pObject = NULL;

    pObject = &Object_Table[0];
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        /* handle each object type */
        if (pObject->Object_Type == Object_Type) {
            return (pObject);
        }

        pObject++;
    }

    return (NULL);
}

static int Read_Property_Common(
    struct my_object_functions *pObject,
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = BACNET_STATUS_ERROR;
//    BACNET_CHARACTER_STRING char_string;
    uint8_t *apdu = NULL;

    if ((rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            /*  only array properties can have array options */
            if (rpdata->array_index != BACNET_ARRAY_ALL) {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
                apdu_len = BACNET_STATUS_ERROR;
            } else {
                /* Device Object exception: requested instance
                   may not match our instance if a wildcard */
                if (rpdata->object_type == OBJECT_DEVICE) {
                    rpdata->object_instance = Device_Descr.object_instance;
                }
                apdu_len =
                    encode_application_object_id(&apdu[0], rpdata->object_type,
                    rpdata->object_instance);
            }
            break;
//        case PROP_OBJECT_NAME:
//            /*  only array properties can have array options */
//            if (rpdata->array_index != BACNET_ARRAY_ALL) {
//                rpdata->error_class = ERROR_CLASS_PROPERTY;
//                rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
//                apdu_len = BACNET_STATUS_ERROR;
//            } else {
//                characterstring_init_ansi(&char_string, "");
//                if (pObject->Object_Name) {
//                    (void) pObject->Object_Name(rpdata->object_instance,
//                        &char_string);
//                }
//                apdu_len =
//                    encode_application_character_string(&apdu[0],
//                    &char_string);
//            }
//            break;
        case PROP_OBJECT_TYPE:
            /*  only array properties can have array options */
            if (rpdata->array_index != BACNET_ARRAY_ALL) {
                rpdata->error_class = ERROR_CLASS_PROPERTY;
                rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
                apdu_len = BACNET_STATUS_ERROR;
            } else {
                apdu_len =
                    encode_application_enumerated(&apdu[0],
                    rpdata->object_type);
            }
            break;
        default:
            if (pObject->Object_Read_Property) {
                apdu_len = pObject->Object_Read_Property(rpdata);
            }
            break;
    }

    return apdu_len;
}

/* Encodes the property APDU and returns the length,
   or sets the error, and returns BACNET_STATUS_ERROR */
int Device_Read_Property(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = BACNET_STATUS_ERROR;
    struct my_object_functions *pObject = NULL;

    /* initialize the default return values */
    pObject = Device_Objects_Find_Functions(rpdata->object_type);
    if (pObject) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(rpdata->object_instance)) {
            apdu_len = Read_Property_Common(pObject, rpdata);
        } else {
            rpdata->error_class = ERROR_CLASS_OBJECT;
            rpdata->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        }
    } else {
        rpdata->error_class = ERROR_CLASS_OBJECT;
        rpdata->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    }

    return apdu_len;
}

bool Device_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;
    struct my_object_functions *pObject = NULL;

    /* initialize the default return values */
    pObject = Device_Objects_Find_Functions(wp_data->object_type);
    if (pObject) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(wp_data->object_instance)) {
            if (pObject->Object_Write_Property) {
                status = pObject->Object_Write_Property(wp_data);
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            }
        } else {
            wp_data->error_class = ERROR_CLASS_OBJECT;
            wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
        }
    } else {
        wp_data->error_class = ERROR_CLASS_OBJECT;
        wp_data->error_code = ERROR_CODE_UNKNOWN_OBJECT;
    }

    return status;
}

/** Looks up the requested Object to see if the functionality is supported.
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @return True if the object instance supports this feature.
 */
bool Device_Value_List_Supported(
    BACNET_OBJECT_TYPE object_type)
{
    bool status = false;
    struct my_object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Value_List) {
            status = true;
        }
    }

    return (status);
}

/** Looks up the requested Object, and fills the Property Value list.
 * If the Object or Property can't be found, returns false.
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance number to be looked up.
 * @param [out] The value list
 * @return True if the object instance supports this feature and value changed.
 */
bool Device_Encode_Value_List(
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_PROPERTY_VALUE * value_list)
{
    bool status = false;        /* Ever the pessamist! */
    struct my_object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions(object_type);
    if (pObject != NULL) {
        if (pObject->Object_Valid_Instance &&
            pObject->Object_Valid_Instance(object_instance)) {
            if (pObject->Object_Value_List) {
                status =
                    pObject->Object_Value_List(object_instance, value_list);
            }
        }
    }

    return status;
}

/** Checks the COV flag in the requested Object
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance to be looked up.
 * @return True if the COV flag is set
 */
bool Device_COV(
	BACNET_OBJECT_TYPE object_type,
	uint32_t object_instance)
{
	bool status = false;
	struct my_object_functions *pObject = NULL;

	pObject = Device_Objects_Find_Functions(object_type);
	if (pObject != NULL) {
			if (pObject->Object_Valid_Instance &&
					pObject->Object_Valid_Instance(object_instance)) {
					if (pObject->Object_COV) {
							status = pObject->Object_COV(object_instance);
					}
			}
	}

	return status;
}

/** Clears the COV flag in the requested Object
 * @ingroup ObjHelpers
 * @param [in] The object type to be looked up.
 * @param [in] The object instance to be looked up.
 */
void Device_COV_Clear(
	BACNET_OBJECT_TYPE object_type,
	uint32_t object_instance)
{
	struct my_object_functions *pObject = NULL;

	pObject = Device_Objects_Find_Functions(object_type);
	if (pObject != NULL) {
			if (pObject->Object_Valid_Instance &&
					pObject->Object_Valid_Instance(object_instance)) {
					if (pObject->Object_COV_Clear) {
							pObject->Object_COV_Clear(object_instance);
					}
			}
	}
}
		
unsigned property_list_count(
    const int *pList)
{
    unsigned property_count = 0;

    if (pList) {
        while (*pList != -1) {
            property_count++;
            pList++;
        }
    }

    return property_count;
}

/* for a given object type, returns the special property list */
void Device_Objects_Property_List(
    BACNET_OBJECT_TYPE object_type,
    struct special_property_list_t *pPropertyList)
{
    struct my_object_functions *pObject = NULL;

    pPropertyList->Required.pList = NULL;
    pPropertyList->Optional.pList = NULL;
    pPropertyList->Proprietary.pList = NULL;

    /* If we can find an entry for the required object type
     * and there is an Object_List_RPM fn ptr then call it
     * to populate the pointers to the individual list counters.
     */

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_RPM_List != NULL)) {
        pObject->Object_RPM_List(&pPropertyList->Required.pList,
            &pPropertyList->Optional.pList, &pPropertyList->Proprietary.pList);
    }

    /* Fetch the counts if available otherwise zero them */
    pPropertyList->Required.count =
        pPropertyList->Required.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Required.pList);

    pPropertyList->Optional.count =
        pPropertyList->Optional.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Optional.pList);

    pPropertyList->Proprietary.count =
        pPropertyList->Proprietary.pList ==
        NULL ? 0 : property_list_count(pPropertyList->Proprietary.pList);

    return;
}

void Device_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
    if (pRequired)
        *pRequired = Device_Properties_Required;
    if (pOptional)
        *pOptional = Device_Properties_Optional;
    if (pProprietary)
        *pProprietary = Device_Properties_Proprietary;

    return;
}

unsigned Device_Count(
    void)
{
    return 1;
}

uint32_t Device_Index_To_Instance(
    unsigned index)
{
    index = index;
    return Device_Descr.object_instance;
}

bool Device_Set_Object_Instance_Number(
    uint32_t object_id)
{
    bool status = true; /* return value */

    if (object_id <= BACNET_MAX_INSTANCE) {
        /* Make the change and update the database revision */
        Device_Descr.object_instance = object_id;
        Device_Inc_Database_Revision();
    } else
        status = false;

    return status;
}

bool Device_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    bool status = false;

    if (object_instance == Device_Descr.object_instance) {
        status = characterstring_init_ansi(object_name, Device_Descr.object_name);
    }

    return status;
}

bool Device_Set_Object_Name(
    BACNET_CHARACTER_STRING * object_name)
{
    bool status = false;        /*return value */

    if (!characterstring_ansi_same(object_name, Device_Descr.object_name)) {
        /* Make the change and update the database revision */
        status = characterstring_ansi_copy(Device_Descr.object_name, MAX_DEV_NAME_LEN, object_name);
        Device_Inc_Database_Revision();
    }

    return status;
}

bool Device_Reinitialize(
    BACNET_REINITIALIZE_DEVICE_DATA * rd_data)
{
    bool status = false;

    if (characterstring_ansi_same(&rd_data->password, "stm32-challenge")) {
        Reinitialize_State = rd_data->state;
        dcc_set_status_duration(COMMUNICATION_ENABLE, 0);
        /* Note: you could use a mix of state
           and password to multiple things */
        /* note: you probably want to restart *after* the
           simple ack has been sent from the return handler
           so just set a flag from here */
        status = true;
    } else {
        rd_data->error_class = ERROR_CLASS_SECURITY;
        rd_data->error_code = ERROR_CODE_PASSWORD_FAILURE;
    }

    return status;
}

BACNET_REINITIALIZED_STATE Device_Reinitialized_State(
    void)
{
    return Reinitialize_State;
}

void Device_Init(
    object_functions_t * object_table)
{
    struct my_object_functions *pObject = NULL;

    /* we don't use the object table passed in
       since there is extra stuff we don't need in there. */
    (void) object_table;
    /* our local object table */
    pObject = &Object_Table[0];
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Init) {
            pObject->Object_Init();
        }
        pObject++;
    }
    dcc_set_status_duration(COMMUNICATION_ENABLE, 0);
		
    if (Device_Descr.object_instance >= BACNET_MAX_INSTANCE) {
        Device_Descr.object_instance = 103;
        srand(Device_Descr.object_instance);
    }
		
		Device_Set_Vendor_Identifier(BACNET_VENDOR_IDENTIFIER);
		Device_Set_Vendor_Name(BACNET_VENDOR_NAME);
		Device_Set_Description(BACNET_DEVICE_DESCRIPTION);
		Device_Set_Model_Name(BACNET_MODEL_NAME);
		Device_Set_Location(BACNET_LOCATION);
		Device_Set_Application_Software_Version(BACNET_APPLICATION_VER);
		Device_Set_System_Status(STATUS_OPERATIONAL, false);
}

/* methods to manipulate the data */
uint32_t Device_Object_Instance_Number(
    void)
{
    return Device_Descr.object_instance;
}

bool Device_Valid_Object_Instance_Number(
    uint32_t object_id)
{
    return (Device_Descr.object_instance == object_id);
}

BACNET_DEVICE_STATUS Device_System_Status(
    void)
{
    return Device_Descr.system_status;
}

int Device_Set_System_Status(
    BACNET_DEVICE_STATUS status,
    bool local)
{
    /*return value - 0 = ok, -1 = bad value, -2 = not allowed */
    int result = -1;

    if (status < MAX_DEVICE_STATUS) {
        Device_Descr.system_status = status;
        result = 0;
    }

    return result;
}

uint16_t Device_Vendor_Identifier(
    void)
{
    return Device_Descr.Vendor_Identifier;
}

void Device_Set_Vendor_Name(const char* vendor_name)
{
	strcpy(Device_Descr.Vendor_Name, vendor_name);
}

BACNET_SEGMENTATION Device_Segmentation_Supported(
    void)
{
    return SEGMENTATION_NONE; 

}

uint32_t Device_Database_Revision(
    void)
{
    return Database_Revision;
}

void Device_Inc_Database_Revision(
    void)
{
    Database_Revision++;
}

/* Since many network clients depend on the object list */
/* for discovery, it must be consistent! */
unsigned Device_Object_List_Count(
    void)
{
    unsigned count = 0; /* number of objects */
    struct my_object_functions *pObject = NULL;

    /* initialize the default return values */
    pObject = &Object_Table[0];
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Count) {
            count += pObject->Object_Count();
        }
        pObject++;
    }

    return count;
}

bool Device_Object_List_Identifier(
    unsigned array_index,
    int *object_type,
    uint32_t * instance)
{
    bool status = false;
    unsigned count = 0;
    unsigned object_index = 0;
    struct my_object_functions *pObject = NULL;

    /* array index zero is length - so invalid */
    if (array_index == 0) {
        return status;
    }
    object_index = array_index - 1;
    /* initialize the default return values */
    pObject = &Object_Table[0];
    while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
        if (pObject->Object_Count && pObject->Object_Index_To_Instance) {
            object_index -= count;
            count = pObject->Object_Count();
            if (object_index < count) {
                *object_type = pObject->Object_Type;
                *instance = pObject->Object_Index_To_Instance(object_index);
                status = true;
                break;
            }
        }
        pObject++;
    }

    return status;
}

bool Device_Valid_Object_Name(
    BACNET_CHARACTER_STRING * object_name1,
    int *object_type,
    uint32_t * object_instance)
{
    bool found = false;
    int type = 0;
    uint32_t instance;
    unsigned max_objects = 0, i = 0;
    bool check_id = false;
    BACNET_CHARACTER_STRING object_name2;
    struct my_object_functions *pObject = NULL;

    max_objects = Device_Object_List_Count();
    for (i = 1; i <= max_objects; i++) {
        check_id = Device_Object_List_Identifier(i, &type, &instance);
        if (check_id) {
            pObject = Device_Objects_Find_Functions((BACNET_OBJECT_TYPE) type);
            if ((pObject != NULL) && (pObject->Object_Name != NULL) &&
                (pObject->Object_Name(instance, &object_name2) &&
                    characterstring_same(object_name1, &object_name2))) {
                found = true;
                if (object_type) {
                    *object_type = type;
                }
                if (object_instance) {
                    *object_instance = instance;
                }
                break;
            }
        }
    }

    return found;
}

bool Device_Valid_Object_Id(
    int object_type,
    uint32_t object_instance)
{
    bool status = false;        /* return value */
    struct my_object_functions *pObject = NULL;

    pObject = Device_Objects_Find_Functions((BACNET_OBJECT_TYPE) object_type);
    if ((pObject != NULL) && (pObject->Object_Valid_Instance != NULL)) {
        status = pObject->Object_Valid_Instance(object_instance);
    }

    return status;
}

bool Device_Object_Name_Copy(
    BACNET_OBJECT_TYPE object_type,
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    struct my_object_functions *pObject = NULL;
    bool found = false;

    pObject = Device_Objects_Find_Functions(object_type);
    if ((pObject != NULL) && (pObject->Object_Name != NULL)) {
        found = pObject->Object_Name(object_instance, object_name);
    }

    return found;
}

bool Device_IP_Address(BACNET_CHARACTER_STRING *ip)
{
	bool status = false;
	char str[16];
	
	if(Device_Descr.ip_addr.addr)
	{
		ip4addr_ntoa_r(&Device_Descr.ip_addr, str, 16);
		status = characterstring_init_ansi(ip, str);
	}
	
	return status;
}

bool Device_Set_IP_Address(char *ip)
{
	bool status = false;

	if(ip4addr_aton(ip, &Device_Descr.ip_addr))
	{
		status = true;
	}
	
	return status;
}

bool Device_Netmask(BACNET_CHARACTER_STRING *netmask)
{
	bool status = false;
	char str[16];
	
	if(Device_Descr.netmask.addr)
	{
		ip4addr_ntoa_r(&Device_Descr.netmask, str, 16);
		status = characterstring_init_ansi(netmask, str);
	}
	
	return status;
}

bool Device_Set_Netmask_Address(char *netmask)
{
	bool status = false;

	if(ip4addr_aton(netmask, &Device_Descr.netmask))
	{
		status = true;
	}
	
	return status;
}

bool Device_Gateway_Address(BACNET_CHARACTER_STRING *gw)
{
	bool status = false;
	char str[16];
	
	if(Device_Descr.gateway.addr)
	{
		ip4addr_ntoa_r(&Device_Descr.gateway, str, 16);
		status = characterstring_init_ansi(gw, str);
	}
	
	return status;
}

bool Device_Set_Gateway_Address(char *gw)
{
	bool status = false;

	if(ip4addr_aton(gw, &Device_Descr.gateway))
	{
		status = true;
	}
	
	return status;
}

bool Device_DHCP_Setting(void)
{
	return Device_Descr.dhcp;
}

bool Device_Set_DHCP_Setting(bool dhcp)
{
	Device_Descr.dhcp = dhcp;	
	return true;
}

/* return the length of the apdu encoded or BACNET_STATUS_ERROR for error */
int Device_Read_Property_Local(
    BACNET_READ_PROPERTY_DATA * rpdata)
{
    int apdu_len = 0;   /* return value */
    int len = 0;        /* apdu len intermediate value */
    BACNET_BIT_STRING bit_string;
    BACNET_CHARACTER_STRING char_string;
    unsigned i = 0;
    int object_type = 0;
    uint32_t instance = 0;
    unsigned count = 0;
    uint8_t *apdu = NULL;
    struct my_object_functions *pObject = NULL;

    if ((rpdata->application_data == NULL) ||
        (rpdata->application_data_len == 0)) {
        return 0;
    }
				
		if(Device_Descr.read_callback)
		{
		  uint8_t id = bacQueue.call(Device_Descr.read_callback, (uint32_t)rpdata->object_property);
		  
		  if(id > 0)
		  { EVRECORD2(BACNET_EVQ_DEVOBJ_RDCB_CALLED, id, 0); }
		  else
		  { EVRECORD2(BACNET_EVQ_DEVOBJ_RDCB_FAILED, id, 0); }
		}
		
    apdu = rpdata->application_data;
    switch (rpdata->object_property) {
				case PROP_OBJECT_NAME:
					Device_Object_Name(Device_Descr.object_instance, &char_string);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
						break;
        case PROP_DESCRIPTION:
            characterstring_init_ansi(&char_string, Device_Descr.Description);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_LOCATION:
            characterstring_init_ansi(&char_string, Device_Descr.Location);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_SYSTEM_STATUS:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                Device_System_Status());
            break;
        case PROP_VENDOR_NAME:
            characterstring_init_ansi(&char_string, Device_Descr.Vendor_Name);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_VENDOR_IDENTIFIER:
            apdu_len = encode_application_unsigned(&apdu[0], Device_Vendor_Identifier());
            break;
        case PROP_MODEL_NAME:
						characterstring_init_ansi(&char_string, Device_Descr.Model_Name);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_FIRMWARE_REVISION:
            characterstring_init_ansi(&char_string, BACnet_Version);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_APPLICATION_SOFTWARE_VERSION:
            characterstring_init_ansi(&char_string, Device_Descr.Application_Software_Version);
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_PROTOCOL_VERSION:
            apdu_len =
                encode_application_unsigned(&apdu[0], BACNET_PROTOCOL_VERSION);
            break;
        case PROP_PROTOCOL_REVISION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                BACNET_PROTOCOL_REVISION);
            break;
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
            /* Note: list of services that are executed, not initiated. */
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_BACNET_SERVICES_SUPPORTED; i++) {
                /* automatic lookup based on handlers set */
                bitstring_set_bit(&bit_string, (uint8_t) i,
                    apdu_service_supported((BACNET_SERVICES_SUPPORTED) i));
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
            /* Note: this is the list of objects that can be in this device,
               not a list of objects that this device can access */
            bitstring_init(&bit_string);
            for (i = 0; i < MAX_ASHRAE_OBJECT_TYPE; i++) {
                /* initialize all the object types to not-supported */
                bitstring_set_bit(&bit_string, (uint8_t) i, false);
            }
            /* set the object types with objects to supported */
            i = 0;
            pObject = &Object_Table[i];
            while (pObject->Object_Type < MAX_BACNET_OBJECT_TYPE) {
                if ((pObject->Object_Count) && (pObject->Object_Count() > 0)) {
                    bitstring_set_bit(&bit_string, pObject->Object_Type, true);
                }
                pObject++;
            }
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_OBJECT_LIST:
            count = Device_Object_List_Count();
            /* Array element zero is the number of objects in the list */
            if (rpdata->array_index == 0)
                apdu_len = encode_application_unsigned(&apdu[0], count);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet.  Note that more than likely you will have */
            /* to return an error if the number of encoded objects exceeds */
            /* your maximum APDU size. */
            else if (rpdata->array_index == BACNET_ARRAY_ALL) {
                for (i = 1; i <= count; i++) {
                    if (Device_Object_List_Identifier(i, &object_type,
                            &instance)) {
                        len =
                            encode_application_object_id(&apdu[apdu_len],
                            object_type, instance);
                        apdu_len += len;
                        /* assume next one is the same size as this one */
                        /* can we all fit into the APDU? */
                        if ((apdu_len + len) >= MAX_APDU) {
                            /* Abort response */
                            rpdata->error_code =
                                ERROR_CODE_ABORT_SEGMENTATION_NOT_SUPPORTED;
                            apdu_len = BACNET_STATUS_ABORT;
                            break;
                        }
                    } else {
                        /* error: internal error? */
                        rpdata->error_class = ERROR_CLASS_SERVICES;
                        rpdata->error_code = ERROR_CODE_OTHER;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else {
                if (Device_Object_List_Identifier(rpdata->array_index,
                        &object_type, &instance))
                    apdu_len =
                        encode_application_object_id(&apdu[0], object_type,
                        instance);
                else {
                    rpdata->error_class = ERROR_CLASS_PROPERTY;
                    rpdata->error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }
            break;
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
            apdu_len = encode_application_unsigned(&apdu[0], MAX_APDU);
            break;
        case PROP_SEGMENTATION_SUPPORTED:
            apdu_len =
                encode_application_enumerated(&apdu[0],
                Device_Segmentation_Supported());
            break;
        case PROP_APDU_TIMEOUT:
            apdu_len = encode_application_unsigned(&apdu[0], apdu_timeout());
            break;
        case PROP_NUMBER_OF_APDU_RETRIES:
            apdu_len = encode_application_unsigned(&apdu[0], apdu_retries());
            break;
        case PROP_DEVICE_ADDRESS_BINDING:
            /* FIXME: encode the list here, if it exists */
            break;
        case PROP_DATABASE_REVISION:
            apdu_len =
                encode_application_unsigned(&apdu[0],
                Device_Database_Revision());
            break;
				case PROP_GATEWAY_IP:	// IP address
						Device_IP_Address(&char_string);
						apdu_len = encode_application_character_string(&apdu[0], &char_string);
						break;
				case PROP_GATEWAY_NMASK: // Netmask
					Device_Netmask(&char_string);
					apdu_len = encode_application_character_string(&apdu[0], &char_string);
					break;
				case PROP_GATEWAY_GW:	// Gateway
					Device_Gateway_Address(&char_string);
					apdu_len = encode_application_character_string(&apdu[0], &char_string);
					break;
				case PROP_GATEWAY_DHCP: // DHCP-Setting
						apdu_len = encode_application_boolean(&apdu[0], Device_DHCP_Setting());
					break;
				
				case PROP_ACTIVE_COV_SUBSCRIPTIONS:
            apdu_len = handler_cov_encode_subscriptions(&apdu[0], MAX_APDU);
            break;
        default:
            rpdata->error_class = ERROR_CLASS_PROPERTY;
            rpdata->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (rpdata->object_property != PROP_OBJECT_LIST) &&
        (rpdata->array_index != BACNET_ARRAY_ALL)) {
        rpdata->error_class = ERROR_CLASS_PROPERTY;
        rpdata->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }

    return apdu_len;
}

bool Device_Write_Property_Local(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool status = false;        /* return value - false=error */
    int len = 0;
    uint8_t encoding = 0;
    size_t length = 0;
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
    if ((wp_data->object_property != PROP_OBJECT_LIST) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
    switch (wp_data->object_property) {
        case PROP_OBJECT_IDENTIFIER:
            if (value.tag == BACNET_APPLICATION_TAG_OBJECT_ID) {
                if ((value.type.Object_Id.type == OBJECT_DEVICE) &&
                    (Device_Set_Object_Instance_Number(value.type.
                            Object_Id.instance))) {
                    /* we could send an I-Am broadcast to let the world know */
                    status = true;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }
            break;

				case PROP_LOCATION:
            status =
                WPValidateString(&value, MAX_DEV_LOC_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Device_Set_Location(characterstring_value(&value.
                        type.Character_String));
            }
            break;

        case PROP_DESCRIPTION:
            status =
                WPValidateString(&value, MAX_DEV_DESC_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Device_Set_Description(characterstring_value(&value.
                        type.Character_String));
            }
            break;
        case PROP_MODEL_NAME:
            status =
                WPValidateString(&value, MAX_DEV_MOD_LEN, true,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                Device_Set_Model_Name(characterstring_value(&value.
                        type.Character_String));
            }
            break;
            
					case PROP_OBJECT_NAME:
							if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
									length = characterstring_length(&value.type.Character_String);
									if (length < 1) {
											wp_data->error_class = ERROR_CLASS_PROPERTY;
											wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
									} else if (length < MAX_DEV_NAME_LEN){
											encoding =
													characterstring_encoding(&value.type.Character_String);
											if (encoding < MAX_CHARACTER_STRING_ENCODING) {
													/* All the object names in a device must be unique. */
													if (Device_Valid_Object_Name(&value.type.
																	Character_String, NULL, NULL)) {
															wp_data->error_class = ERROR_CLASS_PROPERTY;
															wp_data->error_code = ERROR_CODE_DUPLICATE_NAME;
													} else {
															Device_Set_Object_Name(&value.type.
																	Character_String);
															status = true;
													}
											} else {
													wp_data->error_class = ERROR_CLASS_PROPERTY;
													wp_data->error_code =
															ERROR_CODE_CHARACTER_SET_NOT_SUPPORTED;
											}
									} else {
											wp_data->error_class = ERROR_CLASS_PROPERTY;
											wp_data->error_code =
													ERROR_CODE_NO_SPACE_TO_WRITE_PROPERTY;
									}
							} else {
									wp_data->error_class = ERROR_CLASS_PROPERTY;
									wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
							}
							break;
//        case 9600:
//            if (value.tag == BACNET_APPLICATION_TAG_UNSIGNED_INT) {
//                if ((value.type.Unsigned_Int <= 115200) &&
//                    (rs485_baud_rate_set(value.type.Unsigned_Int))) {
//                    status = true;
//                } else {
//                    wp_data->error_class = ERROR_CLASS_PROPERTY;
//                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
//                }
//            } else {
//                wp_data->error_class = ERROR_CLASS_PROPERTY;
//                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
//            }
//            break;
        case PROP_OBJECT_TYPE:
        case PROP_VENDOR_NAME:
        case PROP_VENDOR_IDENTIFIER:
        case PROP_FIRMWARE_REVISION:
        case PROP_APPLICATION_SOFTWARE_VERSION:
        case PROP_PROTOCOL_VERSION:
        case PROP_PROTOCOL_REVISION:
        case PROP_PROTOCOL_SERVICES_SUPPORTED:
        case PROP_PROTOCOL_OBJECT_TYPES_SUPPORTED:
        case PROP_OBJECT_LIST:
        case PROP_MAX_APDU_LENGTH_ACCEPTED:
        case PROP_SEGMENTATION_SUPPORTED:
        case PROP_DEVICE_ADDRESS_BINDING:
        case PROP_ACTIVE_COV_SUBSCRIPTIONS:
          
        case PROP_DATABASE_REVISION:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }
    /* not using len at this time */
    len = len;
    
    //
    // Handle proprietary Diestelgateway DevObj-Properties (write)
		if(Device_Descr.write_callback)
		{
		  uint8_t id = bacQueue.call(Device_Descr.write_callback, (uint32_t)wp_data->object_property);
		  
		  if(id > 0)
		  { EVRECORD2(BACNET_EVQ_DEVOBJ_WRCB_CALLED, id, 0); }
		  else
		  { EVRECORD2(BACNET_EVQ_DEVOBJ_WRCB_FAILED, id, 0); }
		}
    return status;
}

bool Device_Set_Application_Software_Version(const char *name)
{
	strcpy(Device_Descr.Application_Software_Version, name);
	
	return true;
}

bool Device_Set_Description(const char *name)
{	
	strcpy(Device_Descr.Description, name);
	
	return true;
}

bool Device_Set_Location(const char *name)
{
	strcpy(Device_Descr.Location, name);
	
	return true;	
}

bool Device_Set_Model_Name(const char *name)
{
	strcpy(Device_Descr.Model_Name, name);
	
	return true; 
}

void Device_Set_Vendor_Identifier(uint16_t vendor_id)
{
	Device_Descr.Vendor_Identifier = vendor_id;
}
