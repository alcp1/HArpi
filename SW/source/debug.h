//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First Version: copied from HMSG 01.12                                    //
//----------------------------------------------------------------------------//
//  1.01     | 30/Jul/2025 |                               | ALCP             //
// - Updates to remove unused parts from HMSG 01.12                           //
//----------------------------------------------------------------------------//

#ifndef DEBUG_H
//#define DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "hapcan.h"
    
//----------------------------------------------------------------------------//
// EXTERNAL DEFINITIONS
//----------------------------------------------------------------------------//
/* Debug */ 
#define DEBUG_ON
#define DEBUG_VERSION

/* Buffer */
//#define DEBUG_BUFFER
    
/* Manager */
#define DEBUG_MANAGER_ERRORS
#define DEBUG_MANAGER_CONFIG_EVENTS
    
/* CAN Buffer */
#define DEBUG_CANBUF_ERRORS
//#define DEBUG_CANBUF_SEND // Disable for production

/* CAN DEBUG */   
#define DEBUG_CAN_HAPCAN
//#define DEBUG_CAN_STANDARD // Disable for production
    
/* HAPCAN DEBUG */   
#define DEBUG_HAPCAN_ERRORS
   
/* CVS CONFIG DEBUG */
#define DEBUG_CVSCONFIG_ERRORS
#define DEBUG_CVSCONFIG_EVENTS

/* HARPIACTIONS */
#define DEBUG_HARPIACTIONS_ERRORS

/* HARPIACTIONS */
#define DEBUG_HARPIEVENTS_ERRORS
    
//----------------------------------------------------------------------------//
// EXTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// EXTERNAL CONSTANTS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// EXTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/**
 * Debug Initialization
 * 
 * \param   None
 * \return  EXIT_SUCCESS / EXIT_FAILURE
 */
int debug_init(void);

/**
 * Debug End
 * 
 * \param   None
 * \return  EXIT_SUCCESS / EXIT_FAILURE
 */
int debug_end(void);

/**
 * Debug Print
 * 
 * \param   same as printf
 * \return  none
 */
void debug_print(const char * format, ...);

/**
 * Debug Print CAN Frame
 * 
 * \param   same as printf
 * \return  none
 */
void debug_printCAN(const char * text, struct can_frame* const pcf_Frame);

/**
 * Debug Print HAPCAN Frame
 * 
 * \param   same as printf
 * \return  none
 */
void debug_printHAPCAN(const char * text, hapcanCANData *hd);

/**
 * Debug Print HAPCAN Socket Frame
 * 
 * \param   same as printf
 * \return  none
 */
void debug_printSocket(const char * text, uint8_t* data, int dataLen);

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */

