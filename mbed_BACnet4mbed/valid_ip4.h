#ifndef _VALID_IP_V4_H_
#define _VALID_IP_V4_H_

/*----------*/
/* Includes */
/*----------*/

/*------------*/
/* Prototypes */
/*------------*/
bool valid_ip4(char *ip);
bool valid_ip4(uint32_t ip);
bool valid_ip4(ip4_addr_t ip_addr);

#endif /* _VALID_IP_V4_H_ */
