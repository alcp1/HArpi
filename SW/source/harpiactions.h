//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 30/Jul/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

#ifndef HARPIACTIONS_H
#define HARPIACTIONS_H

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
 * Init Gateway data:
 * - empty the list and if list is available, free used memory
 * 
 **/
void harpiactions_init(void);

/**
 * Init list and memory after all elements are added with the 
 * harpiactions_AddElementToList function.
 * 
 **/
void harpiactions_load(harpiLinkedList* element);

/**
 * Search for an harpiActionSetsData data and send the data that matches such
 * an ID
 * 
 * \param   actionSetID (INPUT) The HAPCAN Frame to be searched
 *  
 **/
void harpiactions_SendActionsFromID(int16_t actionsSetID);


#ifdef __cplusplus
}
#endif

#endif

