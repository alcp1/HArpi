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
static int16_t* smIDArray;
static int16_t smIDArrayLen;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool copyListToArray(harpiLinkedList* element);
static void initStateMachinesArray(void);

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

// From the State Machine arrays, create the state machine status and 
// initialize it
static void initStateMachinesArray(void)
{
    int16_t i_Array;
    int16_t i_SM;
    int16_t* tempArray = NULL;
    int16_t tempArrayLen = 0;
    int16_t smcount;
    int16_t totalLen;
    bool ignoreID;
    // Init array
    if(smIDArray != NULL)
    {
        free(smIDArray);
        smIDArray = NULL;
    }
    smIDArrayLen = 0;
    // Init
    smcount = 0;
    totalLen = harpiSMEventsArrayLen + harpiSActionsArrayLen + 
        harpiSTransitionArrayLen;
    // Update if arrays exist and are OK
    if(totalLen > 0)
    {
        // Init Temp Array
        tempArray = (int16_t*)malloc(totalLen * sizeof(int16_t));
        tempArrayLen = totalLen;
        // Init entire array
        for(i_SM = 0; i_SM < tempArrayLen; i_SM++)
        {
            tempArray[i_SM] = -1;
        }
        // Init first position
        if(harpiSMEventsArrayLen > 0)
        {
            tempArray[0] = harpiSMEventsArray[0].stateMachineID;
        }
        else if(harpiSActionsArrayLen > 0)
        {
            tempArray[0] = harpiSActionsArray[0].stateMachineID;
        }
        else if(harpiSTransitionArrayLen > 0)  
        {
            tempArray[0] = harpiSTransitionArray[0].stateMachineID;
        }
        smcount++;
        // Check the harpiSMEventsData array
        for(i_Array = 0; i_Array < harpiSMEventsArrayLen; i_Array++)
        {
            ignoreID = false;
            for(i_SM = 0; i_SM < smcount; i_SM++)
            {
                if(harpiSMEventsArray[i_Array].stateMachineID == 
                    tempArray[i_SM])
                {
                    ignoreID = true;
                }
            }
            if(!ignoreID)
            {
                // Add to the list of states
                tempArray[smcount] = harpiSMEventsArray[i_Array].stateMachineID;
                smcount++;
            }
        }
        // Check the harpiStateActionsData array
        for(i_Array = 0; i_Array < harpiSActionsArrayLen; i_Array++)
        {
            ignoreID = false;
            for(i_SM = 0; i_SM < smcount; i_SM++)
            {
                if(harpiSActionsArray[i_Array].stateMachineID == 
                    tempArray[i_SM])
                {
                    ignoreID = true;
                }
            }
            if(!ignoreID)
            {
                // Add to the list of states
                tempArray[smcount] = harpiSActionsArray[i_Array].stateMachineID;
                smcount++;
            }
        }
        // Check the harpiStateTransitionsData array
        for(i_Array = 0; i_Array < harpiSTransitionArrayLen; i_Array++)
        {
            ignoreID = false;
            for(i_SM = 0; i_SM < smcount; i_SM++)
            {
                if(harpiSTransitionArray[i_Array].stateMachineID == 
                    tempArray[i_SM])
                {
                    ignoreID = true;
                }
            }
            if(!ignoreID)
            {
                // Add to the list of states
                tempArray[smcount] = 
                    harpiSTransitionArray[i_Array].stateMachineID;
                smcount++;
            }
        }
        // Copy from temporary array to final array
        smIDArrayLen = smcount;
        smIDArray = (int16_t*)malloc(smIDArrayLen * sizeof(int16_t));
        for(i_SM = 0; i_SM < smIDArrayLen; i_SM++)
        {
            smIDArray[i_SM] = tempArray[i_SM];
        }
        // Free temporary array
        free(tempArray);
        tempArray = NULL;
    }
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
    //----------------------------------------
    //    - State Machine Events
    //----------------------------------------
    if(harpiSMEventsArray != NULL)
    {
        free(harpiSMEventsArray);
        harpiSMEventsArray = NULL;
    }
    harpiSMEventsArrayLen = 0;
    //----------------------------------------
    //    - State Actions
    //----------------------------------------
    if(harpiSActionsArray != NULL)
    {
        free(harpiSActionsArray);
        harpiSActionsArray = NULL;
    }
    harpiSActionsArrayLen = 0;
    //----------------------------------------
    //    - State Transitions
    //----------------------------------------
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
    //----------------------------------------
    //    - State Machine Events
    //----------------------------------------
    // Clear array
    if(harpiSMEventsArray != NULL)
    {
        free(harpiSMEventsArray);
        harpiSMEventsArray = NULL;
    }
    // Get array size and allocate memory
    harpiSMEventsArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_STATE_MACHINES_AND_EVENTS);
    harpiSMEventsArray = (harpiSMEventsData*)malloc(harpiSMEventsArrayLen * 
        sizeof(harpiSMEventsData));
    //----------------------------------------
    //    - State Actions
    //----------------------------------------
    // Clear array
    if(harpiSActionsArray != NULL)
    {
        free(harpiSActionsArray);
        harpiSActionsArray = NULL;
    }
    // Get array size and allocate memory
    harpiSActionsArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_STATES_AND_ACTIONS);
    harpiSActionsArray = (harpiStateActionsData*)malloc(harpiSActionsArrayLen * 
        sizeof(harpiStateActionsData));
    //----------------------------------------
    //    - State Transitions
    //----------------------------------------
    // Clear array
    if(harpiSTransitionArray != NULL)
    {
        free(harpiSTransitionArray);
        harpiSTransitionArray = NULL;
    }
    // Get array size and allocate memory
    harpiSTransitionArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_STATE_TRANSITIONS);
    harpiSTransitionArray = (harpiStateTransitionsData*)malloc(
        harpiSTransitionArrayLen * sizeof(harpiStateTransitionsData));
    //----------------------------------------
    // Create arrays from list
    //----------------------------------------
    isOK = copyListToArray(element);
    // UNLOCK
    pthread_mutex_unlock(&g_SM_mutex);
    // Clear data if copy had an error
    if(!isOK)
    {
        harpism_init();
    }
    // LOCK
    pthread_mutex_lock(&g_SM_mutex);
    // Init state machine array
    initStateMachinesArray();
    // UNLOCK
    pthread_mutex_unlock(&g_SM_mutex);
}