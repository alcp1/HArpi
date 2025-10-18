//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//
//  1.01     | 18/Oct/2025 |                               | ALCP             //
// - New actions when all loads are OFF                                       //
//----------------------------------------------------------------------------//

#ifndef HARPISM_H
#define HARPISM_H

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
void harpism_init(void);

/**
 * Load list and memory with linked list data
 * \param   element (INPUT) The linked list data
 * 
 **/
void harpism_load(harpiLinkedList* element);

/**
 * Periodic check of state machine
 * 
 **/
void harpism_periodic(void);

/**
 * Callback when all loads of a given state machine are OFF
 * \param   stateMachineID  (INPUT) The state machine ID that has its loads set 
 *                          to OFF
 * 
 **/
void harpism_loadsOFFCallback(int16_t stateMachineID);

#ifdef __cplusplus
}
#endif

#endif

