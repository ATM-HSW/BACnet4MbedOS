/*----------*/
/* Includes */
/*----------*/
#include "mbed.h"
#include "ip4_addr.h"
#include <string.h>


/*---------*/
/* Defines */
/*---------*/
#define IP_LEN 16


/*---------------------*/
/* Functiondefinitions */
/*---------------------*/
bool valid_ip4(char *ip)
{
  uint16_t num;
  uint8_t  flag = 1;
  uint8_t  counter = 0;
  char *rst = ip;
  char *ptr = strtok_r(ip, ".", &rst);
  
  while (ptr && flag )
  {
    num = atoi(ptr);
    
    // Unsigned int renders comparrison with 0 futile
    // if (num >= 0 && num <= 255 && (counter++ < 4))
    if (num <= 255 && (counter++ < 4))
    {
      flag = 1;
      ptr = strtok_r(NULL, ".", &rst);
    }
    else
    {
      flag = 0;
      break;
    }
  }
  
  return (flag == 1) && (counter == 4);
}


bool valid_ip4(uint32_t ip)
{
  ip4_addr_t ip_addr;
  char ip_str[IP_LEN];
  
  ip_addr.addr = ip;
  ip4addr_ntoa_r(&ip_addr, ip_str, IP_LEN);
  
  return valid_ip4(ip_str);
}


bool valid_ip4(ip4_addr_t ip_addr)
{
  char ip_str[IP_LEN];
  
  ip4addr_ntoa_r(&ip_addr, ip_str, IP_LEN);
  
  return valid_ip4(ip_str);
}
