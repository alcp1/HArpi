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
#include <harpievents.h>

//----------------------------------------------------------------------------//
// INTERNAL DEFINITIONS
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL TYPES
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// INTERNAL GLOBAL VARIABLES
//----------------------------------------------------------------------------//
static pthread_mutex_t g_EventSets_mutex = PTHREAD_MUTEX_INITIALIZER;
static harpiEventSetsData* harpiEventSetArray = NULL;
static int16_t harpiEventSetArrayLen = 0;

//----------------------------------------------------------------------------//
// INTERNAL FUNCTIONS
//----------------------------------------------------------------------------//
static void copyListToArray(harpiLinkedList* element);

// Copy from the Linked List to the Array
static void copyListToArray(harpiLinkedList* element)
{
    int16_t i;
    harpiLinkedList* current;
    // Check all
    i = 0;
    if(element != NULL)
    {
        for(current = element; current != NULL; current = current->next) 
        {
            if(current->section == CSV_SECTION_EVENT_SETS)
            {
                if(i >= harpiEventSetArrayLen)
                {
                    #ifdef DEBUG_HARPIEVENTS_ERRORS
                    debug_print("harpievents_load error!\n");
                    #endif
                    break;
                }
                memcpy(&(harpiEventSetArray[i]), &(current->eventSetsData), 
                    sizeof(harpiEventSetsData));
                i++;
            }
        }
    }
}

//----------------------------------------------------------------------------//
// EXTERNAL FUNCTIONS
//----------------------------------------------------------------------------//

void harpievents_init(void)
{
    //---------------------------------------------
    // Delete Linked List and Array - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_EventSets_mutex);
    // Init array
    if(harpiEventSetArray != NULL)
    {
        free(harpiEventSetArray);
        harpiEventSetArray = NULL;
    }
    harpiEventSetArrayLen = 0;
    // UNLOCK
    pthread_mutex_unlock(&g_EventSets_mutex);
}

void harpievents_load(harpiLinkedList* element)
{
    //---------------------------------------------
    // Clear array, copy from list to array, delete linked lisk - PROTECTED
    //---------------------------------------------
    // LOCK
    pthread_mutex_lock(&g_EventSets_mutex);
    // Clear array
    if(harpiEventSetArray != NULL)
    {
        free(harpiEventSetArray);
        harpiEventSetArray = NULL;
    }
    // Get array size and allocate memory
    harpiEventSetArrayLen = harpi_getLinkedListNElements(
        CSV_SECTION_EVENT_SETS);
    harpiEventSetArray = (harpiEventSetsData*)malloc(harpiEventSetArrayLen * 
        sizeof(harpiEventSetsData));
    // Create array from list
    copyListToArray(element);
    // UNLOCK
    pthread_mutex_unlock(&g_EventSets_mutex);
}

void harpievents_handleCAN(hapcanCANData* hapcanData, 
    unsigned long long timestamp)
{
    
}