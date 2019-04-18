/*----------*/
/* Includes */
/*----------*/
#include "bacnet.h"
#include "io_functions.h"

/*--------------------*/
/* External Variables */
/*--------------------*/
extern DigitalOut led3;

/*---------------------*/
/* Functiondefinitions */
/*---------------------*/
void device_object_write_callback(uint32_t prop)
{
  // Example callback
  // No actions taken, since changes to PresentValues are already handled by the
  // corresponding writeProperty handler of the DeviceObject.
  // Additional actions might be taken by this callback.
  // E.g. Permanent Property storage on EEPROM / Flash

  switch ((BACNET_PROPERTY_ID)prop)
  {
  case PROP_OBJECT_IDENTIFIER:
    break;

  case PROP_OBJECT_NAME:
    break;

  case PROP_GATEWAY_IP:
    break;

  case PROP_GATEWAY_NMASK:
    break;

  case PROP_GATEWAY_GW:
    break;

  case PROP_GATEWAY_DHCP:
    break;

  default:
    break;
  }

  return;
}

void bo_setled3(uint32_t propID, bool value)
{
  switch ((BACNET_PROPERTY_ID)propID)
  {
  case PROP_PRESENT_VALUE:
    led3 = value;
    break;

  default:
    break;
  }

  return;
}
