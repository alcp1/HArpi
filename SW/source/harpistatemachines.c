//----------------------------------------------------------------------------//
//                               OBJECT HISTORY                               //
//----------------------------------------------------------------------------//
//  REVISION |    DATE     |                               |      AUTHOR      //
//----------------------------------------------------------------------------//
//  1.00     | 01/Jun/2025 |                               | ALCP             //
// - First version                                                            //
//----------------------------------------------------------------------------//

/*
* Includes
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>
#include <auxiliary.h>
#include <debug.h>
#include <harpistatemachines.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_SM_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiSMEventsData* harpiSMEventsArray = NULL;
static int16_t harpiSMEventsArrayLen = 0;
static harpiStateActionsData* harpiSActionsArray = NULL;
static int16_t harpiSActionsArrayLen = 0;
static harpiStateTransitionsData* harpiSTransitionArray = NULL;
static int16_t harpiSTransitionArrayLen = 0;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool copyListToArray(harpiLinkedList* element);

// Copy from the Linked List to the Array - return true if OK
static bool copyListToArray(harpiLinkedList* element)
{
    int16_t i_events;
    int16_t i_actions;
    int16_t i_transitions;
    bool isOK;
    harpiLinkedList* current;
    // Check all
    i_events = 0;
    i_actions = 0;
    i_transitions = 0;
    isOK = true;
    if(element != NULL)
    {
        for(current = element; current != NULL; current = current->next) 
        {
            switch(current->section)
            {
                // ----------------------------------
                //    - State Machine Events
                // ----------------------------------
                case CSV_SECTION_STATE_MACHINES_AND_EVENTS:
                    if(i_events >= harpiSMEventsArrayLen)
                    {
                        #ifdef DEBUG_HARPISM_ERRORS
                        debug_print("harpism_load error: Events!\n");
                        #endif
                        isOK = false;
                    }
                    else
                    {
                        memcpy(&(harpiSMEventsArray[i_events]),
                            &(current->smEventsData), 
                            sizeof(harpiSMEventsData));
                        i_events++;
                    }
                    break;
                // ----------------------------------
                //    - State Actions
                // ----------------------------------
                case CSV_SECTION_STATES_AND_ACTIONS:
                    if(i_actions >= harpiSActionsArrayLen)
                    {
                        #ifdef DEBUG_HARPISM_ERRORS
                        debug_print("harpism_load error: Actions!\n");
                        #endif
                        isOK = false;
                    }
                    else
                    {
                        memcpy(&(harpiSActionsArray[i_actions]),
                            &(current->stateActionsData), 
                            sizeof(harpiStateActionsData));
                        i_actions++;
                    }
                    break;
                // ----------------------------------
                //    - State Transitions
                // ----------------------------------
                case CSV_SECTION_STATE_TRANSITIONS:
                    if(i_transitions >= harpiSActionsArrayLen)
                    {
                        #ifdef DEBUG_HARPISM_ERRORS
                        debug_print("harpism_load error: Transitions!\n");
                        #endif
                        isOK = false;
                    }
                    else
                    {
                        memcpy(&(harpiSTransitionArray[i_transitions]),
                            &(current->stateTransitionsData), 
                            sizeof(harpiStateTransitionsData));
                        i_transitions++;
                    }
                    break;
                // ----------------------------------
                //    - Others: do nothing
                // ----------------------------------
                default:
                    break;
            }
            if(!isOK)
            {
                break;
            }
        }
    }
    return isOK;
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
void harpism_init(void)
{
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_SM_mutex);
    // Init arrays
    //    - State Machine Events
    if(harpiSMEventsArray != NULL)
    {
        free(harpiSMEventsArray);
        harpiSMEventsArray = NULL;
    }
    harpiSMEventsArrayLen = 0;
    //    - State Actions
    if(harpiSActionsArray != NULL)
    {
        free(harpiSActionsArray);
        harpiSActionsArray = NULL;
    }
    harpiSActionsArrayLen = 0;
    //    - State Transitions
    if(harpiSTransitionArray != NULL)
    {
        free(harpiSTransitionArray);
        harpiSTransitionArray = NULL;
    }
    harpiSTransitionArrayLen = 0;
    // UNLOCK
    pthread_mutex_unlock(&g_SM_mutex);
}

void harpism_load(harpiLinkedList* element)
{
    bool isOK;
    //---------------------------------------------
    // Clear array, copy from list to array, delete linked lisk - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_SM_mutex);
    // Clear array
    if(harpiSMEventsArray != NULL)
    {
        free(harpiSMEventsArray);
        harpiSMEventsArray = NULL;
    }
    // Get array size and allocate memory
    harpiSMEventsArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_STATE_MACHINES_AND_LOADS);
    harpiSMEventsArray = (harpiSMEventsData*)malloc(harpiSMEventsArrayLen * 
        sizeof(harpiSMEventsData));
    // Create array from list
    isOK = copyListToArray(element);
    // UNLOCK
    pthread_mutex_unlock(&g_SM_mutex);
    // Clear data if copy had an error
    if(!isOK)
    {
        harpism_init();
    }
}
