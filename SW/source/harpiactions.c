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
#include <harpiactions.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_ActionSets_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiActionSetsData* harpiActionSetArray = NULL;
static int16_t harpiActionSetArrayLen = 0;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static bool copyListToArray(harpiLinkedList* element);

// Copy from the Linked List to the Array
static bool copyListToArray(harpiLinkedList* element)
{
    int16_t i;
    harpiLinkedList* current;
    bool isOK;
    // Check all
    i = 0;
    isOK = true;
    if(element != NULL)
    {
        for(current = element; current != NULL; current = current->next) 
        {
            if(current->section == CSV_SECTION_ACTION_SETS)
            {
                if(i >= harpiActionSetArrayLen)
                {
                    #ifdef DEBUG_HARPIACTIONS_ERRORS
                    debug_print("harpiactions_load error!\n");
                    #endif
                    isOK = false;
                    break;
                }
                memcpy(&(harpiActionSetArray[i]), &(current->actionSetsData), 
                    sizeof(harpiActionSetsData));
                i++;
            }
        }
    }
    return isOK;
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
void harpiactions_init(void)
{
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Init array
    if(harpiActionSetArray != NULL)
    {
        free(harpiActionSetArray);
        harpiActionSetArray = NULL;
    }
    harpiActionSetArrayLen = 0;
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
}

void harpiactions_load(harpiLinkedList* element)
{
    bool isOK;
    //---------------------------------------------
    // Clear array, copy from list to array, delete linked lisk - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Clear array
    if(harpiActionSetArray != NULL)
    {
        free(harpiActionSetArray);
        harpiActionSetArray = NULL;
    }
    // Get array size and allocate memory
    harpiActionSetArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_ACTION_SETS);
    harpiActionSetArray = (harpiActionSetsData*)malloc(harpiActionSetArrayLen * 
        sizeof(harpiActionSetsData));
    // Create array from list
    isOK = copyListToArray(element);
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
    // Clear data if copy had an error
    if(!isOK)
    {
        harpiactions_init();
    }
}

void harpiactions_SendActionsFromID(int16_t actionsSetID)
{
    int16_t check;
    int16_t i;
    unsigned long long millisecondsSinceEpoch;
    int16_t frameCount;
    hapcanCANData* frames = NULL;
    // Get Timestamp
    millisecondsSinceEpoch = aux_getmsSinceEpoch();
    //------------------------------------------------
    // Avoid nested mutex locks from g_ActionSets_mutex and 
    // hapcan_addToCANWriteBuffer: First create an array of frames to be sent, 
    // then send them
    //------------------------------------------------
    // 1. Prepare frames to be sent - LOCK needed
    //------------------------------------------------
    // Init counter
    frameCount = 0;
    // Init frames to be sent - reserve for the same size as 
    // harpiActionSetArrayLen (worst case - all actions have the same ID)
    if(harpiActionSetArrayLen > 0)
    {
        frames = (hapcanCANData*)malloc(harpiActionSetArrayLen * 
            sizeof(hapcanCANData));
    }
    // LOCK
    pthread_mutex_lock(&g_ActionSets_mutex);
    // Check all elements of the array
    for(i = 0; i < harpiActionSetArrayLen; i++)
    {
        // Check for a match
        if(harpiActionSetArray[i].actionsSetID == actionsSetID)
        {
            // Add to frames to be sent
            memcpy(&(frames[frameCount]), &(harpiActionSetArray[i].frame), 
                sizeof(hapcanCANData));
            frameCount++;
        }
    }
    // UNLOCK
    pthread_mutex_unlock(&g_ActionSets_mutex);
    //---------------------------------
    // 2. Send Frames - LOCK not needed
    //---------------------------------
    // Check all elements of the array
    for(i = 0; i < frameCount; i++)
    {
        // Found - Send CAN Frame for action
        check = hapcan_addToCANWriteBuffer(&(harpiActionSetArray[i].frame),
            millisecondsSinceEpoch);
        if(check != HAPCAN_CAN_RESPONSE)
        {
            #ifdef DEBUG_HAPCAN_ERRORS
            debug_print("harpiactions_SendActionsFromID - ERROR: "
                "hapcan_addToCANWriteBuffer!\n");
            #endif
        }
    }
    // Free allocated memory
    free(frames);
    frames = NULL;
}
