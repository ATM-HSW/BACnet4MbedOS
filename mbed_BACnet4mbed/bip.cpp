/*####COPYRIGHTBEGIN####
 -------------------------------------------
 Copyright (C) 2005 Steve Karg

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to:
 The Free Software Foundation, Inc.
 59 Temple Place - Suite 330
 Boston, MA  02111-1307, USA.

 As a special exception, if other files instantiate templates or
 use macros or inline functions from this file, or you compile
 this file and link it with other works to produce a work based
 on this file, this file does not by itself cause the resulting
 work to be covered by the GNU General Public License. However
 the source code for this file must still be made available in
 accordance with section (3) of the GNU General Public License.

 This exception does not invalidate any other reasons why a work
 based on this file might be covered by the GNU General Public
 License.
 -------------------------------------------
####COPYRIGHTEND####*/

#include "mbed.h"
#include "EthernetInterface.h"

#include <stdint.h>     /* for standard integer types uint8_t etc. */
#include <stdbool.h>    /* for the standard bool type. */
#include "bacdcode.h"
#include "bacint.h"
#include "bip.h"
#include "bvlc.h"

#include "nsapi_types.h"

#include "EvRec_BACnet4mbed.h"

#if PRINT_ENABLED
#include <stdio.h>      /* for standard i/o, like printing */
#include "debug_msg.h"
#endif


/** @file bip.cpp  Configuration and Operations for BACnet/IP */

extern EthernetInterface net;

//static int BIP_Socket = -1;
static UDPSocket BIP_Socket;
/* port to use - stored in network byte order */
static uint16_t BIP_Port = 0xBAC0;   /* this will force initialization in demos */
/* IP Address - stored in network byte order */
static struct in_addr BIP_Address;
/* Broadcast Address - stored in network byte order */
static struct in_addr BIP_Broadcast_Address;

/* ifname is the dotted ip address of the interface */
bool bip_init(char *ifname)
{
	ip4_addr_t ip;
	bool status = ((bool) ip4addr_aton(ifname, &ip));
	
	if(status)
	{ EVRECORDDATA(BACNET_INIT_ADDR_CONVERSION_OK, ifname, STRSIZE(ifname)); }
	else
	{ EVRECORDDATA(BACNET_INIT_ADDR_CONVERSION_FAIL, ifname, STRSIZE(ifname)); }
	
	bip_set_addr(ip.addr);
	bip_set_broadcast_addr(ip.addr | 0xff000000);
	
  BIP_Socket.set_timeout(0);
	BIP_Socket.set_blocking(false);
	
	nsapi_error_t err = BIP_Socket.open(&net);
	if(err == NSAPI_ERROR_OK)
	{ EVRECORD2(BACNET_UDP_SOCK_OPENED, err, 0); }
	else
	{ EVRECORD2(BACNET_UDP_SOCK_FAILED, err, 0); }
	
	err = BIP_Socket.bind(BIP_Port);
	if(err == NSAPI_ERROR_OK)
	{ EVRECORD2(BACNET_UDP_SOCK_BOUND, err, 0); }
	else
	{ EVRECORD2(BACNET_UDP_SOCK_BIND_FAIL, err, 0); }
	
	return (status && (err == NSAPI_ERROR_OK));
}

/* Reinitialize bip */
bool bip_reinit(char *ifname)
{
	EVRECORD2(BACNET_BIP_REINIT, 0, 0);
		
	nsapi_error_t retCode = BIP_Socket.close();
	bool status = bip_init(ifname);
	
	if(status && (retCode >= NSAPI_ERROR_OK))
	{ EVRECORD2(BACNET_BIP_REINIT_OK, 0, 0); }
	else
	{ EVRECORD2(BACNET_BIP_REINIT_FAILED, retCode, status); }
	
	return status;
}

/* lose bip_UDP_socket */
bool bip_close_sock(void)
{
	return (BIP_Socket.close() == NSAPI_ERROR_OK);
}

/** Setter for the BACnet/IP socket handle.
 *
 * @param sock_fd [in] Handle for the BACnet/IP socket.
 */
void bip_set_socket(
    int sock_fd)
{
//    BIP_Socket = sock_fd;
}

