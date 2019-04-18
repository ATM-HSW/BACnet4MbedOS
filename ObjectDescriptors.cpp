/*----------*/
/* Includes */
/*----------*/
#include "ObjectDescriptors.h"
#include "io_functions.h"


/*---------*/
/* Defines */
/*---------*/
#ifndef DEFAULT_OBJ_INSTANCE
  #define DEFAULT_OBJ_INSTANCE 1111
#endif

#ifndef DEFAULT_OBJ_NAME
  #define DEFAULT_OBJ_NAME "BACnet4MbedOS_Demo"
#endif

#ifndef DEFAULT_DHCP_SETTING
	#define DEFAULT_DHCP_SETTING 1
#endif


/*--------*/
/* Makros */
/*--------*/
#ifndef LENGTH
	#define LENGTH(a) (sizeof(a)/sizeof(a[0]))
#endif


/*-----------------*/
/* Typedefinitions */
/*-----------------*/
typedef volatile bool vbool;


/*------------------*/
/* Global Variables */
/*------------------*/
const char *msvdescr_UsrBttnState_Texts[] = 
{ 
  "UNPRESSED",
  "PRESSED"
};

//
// Ticker
Ticker identTick;

//
// BACnet Object Descriptions
DEVICE_OBJECT_DESCR Device_Descr = 
{
	.object_instance = DEFAULT_OBJ_INSTANCE,
	.object_name     = DEFAULT_OBJ_NAME,
	.dhcp 					 = DEFAULT_DHCP_SETTING,
	.read_callback   = NULL,
	.write_callback  = device_object_write_callback,
}; /* DEVICE_OBJECT_DESCR */


BINARY_INPUT_DESCR BI_Descr[] =
{
  // User Button Binary Input
  {
		.Object_Instance    = bidescr_usrBttn,
		.Object_Name        = (char*)"BI_UsrBttn",
    .Object_Description = (char*)"Switch enabling LED2",
		.Polarity           = POLARITY_NORMAL,
	},
  
  { BACNET_INSTANCE_DELIMITER }
}; /* BINARY_INPUT_DESCR */


BINARY_OUTPUT_DESCR BO_Descr[] =
{
  // LED3 Binary Output
  {
		.Object_Instance    = bodescr_led3,
		.Object_Name        = (char*)"BO_LED3",
    .Object_Description = (char*)"Object connected to LED3",
		.Polarity           = POLARITY_NORMAL,
		.write_callback     = bo_setled3,
	},
  
	{ BACNET_INSTANCE_DELIMITER }
}; /* BINARY_OUTPUT_DESCR */


BINARY_VALUE_DESCR BV_Descr[] = 
{
  { BACNET_INSTANCE_DELIMITER }
}; /* BINARY_VALUE_DESCR */


ANALOG_INPUT_DESCR AI_Descr[] = 
{
	{ BACNET_INSTANCE_DELIMITER }
}; /* ANALOG_INPUT_DESCR */


ANALOG_OUTPUT_DESCR AO_Descr[] = 
{
	{ BACNET_INSTANCE_DELIMITER }
}; /* ANALOG_OUTPUT_DESCR */


ANALOG_VALUE_DESCR AV_Descr[] = 
{
  // Internal Temperature Sensor
  {
    .Object_Instance    = avdescr_intTempSensor,
    .Object_Name        = (char*)"AV_JunctionTemperature",
    .Object_Description = (char*)"[READONLY] Internal Temperature Sensor of the microController device. "
                                 "Resolution around [1.5\u2103].",
    .Units              = UNITS_DEGREES_CELSIUS,
    .COV_Increment      = 0.10,
    .PV_WriteProtected  = true,
  },
  
  // UpTime since boot
  {
    .Object_Instance    = avdescr_upTime,
    .Object_Name        = (char*)"AV_System_UpTime",
    .Object_Description = (char*)"[READONLY] Rough estimation on system uptime since last boot.",
    .Units              = UNITS_SECONDS,
    .COV_Increment      = 0.50,
    .PV_WriteProtected  = true,
  },
  
  // UpTime since boot
  {
    .Object_Instance    = avdescr_Counter,
    .Object_Name        = (char*)"AV_Counter_1s",
    .Object_Description = (char*)"[READONLY] Keepalive Counter, counting 1s.",
    .Units              = UNITS_NO_UNITS,
    .COV_Increment      = 0.50,
    .PV_WriteProtected  = true,
  },
  
	{ BACNET_INSTANCE_DELIMITER }
}; /* ANALOG_VALUE_DESCR */


