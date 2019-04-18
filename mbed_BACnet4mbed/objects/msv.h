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
#ifndef MULTISTATE_VALUE_H
#define MULTISTATE_VALUE_H

#include <stdbool.h>
#include <stdint.h>
#include "bacdef.h"
#include "bacerror.h"
#include "rp.h"
#include "wp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
typedef void (*msv_callback_rd) (uint32_t objProp);
typedef void (*msv_callback_wr) (uint32_t objProp, uint8_t value);

typedef struct multistate_value_descr {
      uint32_t Object_Instance;
      char *Object_Name;
      char *Object_Description;
      uint8_t Number_Of_States;
      const char **State_Texts;
      msv_callback_rd read_callback;
      msv_callback_wr write_callback;
      unsigned Event_State:3;
      uint8_t Present_Value[16];
      BACNET_RELIABILITY Reliability;
      bool Out_Of_Service;
      bool Changed;
      bool PV_WriteProtected;
		} MULTISTATE_VALUE_DESCR;

    void Multistate_Value_Property_Lists(
        const int **pRequired,
        const int **pOptional,
        const int **pProprietary);

    bool Multistate_Value_Valid_Instance(
        uint32_t object_instance);
    unsigned Multistate_Value_Count(
        void);
    uint32_t Multistate_Value_Index_To_Instance(
        unsigned index);
    unsigned Multistate_Value_Instance_To_Index(
        uint32_t instance);

    int Multistate_Value_Read_Property(
        BACNET_READ_PROPERTY_DATA * rpdata);

    bool Multistate_Value_Write_Property(
        BACNET_WRITE_PROPERTY_DATA * wp_data);

    /* optional API */
    bool Multistate_Value_Object_Instance_Add(
        uint32_t instance);

    bool Multistate_Value_Object_Name(
        uint32_t object_instance,
        BACNET_CHARACTER_STRING * object_name);
    bool Multistate_Value_Name_Set(
        uint32_t object_instance,
        char *new_name);

    uint32_t Multistate_Value_Present_Value(
        uint32_t object_instance);
    bool Multistate_Value_Present_Value_Set(
        uint32_t object_instance,
        uint32_t value,
				unsigned priority);
		bool Multistate_Value_Present_Value_Relinquish(
				uint32_t object_instance,
				unsigned priority);

		bool Multistate_Value_Encode_Value_List(
				uint32_t object_instance,
				BACNET_PROPERTY_VALUE * value_list);
		bool Multistate_Value_Change_Of_Value(
        uint32_t object_instance);
		void Multistate_Value_Change_Of_Value_Clear(
        uint32_t object_instance);
		
    bool Multistate_Value_Out_Of_Service(
        uint32_t object_instance);
    void Multistate_Value_Out_Of_Service_Set(
        uint32_t object_instance,
        bool value);

    bool Multistate_Value_Description_Set(
        uint32_t object_instance,
        char *text_string);
//    bool Multistate_Value_State_Text_Set(
//        uint32_t object_instance,
//        uint32_t state_index,
//        char *new_name);
    bool Multistate_Value_Max_States_Set(
        uint32_t instance,
        uint32_t max_states_requested);

    void Multistate_Value_Init(
        void);


#ifdef TEST
#include "ctest.h"
    void testMultistateValue(
        Test * pTest);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
