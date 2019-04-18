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
#ifndef BO_H
#define BO_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacerror.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
	
		typedef void (*bo_callback_rd)(uint32_t);
		typedef void (*bo_callback_wr)(uint32_t, bool);
	
		typedef struct binary_output_descr {
			uint32_t Object_Instance;
			char *Object_Name;
            char *Object_Description;
			BACNET_POLARITY Polarity;
			bo_callback_rd read_callback;
			bo_callback_wr write_callback;
			unsigned Event_State:3;
			uint8_t Present_Value[16];
			BACNET_RELIABILITY Reliability;
			bool Out_Of_Service;
		} BINARY_OUTPUT_DESCR;
		
    void Binary_Output_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool Binary_Output_Valid_Instance(
        uint32_t object_instance);
    unsigned Binary_Output_Count(
        void);
    uint32_t Binary_Output_Index_To_Instance(
        unsigned index);
    unsigned Binary_Output_Instance_To_Index(
        uint32_t instance);
    bool Binary_Output_Object_Instance_Add(
        uint32_t instance);

    bool Binary_Output_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Binary_Output_Name_Set(
        uint32_t object_instance,
        char *new_name);

    bool Binary_Output_Description(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_description);
    bool Binary_Output_Description_Set(
        uint32_t instance,
        char *new_name);

    char *Binary_Output_Inactive_Text(
        uint32_t instance);
    bool Binary_Output_Inactive_Text_Set(
        uint32_t instance,
        char *new_name);
    char *Binary_Output_Active_Text(
        uint32_t instance);
    bool Binary_Output_Active_Text_Set(
        uint32_t instance,
        char *new_name);

    int Binary_Output_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    bool Binary_Output_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

    BACNET_BINARY_PV Binary_Output_Present_Value(
        uint32_t instance);
    bool Binary_Output_Present_Value_Set(
        uint32_t instance,
        BACNET_BINARY_PV binary_value,
        unsigned priority);
    bool Binary_Output_Present_Value_Relinquish(
        uint32_t instance,
        unsigned priority);
    BACNET_POLARITY Binary_Output_Polarity(
        uint32_t instance);
    bool Binary_Output_Out_Of_Service(
        uint32_t instance);

    bool Binary_Output_Encode_Value_List(
        uint32_t object_instance,
        BACNET_PROPERTY_VALUE * value_list);
    bool Binary_Output_Change_Of_Value(
        uint32_t instance);
    void Binary_Output_Change_Of_Value_Clear(
        uint32_t instance);

    bool Binary_Output_Create(
        uint32_t object_instance);
    bool Binary_Output_Delete(
        uint32_t object_instance);
    void Binary_Output_Cleanup(
        void);
    void Binary_Output_Init(
        void);

#ifdef TEST
#include "ctest.h"
    void testBinaryOutput(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