MULTISTATE_VALUE_DESCR MSV_Descr[] = 
{
  // User Button MSV
  {
    .Object_Instance    = msvdescr_usrBttn,
    .Object_Name        = (char*)"MSV_usrBttn_State",
    .Object_Description = (char*)"[READONLY] State representation of UserButton.",
    .Number_Of_States   = LENGTH(msvdescr_UsrBttnState_Texts),
    .State_Texts        = msvdescr_UsrBttnState_Texts,
    .PV_WriteProtected  = true,
    .read_callback      = NULL,
    .write_callback     = NULL,
  },

  { BACNET_INSTANCE_DELIMITER }
}; /* MULTISTATE_VALUE_DESCR */



/*---------------------*/
/* Functiondefinitions */
/*---------------------*/

// Pruefung der verwendeten Descriptions auf Vollstaendigkeit
void validateDescrObjects()
{
    bool validDescr = true;
    bool bi  = true, bo = true, bv = true,
         ai  = true, ao = true, av = true,
         msv = true;
    
    // BACnet Binary Input Descriptions
    if (LENGTH(BI_Descr) != bidescr_DELIMITER)
    { validDescr = false; bi = false; }
            
    // BACnet Binary Output Descriptions
    if (LENGTH(BO_Descr) != bodescr_DELIMITER)
    { validDescr = false; bo = false; }
        
    // BACnet Binary Value Descriptions
    if (LENGTH(BV_Descr) != bvdescr_DELIMITER)
    { validDescr = false; bv = false; }
            
    // BACnet Analog Input Descriptions
    if (LENGTH(AI_Descr) != aidescr_DELIMITER)
    { validDescr = false; ai = false; }
        
    // BACnet Analog Output Descriptions
    if (LENGTH(AO_Descr) != aodescr_DELIMITER)
    { validDescr = false; ao = false; }
            
    // BACnet Analog Value Descriptions
    if (LENGTH(AV_Descr) != avdescr_DELIMITER)
    { validDescr = false; av = false; }
            
    // BACnet MultiState Value Descriptions
    if (LENGTH(MSV_Descr) != msvdescr_DELIMITER)
    { validDescr = false; msv = false; }
    
    
    if(!validDescr)
    {
        printf("--INVALID Descriptor Legth(s)---\r\n");
        
        if(!bi)  { printf("  BI_Descr[] - Invalid length\r\n"); }
        if(!bo)  { printf("  BO_Descr[] - Invalid length\r\n"); }
        if(!bv)  { printf("  BV_Descr[] - Invalid length\r\n"); }
        
        if(!ai)  { printf("  AI_Descr[] - Invalid length\r\n"); }
        if(!ao)  { printf("  AO_Descr[] - Invalid length\r\n"); }        
        if(!av)  { printf("  AV_Descr[] - Invalid length\r\n"); }
        
        if(!msv) { printf("  MSV_Descr[] - Invalid length\r\n"); }
        
        error("--INVALID BACnet-Object-Descriptor Legth(s)---");
    }
}


// Ermittlung eines MSV_States [0-...] in Abhaengigkeit eines ControlValues
// Hier: EventState fuer 'msvdescr_usrBttn' in Abheangigkeit eines
//       ControlModes nach 'MP_CONTROL_MODE' (siehe: MP_Bus.h)
uint8_t get_UsrBttnState(bool state)
{
  // BACnet MSV_States start at index 1 (not 0)
  
  // See 'msvdescr_UsrBttnState_Texts'
  // [0] -> 1 - UNPRESSED
  // [1] -> 2 - PRESSED
  
  return ((uint8_t)state) + 1;
}
