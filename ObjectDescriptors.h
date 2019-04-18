#ifndef _OBJECTDESCRIPTORS_H_
#define _OBJECTDESCRIPTORS_H_

/*----------*/
/* Includes */
/*----------*/
#include "mbed.h"

#include "bacnet.h"
#include "ObjectDescriptors_enum.h"
#include "io_Functions.h"


/*---------------------*/
/* Function Prototypes */
/*---------------------*/
void validateDescrObjects(void);
void device_object_write_callback(uint32_t prop);

uint8_t get_UsrBttnState(bool state);

#endif
