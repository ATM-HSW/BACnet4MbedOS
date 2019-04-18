#ifndef EVREC_BACNET_4_MBED_H_
#define EVREC_BACNET_4_MBED_H_

/*----------*/
/* Includes */
/*----------*/
#if ENABLE_EVENT_RECORDER
	#include "EventRecorder.h"
#endif

/*---------*/
/* Defines */
/*---------*/
#ifndef EventLevelOp
	#define EventLevelOp 0x00
#endif

#ifndef EventLevelError
	#define EventLevelError 0x00
#endif


/*--------*/
/* Macros */
/*--------*/

#if BACNET_USER_EVENT_RECORDS
	#define EVRECORD2(a,b,c)		EventRecord2(a,b,c)
	#define EVRECORD4(a,b,c,d,e)	EventRecord4(a,b,c,d,e)
	#define EVRECORDDATA(a,b,c)		EventRecordData(a,b,c)
#else
	#define EVRECORD2(a,b,c)
	#define EVRECORD4(a,b,c,d,e)
	#define EVRECORDDATA(a,b,c)
#endif

#define STRSIZE(str) (strlen(str)*sizeof(char))

/*-------------*/
/* Definitions */
/*-------------*/
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define __ERROR_POS__ __FILE__ ":" TOSTRING(__LINE__)


/*--------------------*/
/* Typedefinitions */
/*--------------------*/

