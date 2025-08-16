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
#include <harpiloads.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_SMLoads_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiSMLoadsData* harpiSMLoadsArray = NULL;
static int16_t harpiSMLoadsArrayLen = 0;

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
            if(current->section == CSV_SECTION_STATE_MACHINES_AND_LOADS)
            {
                if(i >= harpiSMLoadsArrayLen)
                {
                    #ifdef DEBUG_HARPILOADS_ERRORS
                    debug_print("harpiloads_load error!\n");
                    #endif
                    isOK = false;
                    break;
                }
                memcpy(&(harpiSMLoadsArray[i]), &(current->smLoadsData), 
                    sizeof(harpiSMLoadsData));
                i++;
            }
        }
    }
    return isOK;
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
void harpiloads_init(void)
{
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    // Init array
    if(harpiSMLoadsArray != NULL)
    {
        free(harpiSMLoadsArray);
        harpiSMLoadsArray = NULL;
    }
    harpiSMLoadsArrayLen = 0;
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
}

void harpiloads_load(harpiLinkedList* element)
{
    bool isOK;
    //---------------------------------------------
    // Clear array, copy from list to array, delete linked lisk - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_SMLoads_mutex);
    // Clear array
    if(harpiSMLoadsArray != NULL)
    {
        free(harpiSMLoadsArray);
        harpiSMLoadsArray = NULL;
    }
    // Get array size and allocate memory
    harpiSMLoadsArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_STATE_MACHINES_AND_LOADS);
    harpiSMLoadsArray = (harpiSMLoadsData*)malloc(harpiSMLoadsArrayLen * 
        sizeof(harpiSMLoadsData));
    // Create array from list
    isOK = copyListToArray(element);
    // UNLOCK
    pthread_mutex_unlock(&g_SMLoads_mutex);
    // Clear data if copy had an error
    if(!isOK)
    {
        harpiloads_init();
    }
}
