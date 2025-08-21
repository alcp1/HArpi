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
#define HARPIEVENTS_NEW_EVENT   1
#define HARPIEVENTS_NO_EVENT    0
#define HARPIEVENTS_ERROR       -1
    
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
 * Init data:
 * - empty the list and if list is available, free used memory
 * 
 **/
void harpievents_init(void);

/**
 * Load list and memory with linked list data
 * \param   element (INPUT) The linked list data
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

/**
 * Get event from the buffer.
 * 
 * \param   event (OUTPUT) Event to be filled
 * 
 * \return      HARPIEVENTS_NEW_EVENT       "event" filled from buffer |
 *              HARPIEVENTS_NO_EVENT        no new event |
 *              HARPIEVENTS_ERROR           error
 */
int harpievents_getEvent(harpiEvent_t* event);

#ifdef __cplusplus
}
#endif

#endif

