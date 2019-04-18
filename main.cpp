/*----------*/
/* Includes */
/*----------*/
// mbedOS
#include "mbed.h"
#include "EthernetInterface.h"
#include "ObjectDescriptors.h"

// Random number generator
#include "mbed_RNG.h"

/*---------*/
/* Defines */
/*---------*/
#define ETH_IP "192.168.1.100"
#define ETH_NETMASK "255.255.255.0"
#define ETH_GW "192.168.1.1"
#define DHCP true

// ADC / Temperature Measurement
//  see ST - DocID027590 Rev 4
#define Vdd_mV (3300)   // Vdd Voltage of Controller
#define V25 (0.76)      // [V] Ref.Voltage @25°C
#define Avg_Slope (2.5) // [mV/°C] Average Slope of internal Temp. Sensor

#define mV25 (V25 * 1000) // V -> mV

/*--------*/
/* Macros */
/*--------*/
#define NONE_NULL_STR(X) ((X == NULL) ? "\0" : X)

/*-----------------*/
/* Typedefinitions */
/*-----------------*/
typedef volatile bool vbool;
typedef void (*cbFun)(void);

typedef struct            // EventQueue Callback LuT
{                         //  Beschreibung einzelner durch EventQueue
  uint32_t callIntervall; //	aufzurufender Callbackfunktionen und deren
  cbFun cbFunction;       //	zugehoeriger Parameter (siehe (*cbFun)
  uint32_t evId;          //	callInter. - Aufruf-Intervall in [ms]
  char *name;             //	cbFunction - Pointer auf Callback
} EVQ_CB_LUT;             //  evId       - EventId des die Callback rufenden Events (>0)
                          //  name       - Beschreibungstext / Name

/*------------------*/
/* Global Variables */
/*------------------*/
//
// Threads
Thread bacnetThread(osPriorityAboveNormal, 8000, NULL, "BACnet_Thread");
Thread genPurposeThread(osPriorityNormal, 4000, NULL, "genPurpose_Thread");

//
// EventQueues
EventQueue genPurposeQueue(EVENTS_QUEUE_SIZE); // e.g. Sensor-polling

//
// Network / Bus Interfaces
EthernetInterface net;
bool dhcp = DHCP;

//
// STM32F7 RandomNumberGenerator
static mbed_RNG *mbedRNG = mbed_RNG::Instance(); // STM32 internal ADC circuit as RNG

//
// Internal Temperature Sensor
AnalogIn junctionTemp(ADC_TEMP); // AI on internal ADC_Temp

//
// Button(s)
InterruptIn usrBttn(USER_BUTTON); // Blue button on Nucleo

//
// Status-LED(s)
DigitalOut led1(LED1, 0); // Activity LED
DigitalOut led2(LED2, 0); // General-Purpose LED
DigitalOut led3(LED3, 1); // Startup LED

//
// Boot Information
bool systemUp = false;
time_t timeBoot;

/*--------------------*/
/* External Variables */
/*--------------------*/
// BACnet Device Description
extern DEVICE_OBJECT_DESCR Device_Descr;

/*------------*/
/* Prototypes */
/*------------*/
void counter_keepAlive(void);
void read_jTemp(void);
void get_upTime(void);
void usrBttn_action(void);
void flip_led1(void);

/*---------------------------------------------------------------------------*/

//
// main()
//  runs in its own thread in the OS
int main()
{
  //
  // State Variables
  bool initOK = true;
  bool ethIF = false;
  bool bacInit = false;

  //
  // main 'local' Zustandsvariablen
  // Ethernet / LAN
  char *ip; // Ethernet LAN IP-Address after eth.connect()

  //
  // Activity Ticker
  Ticker actTick;

  //
  // Sensor & General Info EventQueue LuT
  EVQ_CB_LUT sensorEvqCbLut[] = {
      // Generall BACnet KeepAlive Ticker
      {
          .callIntervall = 1000,
          .cbFunction = counter_keepAlive,
          .evId = 0,
          .name = (char *)"av_counter_ticker",
      },

      // Internal temperature Sensor reading
      {
          .callIntervall = 250,
          .cbFunction = read_jTemp,
          .evId = 0,
          .name = (char *)"read_jTemp",
      },

      // Uptime counter
      {
          .callIntervall = 500,
          .cbFunction = get_upTime,
          .evId = 0,
          .name = (char *)"uptimeCounter",
      },
  };

  printf("----------------------\r\n"
         "- BACnet4MbedOS DEMO -\r\n"
         "----------------------\r\n");

  printf("Booting...\r\n");

  /* EventQueue Binding */
  {
    //
    // Dispatch EventQueue at specific Thread
    osStatus genStat = genPurposeThread.start(callback(&genPurposeQueue, &EventQueue::dispatch_forever));
  }

  /* Ethernet Interface */
  {
    printf("-Ethernet Interface-\r\n");
    printf("  Connecting...\r\n");

    //
    // Configure EthernetInterface
    printf("  --Ethernet Configuration--\r\n");
    printf("  IP:      %-15s\r\n", ETH_IP);
    printf("  Netmask: %-15s\r\n", ETH_NETMASK);
    printf("  Gateway: %-15s\r\n", ETH_GW);
    printf("  DHCP:    %-15s\r\n", dhcp ? "DHCP" : "static");

    // Ethernet Interface Parametrierung
    led2 = 1;
    net.disconnect();
    net.set_network(ETH_IP, ETH_NETMASK, ETH_GW);
    net.set_dhcp(dhcp);

    //
    // Connect to EthernetInterface
    nsapi_error_t status = net.connect();
    ethIF = (status == NSAPI_ERROR_OK);

    if (ethIF)
    {
      ip = (char *)net.get_ip_address();

      if (ip && strlen(ip) >= 7 && strlen(ip) <= 15) // IPv4 x.x.x.x
      {
        const char *mac = net.get_mac_address();

        printf("  Ethernet connected...\r\n");
        printf("    Connected - IP:  %-s\r\n", ip);
        printf("    Connected - MAC: %-s\r\n", mac);
        printf("    Netmask:         %-s\r\n", NONE_NULL_STR(net.get_netmask()));
        printf("    Gateway:         %-s\r\n", NONE_NULL_STR(net.get_gateway()));

        led2 = 0;
      }
      else
      {
        ethIF = false;
      }
    }
    else // Reconnect using defaults
    {
      printf("  Reconnecting using defaults (DHCP off)...\r\n");

      dhcp = false;

      net.disconnect();
      net.set_dhcp(dhcp);

      nsapi_error_t status = net.connect();
      ethIF = (status == NSAPI_ERROR_OK);
    }

    if (!ethIF)
    {
      error("FAILED_TO_CONNET_TO_ETH\r\n");
    }
  }

  /* BACnet Stack / DescrObject Validation */
  {
    printf("-BACnet Stack / DescrObject Validation-\r\n");

    if (ethIF && ip)
    {
      //
      // BACnet Stack initialisieren
      validateDescrObjects();

      char *netIp = (char *)net.get_ip_address();
      char *netNetmask = (char *)net.get_netmask();
      char *netGateway = (char *)net.get_gateway();

      Device_Set_IP_Address(netIp);           // push IP into Device_decr Object
      Device_Set_Netmask_Address(netNetmask); // push Netmask into Device_descr Object
      Device_Set_Gateway_Address(netGateway); // push Gateway into Device_descr Object
      Device_Set_DHCP_Setting(dhcp);          // push DHCP Setting into Device_descr Object

      printf("  Generating new BACnet ObjInstance via RNG...\r\n");

      uint32_t objInst = mbedRNG->rand() % (BACNET_MAX_INSTANCE + 1);
      objInst &= 0x3FFFFF; // Maxmimum BACnet Instance Number

      printf("  BACnet ObjInstance: %u\r\n", objInst);

      Device_Set_Object_Instance_Number(objInst);

      printf("  |-----BACnet Stack Init Values-----|\r\n");
      printf("  |  IP:          %-18s |\r\n", netIp);
      printf("  |  Netmask:     %-18s |\r\n", netNetmask);
      printf("  |  Gateway:     %-18s |\r\n", netGateway);
      printf("  |  DHCP:        %-18s |\r\n", (dhcp) ? "DHCP" : "static");
      printf("  |----------------------------------|\r\n");
      printf("  |  DevName:     %-18s |\r\n", Device_Descr.object_name);
      printf("  |  DevInstance: %07i            |\r\n", Device_Descr.object_instance);
      printf("  |----------------------------------|\r\n");

      // Init BACnet Stack on 'bacnetThread'
      bacnet_init(ip, &bacnetThread);

      // [BI] UserButton aktualisieren
      BACNET_BINARY_PV state = usrBttn.read() ? BINARY_ACTIVE : BINARY_INACTIVE;
      Binary_Input_Present_Value_Set(bidescr_usrBttn, state);

      // [AV] Internal Temperature Sensor (Junction-Temperature)
      read_jTemp(); // Implicitly updates AV_Present_Value of 'avdescr_jTemp'

      // [MSV] MultistateValues initialisieren
      uint32_t msv_state = get_UsrBttnState((bool)usrBttn.read());
      Multistate_Value_Present_Value_Set(msvdescr_usrBttn, state, BACNET_MAX_PRIORITY);

      bacInit = true;
    }
    else
    {
      initOK = false;
    }

    printf("  BACnet Init: %s\r\n", (bacInit) ? "OK" : "FAILED");
  }

  /* Sensor EventQueue LUT */
  {
    printf("-Sensor EventQueue LUT Init-\r\n");

    // Publish temperature once (initValue)
    printf("  Internal Temp: %03.1f degC\r\n", Analog_Value_Present_Value(avdescr_intTempSensor));
    printf("  System UpTime: %f%s\r\n", Analog_Value_Present_Value(avdescr_upTime),
           (Analog_Value_Units(avdescr_upTime) == UNITS_SECONDS) ? "s" : "h");

    // Process Sensor-CB LuT
    for (uint8_t i = 0; i < LENGTH(sensorEvqCbLut); i++)
    {
      sensorEvqCbLut[i].evId = genPurposeQueue.call_every(sensorEvqCbLut[i].callIntervall, sensorEvqCbLut[i].cbFunction);

      if (!sensorEvqCbLut[i].evId)
      {
        initOK = false;
        printf("  Sensor EvQ failed: %s\r\n", sensorEvqCbLut[i].name);
        error("SENS_EVQ_LUT_FAILED\r\n");
      }
    }
  }

  /* Configure Button Callback */
  {
    usrBttn.fall(&usrBttn_action);
    usrBttn.rise(&usrBttn_action);
  }

  /* Init-Messages */
  {
    if (initOK)
    {
      led3 = 0;

      printf("\r\n"
             "|-------------------------------------------|\r\n"
             "|        --- System starting up ---         |\r\n"
             "|-BACnet------------------------------------|\r\n"
             "|  BACnet Name:  %-25s  |\r\n"
             "|  BACnet ID:    %07i                    |\r\n"
             "|-Ethernet----------------------------------|\r\n"
             "|  MAC:          %-25s  |\r\n"
             "|  IP:           %-25s  |\r\n"
             "|  Netmask:      %-25s  |\r\n"
             "|  Gateway:      %-25s  |\r\n"
             "|  DHCP:         %-25s  |\r\n",
             Device_Descr.object_name,
             Device_Descr.object_instance,
             NONE_NULL_STR(net.get_mac_address()),
             NONE_NULL_STR(net.get_ip_address()),
             NONE_NULL_STR(net.get_netmask()),
             NONE_NULL_STR(net.get_gateway()),
             (dhcp ? "DHCP" : "static"));

      // End of Init Message
      printf("|-------------------------------------------|\r\n");
      printf("\r\n");
    }
    else
    {
      led1 = 0;
      led2 = 1;
      led3 = 1;
      error("INIT_FAILED\r\n");
    }
  }

  /*--------------------------------------------------------------------*/

  //
  // Activity Ticker
  if (initOK)
  {
    actTick.attach_us(&flip_led1, 500000);
  }

  //
  // Declare that 'system' has booted up...
  systemUp = true;

  /* Controller-Loop */
  while (true)
  {
#if MBED_CONF_RTOS_PRESENT
#if MBED_VERSION >= MBED_ENCODE_VERSION(5, 10, 0)
		ThisThread::yield();
#else
		Thread::yield();
#endif
#endif /* MBED_CONF_RTOS_PRESENT */
  }
}

