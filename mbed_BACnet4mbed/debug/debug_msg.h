#ifndef _H_DEBUG_H_
#define _H_DEBUG_H_

/*----------*/
/* Includes */
/*----------*/
#include "time.h"

/*---------*/
/* Defines */
/*---------*/
#define HANDLER_DEBUG           1   // General Debugging messages switch
#define HANDLER_DEBUG_VERBOSE   0   // Additional debugging messages

#define USE_TIMESTAMP           1   // Implement Timestamps into debugging messages
                                    // Use of systemtime for timestamp,
                                    // use of NTP or similar recommended

/*--------*/
/* Makros */
/*--------*/

// Example:
// H_DEBUG_MSG ("Invalid Package! %i", pkg_num);
// --> Output (w/o Timestamp): [BNET : DBG] Invalid Package! 255\r\n

#if HANDLER_DEBUG == 1
    // Simple debugging Messages
    #if USE_TIMESTAMP == 1
        #define H_DEBUG_MSG(X, ...)                                             \
        {                                                                       \
            time_t ctTime;                                                      \
            ctTime = time(NULL);                                                \
            printf("%s [BNET : DBG] "X"\r\n", ctime(&ctTime), ##__VA_ARGS__);   \
        }
    #else
        #define H_DEBUG_MSG(X, ...) printf("[BNET : DBG] "X"\r\n", ##__VA_ARGS__);
    #endif
    
    // Verbose debugging messages
	#if HANDLER_DEBUG_VERBOSE == 1
        #if USE_TIMESTAMP == 1
            #define H_DEBUG_VMSG(X, ...)                                            \
            {                                                                       \
                time_t ctTime;                                                      \
                ctTime = time(NULL);                                                \
                printf("%s [BNET : DBG] "X"\r\n", ctime(&ctTime), ##__VA_ARGS__);   \
            }
        #else
            #define H_DEBUG_VMSG(X, ...) printf("[BNET : DBGv] "X"\r\n", ##__VA_ARGS__);
        #endif
	#else
		#define H_DEBUG_VMSG(X, ...)
	#endif
        
#else
	#define H_DEBUG_MSG(X, ...)    
	#define H_DEBUG_VMSG(X, ...)
#endif


#endif