/** Getter for the BACnet/IP socket handle.
 *
 * @return The handle to the BACnet/IP socket.
 */
int bip_socket(
    void)
{
    return 0;//&BIP_Socket;
}

bool bip_valid(
    void)
{
//    return (BIP_Socket != -1);
	return true;
}

void bip_set_addr(
    uint32_t net_address)
{       /* in network byte order */
    BIP_Address.s_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_addr(
    void)
{
    return BIP_Address.s_addr;
}

void bip_set_broadcast_addr(
    uint32_t net_address)
{       /* in network byte order */
    BIP_Broadcast_Address.s_addr = net_address;
}

/* returns network byte order */
uint32_t bip_get_broadcast_addr(
    void)
{
    return BIP_Broadcast_Address.s_addr;
}


void bip_set_port(
    uint16_t port)
{       /* in network byte order */
    BIP_Port = port;
}

/* returns network byte order */
uint16_t bip_get_port(
    void)
{
    return BIP_Port;
}

static int bip_decode_bip_address(
    BACNET_ADDRESS * bac_addr,
    ip4_addr_t *address,    /* in network format */
    uint16_t * port)
{       /* in network format */
    int len = 0;

    if (bac_addr) {
        memcpy(&address->addr, &bac_addr->mac[0], 4);
        memcpy(port, &bac_addr->mac[4], 2);
        len = 6;
    }
		address->addr = lwip_htonl(address->addr);
    return len;
}

static void bip_mac_to_addr(
    ip4_addr_t *address,
    uint8_t * mac)
{
    if (mac && address) {
        address->addr = ((u32_t) ((((uint32_t) mac[0]) << 24) & 0xff000000));
        address->addr |= ((u32_t) ((((uint32_t) mac[1]) << 16) & 0x00ff0000));
        address->addr |= ((u32_t) ((((uint32_t) mac[2]) << 8) & 0x0000ff00));
        address->addr |= ((u32_t) (((uint32_t) mac[3]) & 0x000000ff));
    }
}

static void bip_addr_to_mac(
    uint8_t * mac,
    ip4_addr_t *address)
{
    if (mac && address) {
        mac[0] = (uint8_t) (address->addr >> 24);
        mac[1] = (uint8_t) (address->addr >> 16);
        mac[2] = (uint8_t) (address->addr >> 8);
        mac[3] = (uint8_t) (address->addr);
    }
}

/** Function to send a packet out the BACnet/IP socket (Annex J).
 * @ingroup DLBIP
 *
 * @param dest [in] Destination address (may encode an IP address and port #).
 * @param npdu_data [in] The NPDU header (Network) information (not used).
 * @param pdu [in] Buffer of data to be sent - may be null (why?).
 * @param pdu_len [in] Number of bytes in the pdu buffer.
 * @return Number of bytes sent on success, negative number on failure.
 */
int bip_send_pdu(
    BACNET_ADDRESS 		*dest,			/* destination address */
    BACNET_NPDU_DATA 	*npdu_data, /* network information */
    uint8_t 					*pdu,      	/* any data to be sent - may be null */
    unsigned 					 pdu_len)		/* number of bytes of data */
{       
    uint8_t mtu[MAX_MPDU] = { 0 };
    nsapi_size_t mtu_len = 0;
    int bytes_sent = 0;
    
    char ip[16];            /* ipv4 in string format "0.0.0.0" */
    uint16_t port = 0;      /* UDP destination port */
    ip4_addr_t address;			/* addr and port in host format */

    (void) npdu_data;
		EVRECORD2(BACNET_SENDING_PDU, 0, 0);
		
    mtu[0] = BVLL_TYPE_BACNET_IP;
    if ((dest->net == BACNET_BROADCAST_NETWORK) || (dest->mac_len == 0))
		{
      /* broadcast */
      address.addr = BIP_Broadcast_Address.s_addr;
      port = BIP_Port;
      mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
			
      #if RTX_USER_EVENT_RECORDS
        char str[16];
        ip4addr_ntoa_r(&address, str, 16);
      #endif /* RTX_USER_EVENT_RECORDS */
      
			EVRECORDDATA(BACNET_SENDING_PDU_BCAST, str, STRSIZE(str));
    }
		else if ((dest->net > 0) && (dest->len == 0))
		{
      /* network specific broadcast */
      if (dest->mac_len == 6) {
        bip_decode_bip_address(dest, &address, &port);
      }
			else
			{
        address.addr = BIP_Broadcast_Address.s_addr;
        port = BIP_Port;
      }
      
      mtu[1] = BVLC_ORIGINAL_BROADCAST_NPDU;
			
      #if RTX_USER_EVENT_RECORDS
        char str[16];
        ip4addr_ntoa_r(&address, str, 16);
        EVRECORDDATA(BACNET_SENDING_PDU_NW_BCAST, str, STRSIZE(str));
      #endif /* RTX_USER_EVENT_RECORDS */
    }
		else if (dest->mac_len == 6)
		{
       bip_decode_bip_address(dest, &address, &port);
       mtu[1] = BVLC_ORIGINAL_UNICAST_NPDU;
			
      #if RTX_USER_EVENT_RECORDS
        char str[16];
        ip4addr_ntoa_r(&address, str, 16);
        EVRECORDDATA(BACNET_SENDING_PDU_UCAST, str, STRSIZE(str));
      #endif /* RTX_USER_EVENT_RECORDS */
    }
		else
		{
      /* invalid address */
			EVRECORD2(BACNET_SEND_PDU_INVALID_ADDR, 0, 0);
      return -1;
    }
    
    mtu_len  = 2;
    mtu_len += encode_unsigned16(&mtu[mtu_len], (uint16_t) (pdu_len + 4 /*inclusive */ ));
    memcpy(&mtu[mtu_len], pdu, pdu_len);
    mtu_len += pdu_len;
    
    /* Format IPv4 as string "0.0.0.0" */
		ip4addr_ntoa_r(&address, ip, 16);
		
    /* Send the packet */
		bytes_sent = BIP_Socket.sendto((const char*)ip, port, (void*) mtu, mtu_len);
		
		if(bytes_sent < NSAPI_ERROR_OK)
		{ EVRECORD2(BACNET_SENDING_PDU_FAILED, bytes_sent, 0); }
		else
		{ EVRECORD2(BACNET_SENT_PDU, bytes_sent, 0); }

    return bytes_sent;
}

/** Implementation of the receive() function for BACnet/IP; receives one
 * packet, verifies its BVLC header, and removes the BVLC header from
 * the PDU data before returning.
 *
 * @param src [out] Source of the packet - who should receive any response.
 * @param pdu [out] A buffer to hold the PDU portion of the received packet,
 * 					after the BVLC portion has been stripped off.
 * @param max_pdu [in] Size of the pdu[] buffer.
 * @param timeout [in] The number of milliseconds to wait for a packet.
 * @return The number of octets (remaining) in the PDU, or zero on failure.
 */
uint16_t bip_receive(
    BACNET_ADDRESS * src,       /* source address */
    uint8_t * pdu,      				/* PDU data */
    uint16_t max_pdu,   				/* amount of space available in the PDU  */
    unsigned timeout)
{
    uint16_t pdu_len = 0;       /* return value */
    uint16_t pdu_offset = 0;
	  uint16_t port = 0;
    uint8_t function = 0;
    uint16_t i = 0;
		SocketAddress sin_addr;
		ip4_addr_t addr;
	
		if(BIP_Socket.recvfrom(&sin_addr, (void*) pdu, max_pdu) <= 0)
		{
			return 0;
		}
	
    /* the signature of a BACnet/IP packet */
    if (pdu[0] != BVLL_TYPE_BACNET_IP) 
		{
			EVRECORD2(BACNET_BIP_RECEIVING_NON_BAC, 0, 0);
      return 0;
    }
		
		ip4addr_aton(sin_addr.get_ip_address(), &addr);
		port = sin_addr.get_port();
		
    function = pdu[1];
		
		char str[16];
		strcpy(str, sin_addr.get_ip_address());
		EVRECORDDATA(BACNET_BIP_RECEIVING, str, STRSIZE(str));
		
		if ((function == BVLC_ORIGINAL_UNICAST_NPDU) ||
        (function == BVLC_ORIGINAL_BROADCAST_NPDU))
		{
        /* ignore messages from me */
        if ((addr.addr == BIP_Address.s_addr) && (port == BIP_Port))
				{ 
					pdu_len = 0;
					EVRECORD2(BACNET_BIP_LISTENED_TO_MYSELF, 0, 0);
				}
				else
				{
					EVRECORD2(BACNET_BIP_DECODING_BCAST_UCAST, 0, 0);
					
          /* data in src->mac[] is in network format */
          src->mac_len = 6;
          bip_addr_to_mac(&src->mac[0], &addr);
          memcpy(&src->mac[4], &port, 2);
          /* decode the length of the PDU - length is inclusive of BVLC */
          (void) decode_unsigned16(&pdu[2], &pdu_len);
          /* subtract off the BVLC header */
          pdu_len -= 4;
          pdu_offset = 4;
					
					if (pdu_len < max_pdu)
					{
						/* shift the buffer to return a valid PDU */
						for (i = 0; i < pdu_len; i++)
						{ pdu[i] = pdu[pdu_offset + i]; }
						
						EVRECORD2(BACNET_BIP_BCAST_UCAST_DECODED, 0, 0);
					}
					else
					{ 
						EVRECORD2(BACNET_BIP_BCAST_UCAST_PDU_TOO_LONG, pdu_len, 0);
						pdu_len = 0;
					}
        }
    }
		else if (function == BVLC_FORWARDED_NPDU)
		{
			EVRECORD2(BACNET_BIP_RECEIVED_FWD_NPDU, 0, 0);
			
      bip_mac_to_addr(&addr, &pdu[4]);
      memcpy(&port, &pdu[8], 2);
			
      if ((addr.addr == BIP_Address.s_addr) && (port == BIP_Port))
			{
        /* ignore forwarded messages from me */
				EVRECORD2(BACNET_BIP_LISTENED_TO_MYSELF, 0, 0);
        pdu_len = 0;
      }
			else
			{
        /* data in src->mac[] is in network format */
        src->mac_len = 6;
        bip_addr_to_mac(&src->mac[0], &addr);
        memcpy(&src->mac[4], &port, 2);
        /* decode the length of the PDU - length is inclusive of BVLC */
        (void) decode_unsigned16(&pdu[2], &pdu_len);
        /* subtract off the BVLC header */
        pdu_len -= 10;
        pdu_offset = 10;
			
				if (pdu_len < max_pdu)
				{
					/* shift the buffer to return a valid PDU */
					for (i = 0; i < pdu_len; i++)
					{ pdu[i] = pdu[pdu_offset + i]; }
					
					EVRECORD2(BACNET_BIP_FWD_NPDU_DECODED, 0, 0);
				}
				else
				{ 
					EVRECORD2(BACNET_BIP_FWD_PDU_TOO_LONG, pdu_len, 0);
					pdu_len = 0;
				}
      }
    }

		EVRECORD2(BACNET_BIP_RECEIVED, 0, 0);
    return pdu_len;
}

void bip_get_my_address(
    BACNET_ADDRESS * my_address)
{
    int i = 0;

    if (my_address) {
        my_address->mac_len = 6;
        memcpy(&my_address->mac[0], &BIP_Address.s_addr, 4);
        memcpy(&my_address->mac[4], &BIP_Port, 2);
        my_address->net = 0;    /* local only, no routing */
        my_address->len = 0;    /* no SLEN */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            /* no SADR */
            my_address->adr[i] = 0;
        }
    }

    return;
}

void bip_get_broadcast_address(
    BACNET_ADDRESS * dest)
{       /* destination address */
    int i = 0;  /* counter */

    if (dest) {
        dest->mac_len = 6;
        memcpy(&dest->mac[0], &BIP_Broadcast_Address.s_addr, 4);
        memcpy(&dest->mac[4], &BIP_Port, 2);
        dest->net = BACNET_BROADCAST_NETWORK;
        dest->len = 0;  /* no SLEN */
        for (i = 0; i < MAX_MAC_LEN; i++) {
            /* no SADR */
            dest->adr[i] = 0;
        }
    }

    return;
}
