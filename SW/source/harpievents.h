//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 01/Jun/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

#ifndef HARPIEVENTS_H
#define HARPIEVENTS_H

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
 * Init events buffer (to be called once)
 * 
 * \return  EXIT_SUCCESS
 *          EXIT_FAILURE (Close and Reinit Buffers)
 **/
int harpievents_createBuffer(void);

/**
 * Init Gateway data:
 * - empty the list and if list is available, free used memory
 * 
 **/
void harpievents_init(void);

/**
 * Init list and memory after all elements are added with the 
 * harpiactions_AddElementToList function.
 * 
 **/
void harpievents_load(harpiLinkedList* element);

/**
 * Check the CAN message received for generating events
 * \param   hapcanData      (INPUT) received HAPCAN Frame
 *          timestamp       (INPUT) Received message timestamp
 * 
 */
void harpievents_handleCAN(hapcanCANData* hapcanData, 
        unsigned long long timestamp);


#ifdef __cplusplus
}
#endif

#endif

