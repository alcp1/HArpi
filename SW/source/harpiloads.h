//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

#ifndef HARPILOADS_H
#define HARPILOADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <harpi.h>

//----------------------------------------------------------------------------//
// EXTERNAL DEFINITIONS
//----------------------------------------------------------------------------//    

    
//----------------------------------------------------------------------------//
// EXTERNAL TYPES
//----------------------------------------------------------------------------//

    
//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
/**
 * Init data:
 * - empty the list and if list is available, free used memory
 * 
 **/
void harpiloads_init(void);

/**
 * Load list and memory with linked list data
 * \param   element (INPUT) The linked list data
 * 
 **/
void harpiloads_load(harpiLinkedList* element);

/**
 * Periodic check for unitialized loads
 * 
 **/
void harpiloads_periodic(void);

/**
 * Check the CAN message received for updating loads status
 * \param   hapcanData      (INPUT) received HAPCAN Frame
 *          timestamp       (INPUT) Received message timestamp
 * 
 */
void harpiloads_handleCAN(hapcanCANData* hapcanData, 
        unsigned long long timestamp);

/**
 * Load list and memory with linked list data
 * \param   stateMachineID (INPUT) The state machine ID
 * 
 **/
harpiLoadStatus_t harpiloads_isAnyLoadON(int16_t stateMachineID);

/**
 * Turn OFF the loads of a given state machine
 * \param   stateMachineID (INPUT) The state machine ID
 * 
 **/
void harpiloads_setLoadsOFF(int16_t stateMachineID);



#ifdef __cplusplus
}
#endif

#endif

