#ifndef _OBJECTDESCRIPTORS_ENUM_H_
#define _OBJECTDESCRIPTORS_ENUM_H_

/*-----------------*/
/* Typedefinitions */
/*-----------------*/

// !!! Sämtliche hier aufgelisteten Werte muessen auch verwendet werden  !!!
// !!! Jede Enumeration muss einer diskreten Description zugeordnet sein !!!
// !!! Ausnahmen: xxx_EOT & xxx_DELIMITER
//     (EOT & DELIMITER muessen jedoch in jeder Enumeration vorhanden sein, 
//      und am Ende der enum stehen)

typedef enum /* BI_DESCRIPTORS */
{
    bidescr_usrBttn = 0,

    bidescr_EOT,
    bidescr_DELIMITER,
} BI_DESCRIPTORS;

typedef enum /* BO_DESCRIPTORS */
{
    bodescr_led3,

    bodescr_EOT,
    bodescr_DELIMITER,
} BO_DESCRIPTORS;

typedef enum /* BV_DESCRIPTORS */
{
    bvdescr_EOT,
    bvdescr_DELIMITER,
} BV_DESCRIPTORS;

typedef enum /* AI_DESCRIPTORS */
{
    aidescr_EOT,
    aidescr_DELIMITER,
} AI_DESCRIPTORS;

typedef enum /* AO_DESCRIPTORS */
{
    aodescr_EOT,
    aodescr_DELIMITER,
} AO_DESCRIPTORS;

typedef enum /* AV_DESCRIPTORS */
{
    avdescr_intTempSensor = 0,
    avdescr_upTime,
    avdescr_Counter,

    avdescr_EOT,
    avdescr_DELIMITER,
} AV_DESCRIPTORS;

typedef enum /* MSV_DESCRIPTORS */
{
    msvdescr_usrBttn = 0,
    
    msvdescr_EOT,
    msvdescr_DELIMITER,
} MSV_DESCRIPTORS;

#endif
