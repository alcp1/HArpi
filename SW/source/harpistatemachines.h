//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First version                                                            //
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

#ifdef __cplusplus
}
#endif

#endif