typedef enum
{	
	// BACnet Handler NPDU
	BACNET_H_NPDU_RCVD			    				= 0xA901 + EventLevelOp,		// Record2 / BACNET_PROTOCOL_VERSION
	BACNET_H_NPDU_CALLING_APDU_H				= 0xA902 + EventLevelOp,		// Record2
	BACNET_H_NPDU_NET_MSG_DCARD					= 0xA90E + EventLevelError,	// Record2 / dest.net
	BACNET_H_NPDU_BNETPROT_DCARD				= 0xA90F + EventLevelError,	// Record2 / BACNET_PROT_VERSION
				
	// BACnet Handler CoV       		
	BACNET_H_COV_RM_UNUSED_SUBSC				= 0xAA00 + EventLevelOp,		// Record2 / ObjIdentifier.instance / ObjIdentifier.type
	BACNET_H_COV_ADD_COV_ADDR						= 0xAA01 + EventLevelOp,		// Record2
	BACNET_H_COV_ADD_COV_SUCCESS				= 0xAA02 + EventLevelOp,		// Record2 / index
	BACNET_H_COV_ADD_COV_FAIL						= 0xAA03 + EventLevelError,	// Record2
	BACNET_H_COV_NOTIF_REQUEST					= 0xAA04 + EventLevelOp,		// Record2
	BACNET_H_COV_DEST_NOT_FOUND					= 0xAA05 + EventLevelError,	// RecordData / destination [BACNET_ADDRESS]
	BACNET_H_COV_NOTIF_SENT							= 0xAA06 + EventLevelOp,		// RecordData / destination [BACNET_ADDRESS]
	BACNET_H_COV_NOTIF_SENDING					= 0xAA07 + EventLevelOp,		// Record2 / ObjIdentifier.type / ObjIdentifier.instance
	BACNET_H_COV_RCV_SEG_MSG_ABORT			= 0xAA08 + EventLevelError,	// Record2
	BACNET_H_COV_REQ_DECODE_FAILED			= 0xAA09 + EventLevelError,	// Record2
	BACNET_H_COV_SENDING_ERROR					= 0xAA0A + EventLevelError,	// Record2 / error_code [BACNET_ERROR_CODE]
	BACNET_H_COV_SENDING_ABORT					= 0xAA0B + EventLevelError,	// Record2 / error_code [BACNET_ERROR_CODE]
	BACNET_H_COV_SENDING_REJECT					= 0xAA0C + EventLevelError,	// Record2 / error_code [BACNET_ERROR_CODE]
	BACNET_H_COV_SENDING_SACK						= 0xAA0D + EventLevelOp,		// Record2
	BACNET_H_COV_RESPONSE_SENT					= 0xAA0E + EventLevelOp,		// Record2 / bytes_sent
	BACNET_H_COV_SENDING_FAILED					= 0xAA0F + EventLevelError,	// Record2 / errno
    
	// BACnet Handler Unconfirmed COV       
	BACNET_H_UCOV_RCVD_NOTIF						= 0xAB01 + EventLevelOp,		// RecordData / src [BACNET_ADDRESS]
	BACNET_H_UCOV_NOTIF_DECODED					= 0xAB02 + EventLevelOp,		// Record2 / cov_data.monitoredObjectIdentifier.instance / cov_data.monitoredObjectIdentifier.type
	BACNET_H_UCOV_NOTIF_DECODE_FAIL			= 0xAB03 + EventLevelError,	// Record2 / cov_data.monitoredObjectIdentifier.instance / cov_data.monitoredObjectIdentifier.type
	
	// BACnet Handler Read Property Multiple
	BACNET_H_RPM_RCVD_REQUEST						= 0xAC01 + EventLevelOp,		// RecordData / source [BACNET_ADDRESS]
	BACNET_H_RPM_RCVD_SEG_MESSAGE				= 0xAC02 + EventLevelOp,		// Record2 / error / rpmdata.error_code
	BACNET_H_RPM_BAD_RPM_ENCODING				= 0xAC03 + EventLevelError,	// Record2 / error / __LINE__
	BACNET_H_RPM_RESPONSE_TOO_BIG				= 0xAC04 + EventLevelError,	// Record2 / error / rpmdata.error_code
	BACNET_H_RPM_ENC_FAIL_BUFF_FULL			= 0xAC05 + EventLevelError,	// Record2 / rpmdata.error_code / __LINE__
	BACNET_H_RPM_MSG_TOO_LARGE					= 0xAC06 + EventLevelError,	// record2 / rpmdata.error_code / __LINE__
	BACNET_H_RPM_SENDING_ERROR					= 0xAC07 + EventLevelError,	// Record2 / rpmdata.error_code / __LINE__
	BACNET_H_RPM_SENDING_ABORT					= 0xAC08 + EventLevelError,	// Record2 / rpmdata.error_code / __LINE__
	BACNET_H_RPM_SENDING_REJECT					= 0xAC09 + EventLevelError,	// Record2 / rpmdata.error_code / __LINE__
	BACNET_H_RPM_RESPONSE_SENT					= 0xAC0A + EventLevelOp,		// Record2 / bytes_sent
	BACNET_H_RPM_SENDING_FAILED					= 0xAC0B + EventLevelError,	// Record2 / errno
	
	// BACnet Handler Read Property
	BACNET_H_RP_RCVD_REQUEST						= 0xAD01 + EventLevelOp,		// RecordData / src [BACNET_ADRESS]
	BACNET_H_RP_REQUEST_DECODED					= 0xAD02 + EventLevelOp,		// Record2 / rpdata.object_type / rpdata.object_instance
	BACNET_H_RP_REQUEST_DECODE_FAIL 		= 0xAD03 + EventLevelOp,		// Record2 / rpdata.object_type / rpdata.object_instance
	BACNET_H_RP_RCVD_SEG_MSG						= 0xAD04 + EventLevelOp,		// Record2
	BACNET_H_RP_MSG_TOO_LARGE						= 0xAD05 + EventLevelOp,		// Record2 / rpdata.error_code
	BACNET_H_RP_SENDING_ACK							= 0xAD06 + EventLevelOp,		// Record2
	BACNET_H_RP_SENDING_ABORT						= 0xAD07 + EventLevelError,	// Record2 / rpdata.error_code
	BACNET_H_RP_SENDING_ERROR						= 0xAD08 + EventLevelError, // Record2 / rpdata.error_code
	BACNET_H_RP_SENDING_REJECT					= 0xAD09 + EventLevelError, // Record2 / rpdata.error_code
	BACNET_H_RP_SENDING_FAILED					= 0xAD0A + EventLevelError,	// Record2 / bytes_sent
	BACNET_H_RP_ERRNO										= 0xAD0F + EventLevelError,	// RecordData / strerror(errno)
			
	// BACnet Handler WhoIs		
	BACNET_H_WIS_RCVD_REQUEST						= 0xAE01 + EventLevelOp,		// RecordData / src [BACNET_ADDRESS]
	BACNET_H_WIS_SENT_IAM           		= 0xAE02 + EventLevelOp,		// Record2
	BACNET_H_WIS_SENT_UCAST_IAM					= 0xAE03 + EventLevelOp,		// Record2
			
	// BACnet Handler WP		
	BACNET_H_WP_CALLED			    				= 0xAF01 + EventLevelOp,		// Record2 / PDUlen
	BACNET_H_WP_RCVD_REQ		    				= 0xAF02 + EventLevelOp,		// Record2 / ReqLen
	BACNET_H_WP_CONF_SACK		    				= 0xAF03 + EventLevelOp,		// Record2 / SACKlen / SERVICE_CONFIRMED_xx
	BACNET_H_WP_RCVD_SEG_MSG	  				= 0xAF04 + EventLevelOp,		// Record2 / len / ABORT_REASON_xx
	BACNET_H_WP_SEND_PDU_FAILED   			= 0xAF0D + EventLevelError,	// Record2 / bytes_sent
	BACNET_H_WP_BAD_REQ			    				= 0xAF0E + EventLevelError,	// Record2 / APDUlen / ABORT_REASON_xx
	BACNET_H_WP_RCV_FAILED		    			= 0xAF0F + EventLevelError,	// Record2 / APDUlen / wp_data.error_code
			
			
	// BACnet Object CB EventQueue		
	BACNET_EVQ_STARTING									= 0xB001 + EventLevelOp,		// Record2
	BACNET_EVQ_STARTED									= 0xB002 + EventLevelOp,		// Record2 / retVal
	BACNET_EVQ_FAILED										= 0xB003 + EventLevelError,	// Record2 / retVal
	
	
	// BACnet EventQueue Read / Write
	BACNET_EVQ_AI_RDCB_CALLED						= 0xB100 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_AI_RDCB_FAILED						= 0xB101 + EventLevelError,	// Record2 / id
	BACNET_EVQ_AI_WRCB_CALLED						= 0xB200 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_AI_WRCB_FAILED						= 0xB201 + EventLevelError,	// Record2 / id
			
	BACNET_EVQ_AO_RDCB_CALLED						= 0xB102 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_AO_RDCB_FAILED						= 0xB103 + EventLevelError,	// Record2 / id
	BACNET_EVQ_AO_WRCB_CALLED						= 0xB202 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_AO_WRCB_FAILED						= 0xB203 + EventLevelError,	// Record2 / id
			
	BACNET_EVQ_AO_VAL_DECODED						= 0xB20E + EventLevelOp,		// Record2 / value / len
	BACNET_EVQ_AO_VAL_DECODE_FAIL				= 0xB20F + EventLevelError,	// Record2 / len
			
	BACNET_EVQ_AV_RDCB_CALLED						= 0xB104 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_AV_RDCB_FAILED						= 0xB105 + EventLevelError,	// Record2 / id
	BACNET_EVQ_AV_WRCB_CALLED						= 0xB204 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_AV_WRCB_FAILED						= 0xB205 + EventLevelError,	// Record2 / id
			
	BACNET_EVQ_BI_RDCB_CALLED						= 0xB106 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_BI_RDCB_FAILED						= 0xB107 + EventLevelError,	// Record2 / id
			
	BACNET_EVQ_BO_RDCB_CALLED						= 0xB108 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_BO_RDCB_FAILED						= 0xB109 + EventLevelError,	// Record2 / id
	BACNET_EVQ_BO_WRCB_CALLED						= 0xB206 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_BO_WRCB_FAILED						= 0xB207 + EventLevelError,	// Record2 / id
			
	BACNET_EVQ_BV_RDCB_CALLED						= 0xB10A + EventLevelOp,		// Record2 / id
	BACNET_EVQ_BV_RDCB_FAILED						= 0xB10B + EventLevelError,	// Record2 / id
	BACNET_EVQ_BV_WRCB_CALLED						= 0xB208 + EventLevelOp,		// Record2 / id
	BACNET_EVQ_BV_WRCB_FAILED						= 0xB209 + EventLevelError,	// Record2 / id
			
	BACNET_EVQ_MSV_RDCB_CALLED					= 0xB10C + EventLevelOp,		// Record2 / id
	BACNET_EVQ_MSV_RDCB_FAILED					= 0xB10D + EventLevelError,	// Record2 / id
	BACNET_EVQ_MSV_WRCB_CALLED					= 0xB20A + EventLevelOp,		// Record2 / id
	BACNET_EVQ_MSV_WRCB_FAILED					= 0xB20B + EventLevelError,	// Record2 / id
			
	BACNET_EVQ_DEVOBJ_RDCB_CALLED				= 0xB10E + EventLevelOp,		// Record2 / id
	BACNET_EVQ_DEVOBJ_RDCB_FAILED				= 0xB10F + EventLevelError,	// Record2 / id
	BACNET_EVQ_DEVOBJ_WRCB_CALLED				= 0xB20C + EventLevelOp,		// Record2 / id
	BACNET_EVQ_DEVOBJ_WRCB_FAILED				= 0xB20D + EventLevelError,	// Record2 / id	
	
	// BACnet Task
	BACNET_PDU_RECEIVED			    				= 0xB301 + EventLevelOp,		// Record2 / PDUlen	
	BACNET_SEND_I_AM_UCAST		    			= 0xB302 + EventLevelOp,		// Record2 / PDUlen
	BACNET_SENT_I_AM_UCAST		    			= 0xB303 + EventLevelOp,		// Record2 / bytes_sent
	BACNET_SEND_I_AM_BCAST		    			= 0xB304 + EventLevelOp,		// Record2 / PDUlen
	BACNET_SENT_I_AM_BCAST		    			= 0xB305 + EventLevelOp,		// Record2 / bytes_sent
	BACNET_PDU_TYPE_CONFIRMED	    			= 0xB306 + EventLevelOp,		// Record2 / PDU_TYPE_xx
	BACNET_FAILED_I_AM_UCAST	    			= 0xB30D + EventLevelError,	// Record2 / pdu_len
	BACNET_FAILED_I_AM_BCAST	    			= 0xB30E + EventLevelError,	// Record2 / bytes_sent
	BACNET_PDU_TYPE_UNKNOWN		    			= 0xB30F + EventLevelError,	// Record2
	
	// BACnet Thread
	BACNET_TASK_STARTING								= 0xB401 + EventLevelOp,		// Record2
	BACNET_TASK_STARTED 								= 0xB402 + EventLevelOp,		// Record2 / retVal
	BACNET_TASK_FAILED									= 0xB403 + EventLevelError,	// Record2 / retVal
	
	// BACnet UDP Socket
	BACNET_UDP_SOCK_OPENED							= 0xB500 + EventLevelOp,		// Record2 / nsapi_error
	BACNET_UDP_SOCK_FAILED							= 0xB501 + EventLevelError,	// Record2 / nsapi_error
	BACNET_UDP_SOCK_BOUND								= 0xB502 + EventLevelOp,		// Record2 / nsapi_error
	BACNET_UDP_SOCK_BIND_FAIL						= 0xB503 + EventLevelError,	// Record2 / nsapi_error
	
	// BACnet Init
	BACNET_INIT_ADDR_CONVERSION_OK			= 0xB600 + EventLevelOp,		// RecordData / ifname_string
	BACNET_INIT_ADDR_CONVERSION_FAIL		= 0xB601 + EventLevelError,	// RecordData / ifname_string
	
	// BACnet Send PDU
	BACNET_SENDING_PDU									= 0xB700 + EventLevelOp,		// Record2
	BACNET_SENDING_PDU_BCAST						= 0xB701 + EventLevelOp,		// RecordData / addr_string
	BACNET_SENDING_PDU_NW_BCAST					= 0xB702 + EventLevelOp,		// RecordData / addr_string
	BACNET_SENDING_PDU_UCAST						= 0xB703 + EventLevelOp,		// RecordData / addr_string
	BACNET_SENT_PDU											= 0xB704 + EventLevelOp,		// Record2 / bytes_sent
	
	BACNET_SENDING_PDU_FAILED						= 0xB705 + EventLevelError,	// Record2 / nsapi_error
	BACNET_SEND_PDU_INVALID_ADDR				= 0xB706 + EventLevelError,	// Record2
	
	// BACnet BIP Receiving
	BACNET_BIP_RECEIVING								= 0xB800 + EventLevelOp,		// RecordData / ip_string
	BACNET_BIP_LISTENED_TO_MYSELF				= 0xB801 + EventLevelOp,		// Record2
	BACNET_BIP_DECODING_BCAST_UCAST			= 0xB802 + EventLevelOp,		// Record2
	BACNET_BIP_BCAST_UCAST_DECODED			= 0xB803 + EventLevelOp,		// Record2
	BACNET_BIP_BCAST_UCAST_PDU_TOO_LONG	= 0xB804 + EventLevelError,	// Record2 / pdu_len
	BACNET_BIP_RECEIVED_FWD_NPDU				= 0xB805 + EventLevelOp,		// Record2
	BACNET_BIP_FWD_NPDU_DECODED					= 0xB806 + EventLevelOp,		// Record2
	BACNET_BIP_FWD_PDU_TOO_LONG					= 0xB807 + EventLevelError,	// Record2 / pdu_len
	BACNET_BIP_RECEIVED									= 0xB808 + EventLevelOp,		// Record2
	BACNET_BIP_RECEIVING_NON_BAC		    = 0xB809 + EventLevelOp,		// Record2
	
	// BACnet BIP Reinit
	BACNET_BIP_REINIT										= 0xB900 + EventLevelOp,		// Record2
	BACNET_BIP_REINIT_OK								= 0xB901 + EventLevelOp,		// Record2
	BACNET_BIP_REINIT_FAILED						= 0xB902 + EventLevelError,	// Record2 / nsapi_error / bip_init_bool
	
}EVENT_DEF_ID_BNET4MBED;

#endif