/*--------------------------------------------------------------------*/

/*---------------------*/
/* Functiondefinitions */
/*---------------------*/

void counter_keepAlive(void)
{
  static uint32_t cnt = 0;
  Analog_Value_Present_Value_Set(avdescr_Counter, cnt, BACNET_MAX_PRIORITY);
  cnt = (cnt < 0xFFFFFFFF) ? cnt + 1 : 0;
}

void flip_led1(void) { led1 = !led1; }

void read_jTemp(void)
{
  float jTemp = junctionTemp.read(); // AnalogValue in 0.01% of VDD (AVDD)
  jTemp *= Vdd_mV;                   // For Vdd=3V3 scale jTemp

  // Calculate junctionTemperature (floored to .1 decimal)
  jTemp = ((jTemp - (float)mV25) / (float)Avg_Slope) + 25;
  jTemp = floor((jTemp * 10)) / 10;

  // Publish as BACnet AnalogValue [AV]
  Analog_Value_Present_Value_Set(avdescr_intTempSensor, jTemp, BACNET_MAX_PRIORITY);
}

void get_upTime(void)
{
  if (systemUp)
  {
    // For a good explanation on float-precision over time (25.08.2018) see:
    // https://randomascii.wordpress.com/2012/02/13/dont-store-that-in-a-float/
    double upTime = difftime(time(NULL), timeBoot);
    uint16_t upTimeScale = (upTime >= 36000) ? 3600 : 1;

    if (upTimeScale >= 3600)
    {
      Analog_Value_Units_Set(avdescr_upTime, UNITS_HOURS);
      Analog_Value_COV_Increment_Set(avdescr_upTime, 0.01);
    }
    else
    {
      Analog_Value_Units_Set(avdescr_upTime, UNITS_SECONDS);
      Analog_Value_COV_Increment_Set(avdescr_upTime, 1.00);
    }

    upTime /= upTimeScale;
    Analog_Value_Present_Value_Set(avdescr_upTime, (float)upTime, BACNET_MAX_PRIORITY);
  }

  return;
}

void usrBttn_action(void)
{
  bool state = usrBttn.read();
  led2 = state;
  Multistate_Value_Present_Value_Set(msvdescr_usrBttn, get_UsrBttnState(state), BACNET_MAX_PRIORITY);
  Binary_Input_Present_Value_Set(bidescr_usrBttn, state ? BINARY_ACTIVE : BINARY_INACTIVE);
}
